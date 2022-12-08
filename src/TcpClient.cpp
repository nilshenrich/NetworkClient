#include "TcpClient.h"

using namespace std;
using namespace networking;

TcpClient::TcpClient(std::ostream &os) : NetworkClient(os) {}
TcpClient::TcpClient(char delimiter, size_t messageMaxLen) : NetworkClient(delimiter, messageMaxLen) {}

TcpClient::~TcpClient()
{
    stop();
}

int TcpClient::init(const char *const,
                    const char *const,
                    const char *const)
{
    return 0;
}

int *TcpClient::connectionInit()
{
    return new int{tcpSocket};
}

void TcpClient::connectionDeinit()
{
    return;
}

string TcpClient::readMsg()
{
    // Buffer to store the data received from the server
    char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

    // Wait for the server to send data
    ssize_t lenMsg{recv(tcpSocket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE, 0)};

    // Return the received message as a string (Empty string if receive failed)
    return string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
}

bool TcpClient::writeMsg(const string &msg)
{
#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Send to server: " << msg << endl;
#endif // DEVELOP

    const size_t lenMsg{msg.size()};
    return send(tcpSocket, msg.c_str(), lenMsg, 0) == (ssize_t)lenMsg;
}
