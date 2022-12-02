# NetworkClient

Installable package to set up a client that can connect to a server on TCP level and send and receive data asynchronously. The connection can be established with unencrypted pure TCP or with encrypted and two-way-authenticated TLS.

The compatible server can be found [here](https://github.com/nilshenrich/NetworkListener).

A test run can be found [here](https://github.com/nilshenrich/NetworkTester/actions)

## Table of contents

1. [General explanation](#general-explanation)
    1. [Specifications](#specifications)
1. [Installation](#installation)
1. [Usage](#usage)
    1. [Preparation](#preparation)
    1. [Methods](#methods)
    1. [Passing worker function](#passing-worker-function)
1. [Example](#example)
    1. [Get certificates](#get-certificates)
    1. [Run example](#run-example)
1. [System requirements](#system-requirements)
1. [Known issues](#known-issues)

## General explanation

This project contains installable C++ libraries for a client on TCP level that can connect to a server and send and receive data asynchronously.

This package contains two libraries: **libnetworkClientTcp** and **libnetworkClientTls**.\
As the names say, **libnetworkClientTcp** creates a simple TCP client with no security. The **libnetworkClientTls** creates a client on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and the server is forced to authenticate itself.

### Specifications

\-

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

*In the subfolder [example](include/NetworkingDefines.h) you can find a good and simple example program that shows how to use the package*

Please mind that the TcpClient and TlsClient are defined in namespace ```networking```, so all example code here expects this using directive:

```cpp
using namespace networking;
```

### Preparation

To use this package, just create an instance of **TcpClient** or **TlsClient** by using one of the provided constructors.\
For the data transfer, either the **fragmentation-mode** or the **forwarding-mode** can be chosen.\
In **fragmentation-mode**, a delimiter character must defined to split the incoming data stream to explicit messages. Please note that when using this mode, the delimiter character can't be part of any message.\
In **forwarding-mode**, all incoming data gets forwarded to an output stream of your choice. I recommend to use the normal writing mode when defining this output stream.

For fragmentation mode, a worker function can be defined that is executed automatically every time a new message is received. For more details, please check [Passing worker function](#passing-worker-function).

1. Implement worker methods

    ```cpp
    // Worker for incoming message (Only used in fragmentation-mode)
    void worker_message(string msg)
    {
        // Do stuff with message
        // (msg could be changed if needed)
    }

    // Output stream
    ofstream ofs{"MyFwdFile"};
    ```

1. Create instance

    ```cpp
    // Fragmentation mode (Delimiter is line break in this case)
    TcpClient tcp_fragm{'\n'};
    TlsClient tls_fragm{'\n'};

    // You can also give optional arguments:
    //  - Maximum amount ot time (number in milliseconds) for server to verify an established connection. -> Default is 1000ms
    //  - Maximum length of messages (incoming and sent). -> Default is the maximum length a string can handle on your system
    TcpClient tcp_fragm_opt{'\n', 50, 100};
    TlsClient tls_fragm_opt{'\n', 50, 100};

    // Forwarding mode
    TcpClient tcp_fwd;
    TlsClient tls_fwd;
    ```

### Methods

All methods can be used the same way for **fragmentation-mode** or **forwarding-mode**.

1. start():

    The **start**-method is used to start a TCP or TLS client. When this method returns 0, the client runs in the background. If the return value is other that 0, please see [NetworkingDefines.h](include/NetworkingDefines.h) for definition of error codes.\
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

### Passing worker function

Passing worker functions might be a bit tricky depending on the definition functions definition.\
The following examples only show passing the worker for incoming messages. Other workers can be passed similarly.\
The following cases can be handled as shown:

1. Standalone function:

    The easiest way is using a standalone function that is not a part of any class.

    ```cpp
    void standalone(const string msg)
    {
        // Some code
    }

    // Delimiter set to \n
    TcpClient tcpClient{'\n'};
    tcpClient.setWorkOnMessage(&standalone);
    ```

1. Member function of this:

    A worker function could also be defined as a class method. If the TCP/TLS client shall be created within the same class that holds the worker function (e.g. in initializer list), this can be done as follows:

    ```cpp
    class ExampleClass
    {
    public:
        // Delimiter set to \n
        ExampleClass(): tcpClient{'\n'}
        {
            tcpClient.setWorkOnMessage(::std::bind(&ExampleClass::classMember, this, ::std::placeholders::_1));
        }
        virtual ~ExampleClass() {}

    private:
        // TCP client as class member
        TcpClient tcpClient;

        void classMember(const string msg)
        {
            // Some code
        }
    };
    ```

    The **bind** function is used to get the function reference to a method from an object, in this case ```this```. For each attribute of the passed function, a placeholder with increasing number must be passed.

1. Member function of foreign class:

    Passing a member function from a foreign class to TCP/TLS client can be done similarly to above example.

    ```cpp
    class ExampleClass
    {
    public:
        ExampleClass() {}
        virtual ~ExampleClass() {}

    private:
        void classMember(const string msg)
        {
            // Some code
        }
    };

    // Create object
    ExampleClass exampleClass;

    // TCP client outside from class
    // Delimiter set to \n
    TcpClient tcpClient{'\n'};
    tcpClient.setWorkOnMessage(::std::bind(&ExampleClass::classMember, exampleClass, ::std::placeholders::_1));
    ```

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

\<no known issues\>
