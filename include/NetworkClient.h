#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#ifdef DEVELOP
#include <iostream>
#endif // DEVELOP

#include <string>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <thread>
#include <mutex>
#include <unistd.h>

#include "NetworkingDefines.h"

namespace networking
{
class NetworkClient
{
public:
    NetworkClient();
    virtual ~NetworkClient();

    /**
     * Client starten
     * @param serverIp
     * @param serverPort
     * @param pathToCaCert
     * @param pathToCert
     * @param pathToPrivKey
     * @param tlsVerify
     * @param tlsMode
     * @return int (0 := success, other := failed)
     */
    int start(const std::string& serverIp,
              const int serverPort,
              const char* const pathToCaCert,
              const char* const pathToCert,
              const char* const pathToPrivKey,
              const int tlsVerify,
              const unsigned int tlsMode);

    /**
     * Client beenden
     */
    void stop();

    /**
     * Senden einer Nachricht an den Server
     * @param msg
     * @return bool
     */
    bool sendMsg(const std::string& msg);

protected:

    /**
     * Wird aufgerufen, sobald eine neue Nachricht empfangen wird
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsMsgFromServer
     */
    virtual void workOnMessage(const std::string tlsMsgFromServer) = 0;

    /**
     * Wird aufgerufen, sobal die Verbindung abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     */
    virtual void workOnClosed() = 0;

private:

    /**
     * Auf eingehende Nachrichten vom Server hören
     * Diese Funktion läuft, solange die TLS Verbindung aktiv ist und wird asynchron ausgeführt
     */
    void receive();

    // Flag, die wiedergibt, ob der Client läuft
    bool running {false};

    // Client Context
    SSL_CTX* clientContext {nullptr};

    // BIO
    BIO* bio {nullptr};

    // TLS Kanal
    SSL* tlsSocket {nullptr};

    // Handler für die Hintergrundaufgabe, eingehende Nachrichten zu empfangen
    std::thread recHandler {};

    // Maximale Größe eines übertragenen Pakets
    static const int MAXIMUM_RECEIVE_PACKAGE_SIZE {16384};

    // Objekt kann nicht kopert werden
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator = (const NetworkClient&) = delete;
};
}

#endif // NETWORKCLIENT_H
