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
#include <fstream>
#include <chrono>

#include "NetworkClient/TcpClient.h"
#include "NetworkClient/TlsClient.h"

using namespace std;
using namespace networking;

// File streams for continuous stream forwarding
ofstream StreamForward_tcp{"output_tcp.txt"};
ofstream StreamForward_tls{"output_tls.txt"};

// Example class for fragmented message transfer that derives from TcpClient and TlsClient
class ExampleClient_fragmented : private TcpClient, private TlsClient
{
public:
    // Constructor and destructor
    ExampleClient_fragmented() : TcpClient{'\x00'}, TlsClient{'\x00'} {}
    virtual ~ExampleClient_fragmented() {}

    // Start TCP and TLS client
    int start()
    {
        // Start TCP client
        int start_tcp{TcpClient::start("localhost", 8081)};

        // Start TLS client
        int start_tls{TlsClient::start("localhost", 8082, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem")};

        // Return code (2 bytes): High byte: TLS, low byte: TCP
        return (start_tls << 8) | start_tcp;
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
        cout << "Message from TCP server: " << tcpMsgFromServer << endl;
        return;
    }

    void workOnMessage_TlsClient(const std::string tlsMsgFromServer)
    {
        cout << "Message from TLS server: " << tlsMsgFromServer << endl;
        return;
    }
};

// Example class for continuous message transfer that derives from TcpClient and TlsClient
class ExampleClient_continuous : private TcpClient, private TlsClient
{
public:
    // Constructor and destructor
    ExampleClient_continuous() : TcpClient{StreamForward_tcp}, TlsClient{StreamForward_tls} {}
    virtual ~ExampleClient_continuous() {}

    // Start TCP and TLS client
    int start()
    {
        // Start TCP client
        int start_tcp{TcpClient::start("localhost", 8081)};

        // Start TLS client
        int start_tls{TlsClient::start("localhost", 8082, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem")};

        // Return code (2 bytes): High byte: TLS, low byte: TCP
        return (start_tls << 8) | start_tcp;
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
        cerr << "Work on TCP message should never be executed" << endl;
        return;
    }

    void workOnMessage_TlsClient(const std::string tlsMsgFromServer)
    {
        cerr << "Work on TLS message should never be executed" << endl;
        return;
    }
};

int main()
{
    // Create clients
    ExampleClient_fragmented client_fragmented;
    ExampleClient_continuous client_continuous;

    // Start client
    int start{client_fragmented.start() & client_continuous.start()};
    if (start)
    {
        cerr << "Error when starting client:s " << start << endl;
        return start;
    }

    cout << "Clients started." << endl;

    char command;
    cout << "Please select command:" << endl
         << "    - F: Fragmented mode" << endl
         << "    - C: Continuous mode" << endl;
    cin >> command;
    switch (command)
    {
    case 'F':
    case 'f':
        client_fragmented.sendMsg_Tcp("Hello fragmented TCP server!");
        client_fragmented.sendMsg_Tls("Hello fragmented TLS server!");
        break;

    case 'C':
    case 'c':
        client_continuous.sendMsg_Tcp("Hello continuous TCP server!");
        client_continuous.sendMsg_Tls("Hello continuous TLS server!");
    }

    // Stop clients
    client_fragmented.stop();
    client_continuous.stop();

    cout << "Clients stopped." << endl;

    StreamForward_tcp.flush();
    StreamForward_tls.flush();

    return 0;
}
