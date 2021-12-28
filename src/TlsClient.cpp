#include "TlsClient.h"

using namespace std;
using namespace networking;

TlsClient::TlsClient()
{
    //ctor
}

TlsClient::~TlsClient()
{
    //dtor
}

int TlsClient::start(const string& serverIp,
                     const int serverPort,
                     const char* const pathToCaCert,
                     const char* const pathToCert,
                     const char* const pathToPrivKey)
{
    return NetworkClient::start(serverIp,
                                serverPort,
                                pathToCaCert,
                                pathToCert,
                                pathToPrivKey,
                                SSL_VERIFY_PEER || SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                                SSL_MODE_AUTO_RETRY);
}

void TlsClient::workOnMessage(const string tlsMsgFromServer)
{
    workOnMessage_TlsClient(move(tlsMsgFromServer));
    return;
}

void TlsClient::workOnClosed()
{
    workOnClosed_TlsClient();
    return;
}
