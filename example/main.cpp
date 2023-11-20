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
#include <thread>

#include "NetworkClient/TcpClient.h"
#include "NetworkClient/TlsClient.h"

using namespace std;
using namespace networking;

// Global functions
void tcp_fragmented_workOnMessage(string msg) { cout << "Message from TCP server: " << msg << endl; }
void tls_fragmented_workOnMessage(string msg) { cout << "Message from TLS server: " << msg << endl; }

void printHelp()
{
    cout << "What mode shall be used?" << endl
         << "    c: Continuous stream" << endl
         << "    f: Fragmented messages" << endl
         << "    q: Exit program" << endl;
}

int main()
{
    printHelp();

    // User decision
    while (true)
    {
        char decision;
        cin >> decision;
        switch (decision)
        {
        case 'c':
        case 'C':
        {
            ofstream ofs_tcp{"MessageStream_TCP_Server"};
            ofstream ofs_tls{"MessageStream_TLS_Server"};
            TcpClient tcpClient{ofs_tcp};
            TlsClient tlsClient{ofs_tls};
            tcpClient.start("localhost", 8081);
            tlsClient.start("localhost", 8082, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
            this_thread::sleep_for(50ms); // Wait a short time for connection to be established
            tcpClient.sendMsg("Hello TCP server! - forwarding mode");
            tlsClient.sendMsg("Hello TLS server! - forwarding mode");
            this_thread::sleep_for(100ms); // Wait short time before closing connection
            break;
        }

        case 'f':
        case 'F':
        {
            TcpClient tcpClient{'\n'};
            TlsClient tlsClient{'\n'};
            tcpClient.setWorkOnMessage(&tcp_fragmented_workOnMessage);
            tlsClient.setWorkOnMessage(&tls_fragmented_workOnMessage);
            tcpClient.start("localhost", 8081);
            tlsClient.start("localhost", 8082, "../keys/ca/ca_cert.pem", "../keys/client/client_cert.pem", "../keys/client/client_key.pem");
            this_thread::sleep_for(50ms); // Wait a short time for connection to be established
            tcpClient.sendMsg("Hello TCP server! - fragmentation mode");
            tlsClient.sendMsg("Hello TLS server! - fragmentation mode");
            this_thread::sleep_for(100ms); // Wait short time before closing connection
            break;
        }

        case 'q':
        case 'Q':
            cout << "Exit example program" << endl;
            return 0;

        default:
            printHelp();
            break;
        }
    }
}
