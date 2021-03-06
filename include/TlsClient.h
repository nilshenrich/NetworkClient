#ifndef TLSCLIENT_H
#define TLSCLIENT_H

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
        TlsClient(char delimiter = '\n');
        virtual ~TlsClient();

        /**
         * @brief Do some stuff when a new message is received
         * This method is abstract and must be implemented by derived classes
         *
         * @param tlsMsgFromServer
         */
        virtual void workOnMessage_TlsClient(const std::string tlsMsgFromServer) = 0;

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

        /**
         * @brief Just call the special receive handler for TLS (wotkOnMessage_TlsClient)
         *
         * @param msg
         */
        void workOnMessage(const std::string msg) override final;

        // TLS context
        std::unique_ptr<SSL_CTX, void (*)(SSL_CTX *)> clientContext{nullptr, SSL_CTX_free};

        // Disallow copy
        TlsClient(const TlsClient &) = delete;
        TlsClient &operator=(const TlsClient &) = delete;
    };
}

#endif // TLSCLIENT_H
