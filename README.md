# NetworkClient

Installable package to set up a client that can connect to a server on TCP level and send and receive data asynchronously. The connection can be established with unencrypted pure TCP or with encrypted and two-way-authenticated TLS.

The compatible server can be found [here](https://github.com/nilshenrich/NetworkListener).

## Table of contents

1. [General explanation](#general-explanation)
    1. [Specifications](#specifications)
1. [Installation](#installation)
1. [Usage](#usage)
    1. [Non-abstract Methods](#non-abstract-methods)
1. [Example](#example)
    1. [Get certificates](#get-certificates)
    1. [Run example](#run-example)
1. [System requirements](#system-requirements)
1. [Known issues](#known-issues)
    1. [Success on server rejection](#success-on-server-rejection)

## General explanation

This project contains installable C++ libraries for a client on TCP level that can connect to a server and send and receive data asynchronously.

This package contains two libraries: **libnetworkClientTcp** and **libnetworkClientTls**.\
As the names say, **libnetworkClientTcp** creates a simple TCP client with no security. The **libnetworkClientTls** creates a client on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and the server is forced to authenticate itself.

### Specifications

1. Maximum message size (Sending and receiving):  **std::string::max_size() - 1** (2³² - 2 (4294967294) for most systems)

## Installation

As already mentioned in [General explanation](#general-explanation), this project contains two installable libraries **libnetworkClientTcp** and **libnetworkClientTls**. These libraries can be installed this way:

1. Install **build-essential**, **cmake**, **openssl** and **libssl-dev**:

    ```console
    sudo apt install build-essential cmake
    ```

    **build-essential** contains compilers and libraries to compile C/C++ code on debian-based systems.\
    **cmake** is used to easily create a Makefile.
    - **openssl** is a great toolbox for any kind of tasks about encryption and certification. It is here used to create certificates and keys used for encryption with TLS.
    - **libssl-dev** is the openssl library for C/C++.

1. Create a new folder **build** in the repositories root directory where everything can be compiled to:

    ```console
    mkdir build
    ```

1. Navigate to this newly created **build** folder and run the compilation process:

    ```console
    cd build
    cmake ..
    make
    ```

    *To get information printed on the screen, the package can be built in debug mode (Not recommended when installing libraries) by setting the define: **cmake&#160;&#x2011;DCMAKE_BUILD_TYPE=Debug&#160;..** (Same when compiling example)*

1. To install the created libraries and header files to your system, run

    ```console
    sudo make install
    ```

1. (optional) In some systems the library paths need to be updated before **libnetworkClientTcp** and **libnetworkClientTls** can be used. So if you get an error like

    ```console
    ./example: error while loading shared libraries: libnetworkClientTcp.so.1: cannot open shared object file: No such file or directory
    ```

    please run

    ```console
    sudo /sbin/ldconfig
    ```

Now the package is reade to use. Please see the example for how to use it.

## Usage

*In the subfolder [example](https://github.com/nilshenrich/NetworkClient/blob/main/include/NetworkingDefines.h) you can find a good and simple example program that shows how to use the package*

To use this package, a new class must be created deriving from **TcpClient** or **TlsClient** or both. These two classes are abstract, so an object of one of these raw types can't be created.

In this case, I would recommend a private derivation, because all **TcpClient**/**TlsClient** methods are not meant to be used in other places than a direct child class.

1. Create a new class derived from **TcpClient** and **TlsClient**:

    ```cpp
    #include "NetworkListener/TcpClient.h"
    #include "NetworkListener/TlsClient.h"

    using namespace std;
    using namespace networking;

    // New class with TCP and TLS client functionality
    class ExampleClient : private TcpClient, private TlsClient
    {
    public:
        // Constructor and destructor
        ExampleClient() {}
        virtual ~ExampleClient() {}
    };
    ```

1. Implement abstract methods from base classes
    1. Work on message over TCP (unencrypted):

        ```cpp
        void workOnMessage_TcpClient(const std::string tcpMsgFromServer)
        {
            // Do some stuff when message is received
        }
        ```

        This method is called automatically as soon as a new message from the server is received over an unencrypted TCP connection.

        The **tcpMsgFromServer** parameter contains the received message as raw string.

        This method is started in its own thread. So feel free to insert time intensive code here, the client continues running in parallel. But please make sure, your inserted code is thread safe (e.g. use [mutex](https://en.cppreference.com/w/cpp/thread/mutex)), so multiple executions of this method at the same time don't lead to a program crash.

    1. Work on message over TLS (encrypted):

        ```cpp
        void workOnMessage_TlsClient(const std::string tlsMsgFromServer)
        {
            // Do some stuff when message is received
        }
        ```

        This method is just the same as **workOnMessage_TcpClient**, but for receiving an encrypted message over a TLS connection.

    **!! Please do never call one of these 2 abstract methods somewhere in your code. These methods are automatically called by the NetworkClient library.**

    *Please note that all parameters of these abstract methods are **const**, so they can't be changed. If you need to do a message adaption, but don't want to copy the whole string for performance reasons, use the **move**-constructor:*

    ```cpp
    std::string modifiable = std::move(tcpMsgFromServer);
    modifiable += '\n'; // Message modification
    ```

After these two steps your program is ready to be compiled.\
But there are some further methods worth knowing about.

### Non-abstract Methods

1. start():

    The **start**-method is used to start a TCP or TLS client. When this method returns 0, the client runs in the background. If the return value is other that 0, please see [NetworkingDefines.h](https://github.com/nilshenrich/NetworkClient/blob/main/include/NetworkingDefines.h) for definition of error codes.\
    If your class derived from both **TcpClient** and **TlsClient**, the class name must be specified when calling **start()**:

    ```cpp
    TcpClient::start("serverHost", 8081);
    TlsClient::start("serverHost", 8082, "ca_cert.pem", "client_cert.pem", "client_key.pem");
    ```

1. stop():

    The **stop**-method stops a running client.\
    As for **start()**, if your class derived from both **TcpClient** and **TlsClient**, the class name must be specified when calling **stop()**:

    ```cpp
    TcpClient::stop();
    TlsClient::stop();
    ```

1. sendMsg():

    The **sendMsg**-method sends a message to the server (over TCP or TLS). If the return value is **true**, the sending was successful, if it is **false**, not.\
    As for **start()**, if your class derived from both **TcpClient** and **TlsClient**, the class name must be specified when calling **sendMsg()**:

    ```cpp
    TcpClient::sendMsg("example message over TCP");
    TlsClient::sendMsg("example message over TLS");
    ```

    Please make sure to only use **TcpClient::sendMsg()** for TCP connections and **TlsClient::sendMsg()** for TLS connection.

1. isRunning():

    The **isRunning**-method returns the running flag of the NetworkClient.\
    **True** means: *The client is running*\
    **False** means: *The client is not running*

## Example

This repository contains a small example to show the usage of this package. It creates two client, one using unsecure TCP, the other using encrypted and two-way-authenticated TLS (two-way authentication means, the client authenticates itself with a CA-signed certificate ad forces the server to also authenticate itself with his own CA-signed certificate).\
This program send two example messages to the server, one over the unencrypted TCP connection and the other over the encrypted TLS connection. It also prints received messages to the screen.

### Get certificates

Before the encrypted TLS client can run properly, the needed certificates and private keys need to be copied or linked from the [NetworkListener](https://github.com/nilshenrich/NetworkListener) repository. Please copy the whole folder **keys** from the NetworkListener example directory to the NetworkClient example directory.

### Run example

The example can be compiled the same way as the libraries (Without installing at the end):

```console
cd example
mkdir build
cd build
cmake ..
make
./example
```

## System requirements

Linux distribution based on debian buster or later.

The installation process in this project is adapted to debian-based linux distributions. But smart guys maybe achieve to make it usable on other systems (In the end it is just C++ code compilable with C++17 standard or higher).

## Known issues

### [Success on server rejection](https://github.com/nilshenrich/NetworkClient/issues/10)

If the TLS client accepts the server certificate, the connection is assumed to be established, no matter if the server accepts the client's certificate or not.

If a client tries to connect to TLS server with a self-signed certificate, the connection is rejected on server side, but the client assumes the connection to be accepted nevertheless.

The good new is, an encrypted connection is not established for real, so messages can't be sent and received.
