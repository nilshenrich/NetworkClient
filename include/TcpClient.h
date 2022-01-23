#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "NetworkClient.h"

namespace networking
{
    class TcpClient : public NetworkClient<int>
    {
    public:
        TcpClient();
        virtual ~TcpClient();

        /**
         * @brief Do some stuff when a new message is received
         * This method is abstract and must be implemented by derived classes
         * 
         * @param tcpMsgFromServer 
         */
        virtual void workOnMessage_TcpClient(const std::string tcpMsgFromServer) = 0;

    private:
        /**
         * @brief Initialize the client
         * Do nothing for TCP client
         * 
         * @return int 
         */
        int init(const char *const,
                 const char *const,
                 const char *const) override final;

        /**
         * @brief Deinitialize the client
         * Do nothing for TCP client
         */
        void deinit() override final;

        /**
         * @brief Initialize the connection
         * Just return pointer to the TCP socket
         * 
         * @return int* 
         */
        int connectionInit(int *socket) override final;

        /**
         * @brief Read raw data from the unencrypted TCP socket
         * 
         * @return std::string 
         */
        std::string readMsg() override final;

        /**
         * @brief Send raw data to the unencrypted TCP socket
         * 
         * @param msg 
         * @return true 
         * @return bool 
         */
        bool writeMsg(const std::string &msg) override final;

        /**
         * @brief Just call the special receive handler for TCP (wotkOnMessage_TcpClient)
         * 
         * @param msg 
         */
        void workOnMessage(const std::string msg) override final;

        // Disallow copy
        TcpClient(const TcpClient &) = delete;
        TcpClient &operator=(const TcpClient &) = delete;
    };
}

#endif // TCPCLIENT_H
