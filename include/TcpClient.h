#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <limits>

#include "NetworkClient.h"

namespace networking
{
    class TcpClient : public NetworkClient<int>
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os                                Stream to forward incoming stream to
         * @param connectionEstablishedTimeout_ms   Connection timeout [ms]
         */
        TcpClient(std::ostream &os = std::cout, int connectionEstablishedTimeout_ms = 1000);

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter                         Character to split messages on
         * @param messageMaxLen                     Maximum message length
         * @param connectionEstablishedTimeout_ms   Connection timeout [ms]
         * @param workOnMessage                     Working function on incoming message
         */
        // TODO: Change order, so message length can use default value?
        TcpClient(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1, int connectionEstablishedTimeout_ms = 1000,
                  std::function<void(const std::string)> workOnMessage = nullptr);

        /**
         * @brief Destructor
         */
        virtual ~TcpClient();

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
         * @brief Initialize the connection
         * Just return pointer to the TCP socket
         *
         * @return int*
         */
        int *connectionInit() override final;

        /**
         * @brief Deinitialize the connection (Do nothing)
         */
        void connectionDeinit() override final;

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

        // Disallow copy
        TcpClient(const TcpClient &) = delete;
        TcpClient &operator=(const TcpClient &) = delete;
    };
}

#endif // TCPCLIENT_H
