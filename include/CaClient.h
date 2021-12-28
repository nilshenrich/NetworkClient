#ifndef CACLIENT_H
#define CACLIENT_H

#include "NetworkClient.h"

namespace networking
{
class CaClient: public NetworkClient
{
public:
    CaClient();
    virtual ~CaClient();

    /**
     * TLS Client starten
     * @param serverIp
     * @param serverPort
     * @return int (0 := success, other := failed)
     */
    int start(const std::string& serverIp, const int serverPort);

    /**
     * Wird aufgerufen, sobald eine neue Nachricht empfangen wird
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param caMsgFromServer
     */
    virtual void workOnMessage_CaClient(const std::string caMsgFromServer) = 0;

    /**
     * Wird aufgerufen, sobal die Verbindung abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     */
    virtual void workOnClosed_CaClient() = 0;

private:

    /**
     * Wird aufgerufen, sobald eine neue Nachricht empfangen wird
     * Diese Methode muss von erbenden Klassen überschrieben werden
     * @param tlsMsgFromServer
     */
    void workOnMessage(const std::string caMsgFromServer);

    /**
     * Wird aufgerufen, sobal die Verbindung abreißt
     * Diese Methode muss von erbenden Klassen überschrieben werden
     */
    void workOnClosed();

    // Start der Basisklasse entfernen
    int start(const std::string&, const int, const char* const, const char* const, const char* const, const int, const unsigned int) = delete;

    // Object kann nicht kopiert werden
    CaClient(const CaClient&) = delete;
    CaClient& operator = (const CaClient&) = delete;
};
}

#endif // CACLIENT_H
