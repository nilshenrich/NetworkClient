#include "CaClient.h"

using namespace std;
using namespace networking;

CaClient::CaClient()
{
//ctor
}

CaClient::~CaClient()
{
    //dtor
}

int CaClient::start(const string& serverIp, const int serverPort)
{
    return NetworkClient::start(serverIp, serverPort, nullptr, nullptr, nullptr, SSL_VERIFY_NONE, SSL_MODE_AUTO_RETRY);
}

void CaClient::workOnMessage(string caMsgFromServer)
{
    workOnMessage_CaClient(move(caMsgFromServer));
    return;
}

void CaClient::workOnClosed()
{
    workOnClosed_CaClient();
    return;
}
