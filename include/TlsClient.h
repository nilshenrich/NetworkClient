#ifndef TLSCLIENT_H
#define TLSCLIENT_H

#include "NetworkClient.h"

namespace networking
{
class TlsClient: public NetworkClient
{
public:
    TlsClient();
    virtual ~TlsClient();

    /**
     * TLS Client starten
     * @param serverIp
     * @param serverPort
     * @param pathToCaCert
     * @param pathToCert
     * @param pathToPrivKey
     * @return int (0 := success, other := failed)
     */
    int start(const std::string& serverIp,
              const int serverPort,
              const char* const pathToCaCert,
              const char* const pathToCert,
              const char* const pathToPrivKey);

    /**
     * Wird aufgerufen, sobald eine neue Nachricht empfangen wird
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsMsgFromServer
     */
    virtual void workOnMessage_TlsClient(const std::string tlsMsgFromServer) = 0;

    /**
     * Wird aufgerufen, sobal die Verbindung abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     */
    virtual void workOnClosed_TlsClient() = 0;

private:

    /**
     * Wird aufgerufen, sobald eine neue Nachricht empfangen wird
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsMsgFromServer
     */
    void workOnMessage(const std::string tlsMsgFromServer);

    /**
     * Wird aufgerufen, sobal die Verbindung abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     */
    void workOnClosed();

    // Start der Basisklasse entfernen
    int start(const std::string&, const int, const char* const, const char* const, const char* const, const int, const unsigned int) = delete;

    // Object kann nicht kopiert werden
    TlsClient(const TlsClient&) = delete;
    TlsClient& operator = (const TlsClient&) = delete;
};
}

#endif // TLSCLIENT_H
