/**
 * @file NetworkingDefines.h
 * @author Nils Henrich
 * @brief Basic definitions for the network client
 * @version 1.0
 * @date 2021-12-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef NETWORKINGDEFINES_H_INCLUDED
#define NETWORKINGDEFINES_H_INCLUDED

namespace networking
{
    enum : int
    {
        NETWORKCLIENT_START_OK = 0,                     // Client started successfully
        NETWORKCLIENT_ERROR_START_WRONG_PORT = 10,      // Client could not start because of wrong port number
        NETWORKCLIENT_ERROR_START_SET_CONTEXT = 20,     // Client could not start because of SSL context error
        NETWORKCLIENT_ERROR_START_WRONG_CA_PATH = 30,   // Client could not start because of wrong path to CA cert file
        NETWORKCLIENT_ERROR_START_WRONG_CERT_PATH = 31, // Client could not start because of wrong path to certifcate file
        NETWORKCLIENT_ERROR_START_WRONG_KEY_PATH = 32,  // Client could not start because of wrong path to key file
        NETWORKCLIENT_ERROR_START_WRONG_CA = 33,        // Client could not start because of bad CA cert file
        NETWORKCLIENT_ERROR_START_WRONG_CERT = 34,      // Client could not start because of bad certificate file
        NETWORKCLIENT_ERROR_START_WRONG_KEY = 35,       // Client could not start because of bad key file or non matching key with certificate
        NETWORKCLIENT_ERROR_START_CREATE_SOCKET = 40,   // Client could not start because of TCP socket creation error
        NETWORKCLIENT_ERROR_START_SET_SOCKET_OPT = 41,  // Client could not start because of TCP socket options error
        NETWORKCLIENT_ERROR_START_CONNECT = 50,         // Client could not start because of TCP socket connection error
        NETWORKCLIENT_ERROR_START_DO_HANDSHAKE = 60,    // Client could not start because of TLS handshake error (server side)
        NETWORKCLIENT_ERROR_START_VERIFY_HANDSHAKE = 61 // Client could not start because of TLS handshake error (client side)
    };

    enum : char
    {
        NETWORKCLIENT_CHAR_TRANSFER_START = '\x02',
        NETWORKCLIENT_CHAR_TRANSFER_END = '\x03'
    };
}

#endif // NETWORKINGDEFINES_H_INCLUDED
