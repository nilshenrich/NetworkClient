/**
 * @file example.cpp
 * @author Nils Henrich
 * @brief Example how to use the networking library
 * @version 1.0
 * @date 2022-01-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <iostream>
#include <chrono>

#include "NetworkClient/TcpClient.h"
#include "NetworkClient/TlsClient.h"

using namespace std;
using namespace networking;

// Example class that derives from TcpClient and TlsClient
class ExampleClient : private TcpClient, private TlsClient
{
public:
    // Constructor and destructor
    ExampleClient() {}
    virtual ~ExampleClient() {}

    // Start TCP and TLS client
    int start()
    {
        // Start TCP client
        TcpClient::start("localhost", 8081);

        // Start TLS client
        return TlsClient::start("localhost", 8082, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
    }

    // Stop TCP and TLS client
    void stop()
    {
        // Stop TCP server
        TcpClient::stop();

        // Stop TLS server
        TlsClient::stop();

        return;
    }

    // Send unencrypted data to server
    void sendMsg_Tcp(const string &msg)
    {
        // Send message to TCP server
        TcpClient::sendMsg(msg);

        return;
    }

    // Send encrypted data to server
    void sendMsg_Tls(const string &msg)
    {
        // Send message to TLS server
        TlsClient::sendMsg(msg);

        return;
    }

private:
    // Override abstract methods
    void workOnMessage_TcpClient(const std::string tcpMsgFromServer)
    {
        cout << "Message from TCP client: " << tcpMsgFromServer << endl;
        return;
    }

    void workOnMessage_TlsClient(const std::string tlsMsgFromServer)
    {
        cout << "Message from TLS client: " << tlsMsgFromServer << endl;
        return;
    }
};

int main()
{
    // Create client
    ExampleClient client;

    // Start client
    int start{client.start()};
    if (start)
    {
        cerr << "Error when starting client: " << start << endl;
        return start;
    }

    cout << "Client started." << endl;

    // Send TCP message to server
    client.sendMsg_Tcp("Hello TCP server!");

    // Send TLS message to server
    client.sendMsg_Tls("Hello TLS server!");

    // Wait for one second
    this_thread::sleep_for(1s);

    // Stop client
    client.stop();

    cout << "Client stopped." << endl;

    return 0;
}
