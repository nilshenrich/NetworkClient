/**
 * @file NetworkClient.h
 * @author Nils Henrich
 * @brief Base framework for all classes that build a network client based on TCP.
 * This class contains no functionality, but serves a base framework for the creation of stable clients based on TCP.
 * When compiling with the -DDEBUG flag, the class will print out all received messages to the console.
 * @version 1.0
 * @date 2021-12-28
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#ifdef DEVELOP
#include <iostream>
#endif // DEVELOP

#include <string>
#include <vector>
#include <cstring>
#include <thread>
#include <mutex>
#include <memory>
#include <limits>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include "NetworkingDefines.h"

namespace networking
{
    /**
     * @brief Exception class for the NetworkClient class.
     */
    class NetworkClient_error : public std::exception
    {
    public:
        NetworkClient_error(std::string msg = "unexpected networking error") : msg{msg} {}
        virtual ~NetworkClient_error() {}

        const char *what()
        {
            return msg.c_str();
        }

    private:
        std::string msg;

        // Delete default constructor
        NetworkClient_error() = delete;

        // Disallow copy
        NetworkClient_error(const NetworkClient_error &) = delete;
        NetworkClient_error &operator=(const NetworkClient_error &) = delete;
    };

    /**
     * @brief Class to manage running flag in threds.
     *
     */
    class NetworkClient_running_manager
    {
    public:
        NetworkClient_running_manager(bool &flag) : flag{flag}
        {
            flag = true;
        }
        virtual ~NetworkClient_running_manager()
        {
            flag = false;
        }

    private:
        bool &flag;

        // Delete default constructor
        NetworkClient_running_manager() = delete;

        // Disallow copy
        NetworkClient_running_manager(const NetworkClient_running_manager &) = delete;
        NetworkClient_running_manager &operator=(const NetworkClient_running_manager &) = delete;
    };

    /**
     * @brief Template class for the NetworkClient class.
     *
     * @tparam SocketType
     * @tparam SocketDeleter
     */
    template <class SocketType, class SocketDeleter = std::default_delete<SocketType>>
    class NetworkClient
    {
    public:
        NetworkClient(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1) : DELIMITER{delimiter}, MAXIMUM_MESSAGE_LENGTH{messageMaxLen} {}
        virtual ~NetworkClient() {}

        /**
         * @brief Start the client and connects to the server.
         * If connection to listener succeeds, this method returns NETWORKCLIENT_START_OK, otherwise it returns an error code.
         *
         * @param serverIp
         * @param serverPort
         * @param pathToCaCert
         * @param pathToCert
         * @param pathToPrivKey
         * @return int
         */
        int start(const std::string &serverIp,
                  const int serverPort,
                  const char *const pathToCaCert = nullptr,
                  const char *const pathToCert = nullptr,
                  const char *const pathToPrivKey = nullptr);

        /**
         * @brief Stop the client and disconnects from the server.
         */
        void stop();

        /**
         * @brief Send a message to the server if connected.
         *
         * @param msg
         * @return true
         * @return false
         */
        bool sendMsg(const std::string &msg);

        /**
         * @brief Return if client is running
         *
         * @return bool (true if running, false if not)
         */
        bool isRunning() const;

    protected:
        /**
         * @brief Initialize the client.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param pathToCaCert
         * @param pathToCert
         * @param pathToPrivKey
         * @return int
         */
        virtual int init(const char *const pathToCaCert,
                         const char *const pathToCert,
                         const char *const pathToPrivKey) = 0;

        /**
         * @brief Deinitialize the client.
         * This method is abstract and must be implemented by derived classes.
         */
        virtual void deinit() = 0;

        /**
         * @brief Initialize the connection to the server.
         * This method is abstract and must be implemented by derived classes.
         *
         * @return SocketType*
         */
        virtual SocketType *connectionInit() = 0;

        /**
         * @brief Read raw received data from the server connection.
         * This method is expected to return the received data as a string with blocking read (Empty string means failure).
         * This method is abstract and must be implemented by derived classes.
         *
         * @return std::string
         */
        virtual std::string readMsg() = 0;

        /**
         * @brief Write raw data to the server connection.
         * This method is expected to return true if the data was written successfully, otherwise false.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param msg
         * @return true
         * @return false
         */
        virtual bool writeMsg(const std::string &msg) = 0;

        /**
         * @brief Do some stuff when a new message is received from the server.
         * This method is called automatically as soon as a new message is received from the server.
         * This method is abstract and must be implemented by derived classes.
         *
         * @param msg
         */
        virtual void workOnMessage(const std::string msg) = 0;

        // Client sockets (TCP and user defined)
        int tcpSocket;
        SocketType *clientSocket{nullptr};

        // Maximum package size for receiving data
        const static int MAXIMUM_RECEIVE_PACKAGE_SIZE{16384};

    private:
        /**
         * @brief Read incoming data from the server connection.
         * This method runs infinitely until the client is stopped.
         */
        void receive();

        // Flag to indicate if the client is running
        bool running{false};

        // Client socket address
        struct sockaddr_in socketAddress
        {
        };

        // Thread for receiving data from the server
        std::thread recHandler{};

        // All working threads and their running status
        std::vector<std::thread> workHandlers;
        std::vector<std::unique_ptr<bool>> workHandlersRunning;

        // Delimiter for the message framing (incoming and outgoing) (default is '\n')
        const char DELIMITER;

        // Maximum message length (incoming and outgoing) (default is 2³² - 2 = 4294967294)
        const size_t MAXIMUM_MESSAGE_LENGTH;

        // Disallow copy
        NetworkClient() = delete;
        NetworkClient(const NetworkClient &) = delete;
        NetworkClient &operator=(const NetworkClient &) = delete;
    };

    template <class SocketType, class SocketDeleter>
    int NetworkClient<SocketType, SocketDeleter>::start(
        const std::string &serverIp,
        const int serverPort,
        const char *const pathToCaCert,
        const char *const pathToCert,
        const char *const pathToPrivKey)
    {
        using namespace std;

        // Check if client is already running
        // If so, return with error
        if (running)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Client already running" << endl;
#endif // DEVELOP

            return -1;
        }

        // Check if the port number is valid
        if (1 > serverPort || 65535 < serverPort)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": The port " << serverPort << " couldn't be used" << endl;
#endif // DEVELOP

            return NETWORKCLIENT_ERROR_START_WRONG_PORT;
        }

        // Initialize the client
        // If initialization fails, return with error
        int initCode{init(pathToCaCert, pathToCert, pathToPrivKey)};
        if (initCode)
            return initCode;

        // Create the client tcp socket and connect to the server
        // If socket creation fails, stop client and return with error
        tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == tcpSocket)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error while creating client TCP socket" << endl;
#endif // DEVELOP

            stop();
            return NETWORKCLIENT_ERROR_START_CREATE_SOCKET;
        }

        // Set the socket options
        // If setting fails, stop client and return with error
        int opt{0};
        if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error while setting TCP socket options" << endl;
#endif // DEVELOP

            stop();
            return NETWORKCLIENT_ERROR_START_SET_SOCKET_OPT;
        }

        // Initialize the socket address
        struct hostent *serverHost;
        serverHost = gethostbyname(serverIp.c_str());
        memset(&socketAddress, 0, sizeof(socketAddress));
        socketAddress.sin_family = AF_INET;
        socketAddress.sin_addr.s_addr = *(uint32_t *)serverHost->h_addr_list[0];
        socketAddress.sin_port = htons(serverPort);

        // Connect to the server via unencrypted TCP
        // If connection fails, stop client and return with error
        if (connect(tcpSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)))
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Error while connecting to server" << endl;
#endif // DEVELOP

            stop();
            return NETWORKCLIENT_ERROR_START_CONNECT;
        }

        // Initialize the TCP connection to the server
        // If initialization fails, stop client and return with error
        clientSocket = connectionInit();
        if (!clientSocket)
        {
            stop();
            return NETWORKCLIENT_ERROR_START_CONNECT_INIT;
        }

        // Receive incoming data from the server infinitely in the background while the client is running
        // If background task already exists, return with error
        if (recHandler.joinable())
            throw NetworkClient_error("Error while starting background receive task. Background task already exists");
        recHandler = thread{&NetworkClient::receive, this};

        // Client is now running
        running = true;

        return NETWORKCLIENT_START_OK;
    }

    template <class SocketType, class SocketDeleter>
    void NetworkClient<SocketType, SocketDeleter>::stop()
    {
        using namespace std;

        // Stop the client
        running = false;

        // Block the TCP socket to abort receiving process
        int shut{shutdown(tcpSocket, SHUT_RDWR)};

        // Wait for the background receive thread to finish
        if (recHandler.joinable())
            recHandler.join();

        // If shutdown failed, abort stop here
        if (shut)
            return;

        // Close the TCP socket
        close(tcpSocket);

        // Deinitialize the client
        deinit();

        return;
    }

    template <class SocketType, class SocketDeleter>
    bool NetworkClient<SocketType, SocketDeleter>::sendMsg(const std::string &msg)
    {
        using namespace std;

        // Check if message doesn't contain delimiter
        if (msg.find(DELIMITER) != string::npos)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Message contains delimiter" << endl;
#endif // DEVELOP

            return false;
        }

        // Check if message is too long
        if (msg.length() > MAXIMUM_MESSAGE_LENGTH)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Message is too long" << endl;
#endif // DEVELOP

            return false;
        }

        // Send the message to the server with leading and trailing characters to indicate the message length
        if (running)
            return writeMsg(msg + string{DELIMITER});

#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Client not running" << endl;
#endif // DEVELOP

        return false;
    }

    template <class SocketType, class SocketDeleter>
    void NetworkClient<SocketType, SocketDeleter>::receive()
    {
        using namespace std;

        // Do receive loop until client is stopped
        string buffer;
        while (1)
        {
            // Wait for incoming data from the server
            // This method blocks until data is received
            // An empty string is returned if the connection is crashed
            string msg{readMsg()};
            if (msg.empty())
            {
#ifdef DEVELOP
                cout << typeid(this).name() << "::" << __func__ << ": Connection to server lost" << endl;
#endif // DEVELOP

                // Stop the client
                running = false;

                // Wait for all work handlers to finish
                for (auto &it : workHandlers)
                    it.join();

                // Block the TCP socket to abort receiving process
                // If shutdown failed, abort stop here
                if (shutdown(tcpSocket, SHUT_RDWR))
                    return;

                // Close the TCP socket
                close(tcpSocket);

                // Deinitialize the client
                deinit();

                return;
            }

            // Get raw message separated by delimiter
            size_t delimiter_pos{msg.find(DELIMITER)};
            if (string::npos == delimiter_pos)
            {
                // If delimiter is not found, the whole packet is part of the message
                buffer += msg;
            }
            else
            {
                // If delimiter is found, the message is split into two parts
                do
                {
                    buffer += msg.substr(0, delimiter_pos);
                    msg = msg.substr(delimiter_pos + 1);
                    delimiter_pos = msg.find(DELIMITER);

#ifdef DEVELOP
                    cout << typeid(this).name() << "::" << __func__ << ": Received message from server: " << buffer << endl;
#endif // DEVELOP

                    unique_ptr<bool> workRunning{new bool{true}};
                    thread work_t{[this](bool *workRunning_p, string buffer)
                                  {
                                      // Mark thread as running
                                      NetworkClient_running_manager running_mgr{*workRunning_p};

                                      // Run code to handle the incoming message
                                      workOnMessage(move(buffer));

                                      return;
                                  },
                                  workRunning.get(), move(buffer)};

                    // Remove all finished work handlers from the vector
                    size_t workHandlers_s{workHandlersRunning.size()};
                    for (size_t i{0}; i < workHandlers_s; i += 1)
                    {
                        if (!*workHandlersRunning[i].get())
                        {
                            workHandlers[i].join();
                            workHandlers.erase(workHandlers.begin() + i);
                            workHandlersRunning.erase(workHandlersRunning.begin() + i);
                            i -= 1;
                            workHandlers_s -= 1;
                        }
                    }

                    workHandlers.push_back(move(work_t));
                    workHandlersRunning.push_back(move(workRunning));
                } while (string::npos != delimiter_pos);
            }
        }
    }

    template <class SocketType, class SocketDeleter>
    bool NetworkClient<SocketType, SocketDeleter>::isRunning() const
    {
        return running;
    }
}

#endif // NETWORKCLIENT_H
