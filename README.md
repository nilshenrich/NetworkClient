# NetworkClient

Installable package to set up a client that can connect to a server on TCP level and send and receive data asynchronously. The connection can be established with unencrypted pure TCP or with encrypted and two-way-authenticated TLS.

The compatible server can be found [here](https://github.com/nilshenrich/NetworkListener).

# Table of contents

1. [General explanation](#general-explanation)
1. [Installation](#installation)
1. [Example](#example)
1. [System requirements](#system-requirements)

# General explanation

This project contains installable C++ libraries for a client on TCP level that can connect to a server and send and receive data asynchronously.

This packge contains two libraries: **libnetworkClientTcp** and **libnetworkClientTls**.\
As the names say, **libnetworkClientTcp** creates a simple TCP client with no security. The **libnetworkClientTls** creates a client on TLS level. This means, an established connection is encrypted with the latest compatible TLS version and the server is forced to authenticate itself.

## Specifications

1. Maximum message size:  **std::string::max_size() - 2** (2<sup>32</sup> - 2 (4294967294) for most)

# Installation

As already mentioned in [General explanation](#general-explanation), this project contains two installable libraries **libnetworkClientTcp** and **libnetworkClientTls**. These libraries can be installed this way:

1. Install **build-essential** and **cmake**:
    ```console
    sudo apt install build-essential cmake
    ```
    **build-essential** contains compilers and libraries to compile C/C++ code on debian-based systems.\
    **cmake** is used to easily create a Makefile.

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
    *To get information printed on the screen, the package can be built in debug mode (Not recommended when installing libraries) by setting the define: __cmake&#160;&#x2011;DCMAKE_BUILD_TYPE=Debug&#160;..__ (Same when compiling example)*

1. To install the created libraries and header files to your system, run
    ```console
    sudo make install
    ```

1. (optional) In some systems the library paths need to be updated before **libnetworkClientTcp** and **libnetworkClientTls** can be used. So if you get an error like
    ```
    ./example: error while loading shared libraries: libnetworkClientTcp.so.1: cannot open shared object file: No such file or directory
    ```
    please run
    ```console
    sudo /sbin/ldconfig
    ```

Now the package is reade to use. Please see the example for how to use it.

# Example

This repository contains a small example to show the usage of this package. It creates two client, one using unsecure TCP, the other using ecrypted and two-way-authenticated TLS (two-way authentication means, the client authenticates itself with a CA-signed certificate ad forces the server to also authenticate itself with his own CA-signed certificate).\
This program send two example messages to the server, one over the unencrypted TCP connection and the other over the encrypted TLS connection. It also prints received messages to the screen.

## Get certificates

Before the encrypted TLS client can run properly, the needed certificates and private keys need to be copied or linked from the [NetworkListener](https://github.com/nilshenrich/NetworkListener) repository. Please copy the whole folder **keys** from the NetworkListener example directory to the NetworkClient example directory.

## Run example

The example can be compiled the same way as the libraries (Without installing at the end):
```console
cd example
mkdir build
cd build
cmake ..
make
./example
```

# System requirements

The installation process in this project is adapted to debian-based linux distros. But smart guys maybe achieve to make it usable on other sytems (In the end it is just C++ code compilable with C++17 standard or higher).
