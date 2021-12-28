#include "NetworkClient.h"

using namespace std;
using namespace networking;

NetworkClient::NetworkClient()
{
    //ctor
}

NetworkClient::~NetworkClient()
{
    // Client beenden
    stop();

    return;
}

int NetworkClient::start(const std::string& serverIp,
                         const int serverPort,
                         const char* const pathToCaCert,
                         const char* const pathToCert,
                         const char* const pathToPrivKey,
                         const int tlsVerify,
                         const unsigned int tlsMode)
{
    // Prüfen, ob Client bereits läuft
    if (running)
    {
#ifdef DEVELOP
        cerr << "TLS Client läuft bereits" << endl;
#endif // DEVELOP

        return TLSCLIENT_START_OK;
    }

    // Prüfen, ob der Port im verfügbaren Bereich liegt
    if (1 > serverPort || 65536 < serverPort)
    {
        cerr << "Der Port muss zwischen 1 und 65535 liegen" << endl;

        return TLSCLIENT_ERROR_START_WRONG_PORT;
    }

    // Connection String erstellen
    const char* const connstring = (serverIp + ":" + to_string(serverPort)).c_str();

    // OpenSSL initialisieren
    OpenSSL_add_ssl_algorithms();

    // Verschlüsselungsmethode festlegen (Neueste verfügbare ersion von TLS clientseitig)
    if (!(clientContext = SSL_CTX_new(TLS_client_method())))
    {
        cerr << "Verschlüsselungsmethode kann nicht festgelegt werden" << endl;

        // Client beenden
        stop();

        return TLSCLIENT_ERROR_START_SET_CONTEXT;
    }

    if (pathToCaCert)
    {
        // CA Zertifikat für Server laden
        if (1 != SSL_CTX_load_verify_locations(clientContext, pathToCaCert, NULL))
        {
            cerr << "CA Zertifikat \"" << pathToCaCert << "\" kann nicht geladen werden" << endl;

            // Server beenden
            stop();

            return TLSCLIENT_ERROR_START_WRONG_CA_PATH;
        }
    }

    // Client Zertifikat laden
    if (pathToCert && pathToPrivKey)
    {
        if (1 != SSL_CTX_use_certificate_file(clientContext, pathToCert, SSL_FILETYPE_PEM))
        {
            cerr << "Client Zertifikat \"" << pathToCert << "\" konnte nicht geladen werden" << endl;

            // Server beenden
            stop();

            return TLSCLIENT_ERROR_START_WRONG_CERT_PATH;
        }

        // Client Private Key laden
        if (1 != SSL_CTX_use_PrivateKey_file(clientContext, pathToPrivKey, SSL_FILETYPE_PEM))
        {
            cerr << "Client Private Key \"" << pathToPrivKey << "\" konnte nicht geladen werden" << endl;

            // Server beenden
            stop();

            return TLSCLIENT_ERROR_START_WRONG_KEY_PATH;
        }

        // Prüfen, ob das Zertifikat und der Private Key zusammen passen
        if (1 != SSL_CTX_check_private_key(clientContext))
        {
            cerr << "Client Private Key passt nicht zum Zertifikat" << endl;

            // Server beenden
            stop();

            return TLSCLIENT_ERROR_START_WRONG_KEY;
        }
    }

    // TLS Modus festlegen
    SSL_CTX_set_mode(clientContext, tlsMode);

    // Authentifizierung des Servers fordern
    SSL_CTX_set_verify(clientContext, tlsVerify, NULL);

    // Signatur des Servers Zertifikats prüfen (Muss zum CA Zertifikat passen)
    if (tlsVerify)
        SSL_CTX_set_verify_depth(clientContext, 1);

    // BIO erstellen
    if (!(bio = BIO_new_ssl_connect(clientContext)))
    {
        cerr << "BIO konnte nicht erstellt werden" << endl;

        // Client beenden
        stop();

        return TLSCLIENT_ERROR_START_CREATE_BIO;
    }

    // TLS Handler aus BIO erstellen
    BIO_get_ssl(bio, &tlsSocket);

    // Mit Server verbinden
    BIO_set_conn_hostname(bio, connstring);

    // TLS Handshake durchführen
    if (1 != SSL_do_handshake(tlsSocket))
    {
        cerr << "TLS Handshake fehlgeschlagen" << endl;

        // Client beenden
        stop();

        return TLSCLIENT_ERROR_START_DO_HANDSHAKE;
    }

    if (tlsVerify)
    {
        // TLS Handshake verifizieren
        if (X509_V_OK != SSL_get_verify_result(tlsSocket))
        {
            cerr << "TLS Handshake konnte nicht verifiziert werden" << endl;

            // Client beenden
            stop();

            return TLSCLIENT_ERROR_START_VERIFY_HANDSHAKE;
        }
    }

    // Der Client läuft jetzt
    running = true;

    // Auf neue eingehende Nachrichten hören im Hintergrund
    recHandler = thread(&NetworkClient::receive, this);

    cout << "Mit Server verbunden" << endl;

    return TLSCLIENT_START_OK;
}

void NetworkClient::stop()
{
    running = false;

    // TCP Socket file descriptor auslesen
    int fd{0};
    BIO_get_fd(bio, &fd);

    // TCP Socket schließen
    if (bio)
    {
        BIO_ssl_shutdown(bio);
        bio = nullptr;
    }
    else
    {
        cerr << "Client Socket konnte nicht blockiert werden" << endl;

        return;
    }

    // Warten bis der receive Prozess beendet wurde
    if (recHandler.joinable())
        recHandler.join();

    // BIO leeren
    BIO_free_all(bio);

    // TCP Soket schließen
    close(fd);

    // Client Context wieder feigeben
    SSL_CTX_free(clientContext);
    clientContext = nullptr;

    cout << "Client beendet" << endl;

    return;
}

bool NetworkClient::sendMsg(const std::string& msg)
{
    // Nachricht in char array umwandeln (utf8)
    const char* buffer = msg.c_str();

    // Länge der zu sendenden Nachricht
    int len = msg.size();

    // Nachricht senden
    // War das Senden erfolgreicht, entspricht der Rückgabewert der Länge der Nachricht
    if (len != SSL_write(tlsSocket, buffer, len))
    {
        cerr << "Senden an Server fehlgeschlagen" << endl;

        return false;
    }

    return true;
}

void NetworkClient::receive()
{
    // Puffer für eingehende Nachrichten
    char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE] {0};

    // So lange der Client läuft ...
    while (running)
    {
        // Auf neue Nachrichten warten
        const int len = SSL_read(tlsSocket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE);

        // Ist die Länge der empfangenen Nachricht kleiner gleich 0, war das Empfangen fehlerhaft
        // Fehlerhaftes Empfangen wird behandelt, als wäre die Verbindung zu diesem Client abgerissen
        if (0 >= len)
        {
            cerr << "Verbindung zu Server abgerissen" << endl;

            continue;
        }

        // Empfangene Nachricht in String umwandeln (utf8) und zur Queue hinzufügen
        string msg(buffer, len);
        thread t(&NetworkClient::workOnMessage, this, move(msg));
        t.detach();
    }

    cout << "Keine Nachrichten mehr von Server annehmen" << endl;

    return;
}
