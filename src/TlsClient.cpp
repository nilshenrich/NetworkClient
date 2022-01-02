#include "TlsClient.h"

using namespace std;
using namespace networking;

TlsClient::TlsClient()
{
    //ctor
}

TlsClient::~TlsClient()
{
    //dtor
}

int TlsClient::init(const char *const pathToCaCert,
                    const char *const pathToCert,
                    const char *const pathToPrivKey)
{
    // Initialize OpenSSL algorithms
    OpenSSL_add_ssl_algorithms();

    // Set encryption method to latest client side TLS version (Stop client and return with error if failed)
    if (!(clientContext = SSL_CTX_new(TLS_client_method())))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when setting encryption method to latest client side TLS version" << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_SET_CONTEXT;
    }

    // Load the CA certificate the client should trust (Stop client and return with error if failed)
    if (1 != SSL_CTX_load_verify_locations(clientContext, pathToCaCert, nullptr))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when loading the CA certificate the client should trust: " << pathToCaCert << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_WRONG_CA_PATH;
    }

    // Load the client certificate (Stop client and return with error if failed)
    if (1 != SSL_CTX_use_certificate_file(clientContext, pathToCert, SSL_FILETYPE_PEM))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when loading the client certificate: " << pathToCert << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_WRONG_CERT_PATH;
    }

    // Load the client private key (Stop client and return with error if failed)
    if (1 != SSL_CTX_use_PrivateKey_file(clientContext, pathToPrivKey, SSL_FILETYPE_PEM))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when loading the client private key: " << pathToPrivKey << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_WRONG_KEY_PATH;
    }

    // Check if the client certificate and private key matches (Stop client and return with error if failed)
    if (1 != SSL_CTX_check_private_key(clientContext))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Client certificate and private key do not match" << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_WRONG_KEY;
    }

    // Set TLS mode: SSL_MODE_AUTO_RETRY
    SSL_CTX_set_mode(clientContext, SSL_MODE_AUTO_RETRY);

    // Force server to authenticate itself
    SSL_CTX_set_verify(clientContext, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

    // Server certificate must be issued directly by a trusted CA
    SSL_CTX_set_verify_depth(clientContext, 1);

    return NETWORKCLIENT_START_OK;
}

void TlsClient::deinit()
{
    SSL_CTX_free(clientContext);
    clientContext = nullptr;

    return;
}

SSL *TlsClient::connectionInit()
{
    // Create new TLS channel (Return nullptr if failed)
    SSL *tlsSocket{SSL_new(clientContext)};
    if (!tlsSocket)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when creating new TLS channel" << endl;
#endif // DEVELOP

        return nullptr;
    }

    // Bind the TLS channel to the TCP socket (Return nullptr if failed)
    if (!SSL_set_fd(tlsSocket, tcpSocket))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when binding the TLS channel to the TCP socket" << endl;
#endif // DEVELOP

        return nullptr;
    }

    // Do TLS handshake (Return nullptr if failed)
    if (1 != SSL_connect(tlsSocket))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error when doing TLS handshake" << endl;
#endif // DEVELOP

        return nullptr;
    }

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Enrypted connection to server established" << endl;
#endif // DEVELOP

    return tlsSocket;
}

string TlsClient::readMsg()
{
    // Buffer for incoming message
    char buffer[MAXIMUM_RECEIVE_PACKAGE_SIZE]{0};

    // Wait for server to send message
    const int lenMsg{SSL_read(clientSocket, buffer, MAXIMUM_RECEIVE_PACKAGE_SIZE)};

    // Return received message as string (Return empty string if receive failed)
    return string{buffer, 0 < lenMsg ? static_cast<size_t>(lenMsg) : 0UL};
}

bool TlsClient::writeMsg(const string &msg)
{
    // Get size of message to send
    const int lenMsg{(int)msg.size()};

    // Send message to server (Return false if send failed)
    return SSL_write(clientSocket, msg.c_str(), lenMsg) == lenMsg;
}

void TlsClient::workOnMessage(const string msg)
{
    workOnMessage_TlsClient(move(msg));
    return;
}
