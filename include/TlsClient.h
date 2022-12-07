#ifndef TLSCLIENT_H
#define TLSCLIENT_H

#include <limits>
#include <openssl/ssl.h>

#include "NetworkClient.h"

namespace networking
{
    /**
     * @brief Deleter for TLS object
     */
    struct NetworkClient_SSL_Deleter
    {
        void operator()(SSL *ssl)
        {
            SSL_free(ssl);
            return;
        }
    };

    /**
     * @brief Class for encrypted TLS client
     */
    class TlsClient : public NetworkClient<SSL, NetworkClient_SSL_Deleter>
    {
    public:
        /**
         * @brief Constructor for continuous stream forwarding
         *
         * @param os                                Stream to forward incoming stream to
         */
        TlsClient(std::ostream &os = std::cout);

        /**
         * @brief Constructor for fragmented messages
         *
         * @param delimiter                         Character to split messages on
         * @param messageMaxLen                     Maximum message length
         */
        TlsClient(char delimiter, size_t messageMaxLen = std::numeric_limits<size_t>::max() - 1);

        /**
         * @brief Destructor
         */
        virtual ~TlsClient();

    private:
        /**
         * @brief Initialize the client
         * Load certificates and keys
         *
         * @param pathToCaCert
         * @param pathToCert
         * @param pathToPrivKey
         * @return int
         */
        int init(const char *const pathToCaCert,
                 const char *const pathToCert,
                 const char *const pathToPrivKey) override final;

        /**
         * @brief Initialize the connection
         * Do handshake with the server and return pointer to the TLS context
         *
         * @return SSL*
         */
        SSL *connectionInit() override final;

        /**
         * @brief Deinitialize the connection (Shutdown the TLS connection)
         */
        void connectionDeinit() override final;

        /**
         * @brief Read raw data from the encrypted TLS socket
         *
         * @return std::string
         */
        std::string readMsg() override final;

        /**
         * @brief Write raw data to the encrypted TLS socket
         *
         * @param msg
         * @return true
         * @return false
         */
        bool writeMsg(const std::string &msg) override final;

        // TLS context
        std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)> clientContext{nullptr, SSL_CTX_free};

        // Disallow copy
        TlsClient(const TlsClient &) = delete;
        TlsClient &operator=(const TlsClient &) = delete;
    };
}

#endif // TLSCLIENT_H
