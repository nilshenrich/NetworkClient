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
#include <memory>
#include <limits>
#include <atomic>
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
     * @brief Class to manage running flag in threads.
     *
     */
    using RunningFlag = std::atomic_bool;
    class NetworkClient_running_manager
    {
    public:
        NetworkClient_running_manager(RunningFlag &flag) : flag{flag} {}
        virtual ~NetworkClient_running_manager()
        {
            flag = false;
        }

    private:
        RunningFlag &flag;

        // Delete default constructor
        NetworkClient_running_manager() = delete;

        // Disallow copy
        NetworkClient_running_manager(const NetworkClient_running_manager &) = delete;
        NetworkClient_running_manager &operator=(const NetworkClient_running_manager &) = delete;
    };

    /**
     * @brief Template class for the NetworkClient class.
     *
     * @param SocketType
     * @param SocketDeleter
     */
    template <class SocketType, class SocketDeleter = std::default_delete<SocketType>>
    class NetworkClient
    {
    public:
        NetworkClient(char delimiter, size_t messageMaxLen, int connectionEstablishedTimeout_ms)
            : DELIMITER_FOR_FRAGMENTATION{delimiter},
              MAXIMUM_MESSAGE_LENGTH{messageMaxLen},
              CONNECTION_ESTABLISHED_TIMEOUT_ms{connectionEstablishedTimeout_ms} {}
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
         * @brief Initialize the connection to the server.
         * This method is abstract and must be implemented by derived classes.
         *
         * @return SocketType*
         */
        virtual SocketType *connectionInit() = 0;

        /**
         * @brief Deinitialize the connection to the server.
         * This method is abstract and must be implemented by derived classes.
         */
        virtual void connectionDeinit() = 0;

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
        std::unique_ptr<SocketType, SocketDeleter> clientSocket{nullptr};

        // Maximum package size for receiving data
        const static int MAXIMUM_RECEIVE_PACKAGE_SIZE{16384};

    private:
        /**
         * @brief Read incoming data from the server connection.
         * This method runs infinitely until the client is stopped.
         */
        void receive();

        // Flag to indicate if the client is running
        RunningFlag running{false};

        // Client socket address
        struct sockaddr_in socketAddress
        {
        };

        // Thread for receiving data from the server
        std::thread recHandler{};

        // timeout thread for waiting for connection established marker
        std::thread estConnTimeoutHandler{};

        // All working threads and their running status
        std::vector<std::thread> workHandlers;
        std::vector<std::unique_ptr<RunningFlag>> workHandlersRunning;

        // Delimiter for the message framing (incoming and outgoing) (default is '\n')
        const char DELIMITER_FOR_FRAGMENTATION;

        // Maximum message length (incoming and outgoing) (default is 2³² - 2 = 4294967294)
        const size_t MAXIMUM_MESSAGE_LENGTH;

        // Timeout for waiting for connection established marker (default is 1 second)
        const std::chrono::milliseconds CONNECTION_ESTABLISHED_TIMEOUT_ms;

        // Disallow copy
        NetworkClient() = delete;
        NetworkClient(const NetworkClient &) = delete;
        NetworkClient &operator=(const NetworkClient &) = delete;
    };

    // ============================== Implementation of non-abstract methods. ==============================
    // ====================== Must be in header file because of the template class. =======================

#include "../src/NetworkClient.tpp"
}

#endif // NETWORKCLIENT_H
