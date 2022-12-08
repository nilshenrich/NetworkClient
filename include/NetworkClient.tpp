template <class SocketType, class SocketDeleter>
int NetworkClient<SocketType, SocketDeleter>::start(
    const std::string &serverIp,
    const int serverPort,
    const char *const pathToCaCert,
    const char *const pathToCert,
    const char *const pathToPrivKey)
{
    using namespace std;

    // Check if client is already running
    // If so, return with error
    if (running)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Client already running" << endl;
#endif // DEVELOP

        return -1;
    }

    // Check if the port number is valid
    if (1 > serverPort || 65535 < serverPort)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": The port " << serverPort << " couldn't be used" << endl;
#endif // DEVELOP

        return NETWORKCLIENT_ERROR_START_WRONG_PORT;
    }

    // Initialize the client
    // If initialization fails, return with error
    int initCode{init(pathToCaCert, pathToCert, pathToPrivKey)};
    if (initCode)
        return initCode;

    // Create the client tcp socket and connect to the server
    // If socket creation fails, stop client and return with error
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == tcpSocket)
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error while creating client TCP socket" << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_CREATE_SOCKET;
    }

    // Set the socket options
    // If setting fails, stop client and return with error
    int opt{0};
    if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error while setting TCP socket options" << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_SET_SOCKET_OPT;
    }

    // Initialize the socket address
    struct hostent *serverHost;
    serverHost = gethostbyname(serverIp.c_str());
    memset(&socketAddress, 0, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = *(uint32_t *)serverHost->h_addr_list[0];
    socketAddress.sin_port = htons(serverPort);

    // Connect to the server via unencrypted TCP
    // If connection fails, stop client and return with error
    if (connect(tcpSocket, (struct sockaddr *)&socketAddress, sizeof(socketAddress)))
    {
#ifdef DEVELOP
        cerr << typeid(this).name() << "::" << __func__ << ": Error while connecting to server" << endl;
#endif // DEVELOP

        stop();
        return NETWORKCLIENT_ERROR_START_CONNECT;
    }

    // Initialize the TCP connection to the server
    // If initialization fails, stop client and return with error
    clientSocket.reset(connectionInit());
    if (!clientSocket.get())
    {
        stop();
        return NETWORKCLIENT_ERROR_START_CONNECT_INIT;
    }

    // Receive incoming data from the server infinitely in the background while the client is running
    // If background task already exists, return with error
    if (recHandler.joinable())
        throw NetworkClient_error("Error while starting background receive task. Background task already exists");
    recHandler = thread{&NetworkClient::receive, this};

    // Client is now running
    running = true;

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Client started" << endl;
#endif // DEVELOP

    return NETWORKCLIENT_START_OK;
}

template <class SocketType, class SocketDeleter>
void NetworkClient<SocketType, SocketDeleter>::stop()
{
    using namespace std;

    // Stop the client
    running = false;

    // Block the TCP socket to abort receiving process
    connectionDeinit();
    int shut{shutdown(tcpSocket, SHUT_RDWR)};

    // Wait for the background receive thread to finish
    if (recHandler.joinable())
        recHandler.join();

    // If shutdown failed, abort stop here
    if (shut)
        return;

    // Close the TCP socket
    close(tcpSocket);

#ifdef DEVELOP
    cout << typeid(this).name() << "::" << __func__ << ": Client stopped" << endl;
#endif // DEVELOP

    return;
}

template <class SocketType, class SocketDeleter>
bool NetworkClient<SocketType, SocketDeleter>::sendMsg(const std::string &msg)
{
    using namespace std;

    if (MESSAGE_FRAGMENTATION_ENABLED)
    {
        // Check if message doesn't contain delimiter
        if (msg.find(DELIMITER_FOR_FRAGMENTATION) != string::npos)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Message contains delimiter" << endl;
#endif // DEVELOP

            return false;
        }

        // Check if message is too long
        if (msg.length() > MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION)
        {
#ifdef DEVELOP
            cerr << typeid(this).name() << "::" << __func__ << ": Message is too long" << endl;
#endif // DEVELOP

            return false;
        }
    }

    // Send the message to the server with leading and trailing characters to indicate the message length
    if (running)
        return writeMsg(MESSAGE_FRAGMENTATION_ENABLED ? msg + string{DELIMITER_FOR_FRAGMENTATION} : msg);

#ifdef DEVELOP
    cerr << typeid(this).name() << "::" << __func__ << ": Client not running" << endl;
#endif // DEVELOP

    return false;
}

template <class SocketType, class SocketDeleter>
void NetworkClient<SocketType, SocketDeleter>::setWorkOnMessage(std::function<void(const std::string)> worker)
{
    workOnMessage = worker;
}

template <class SocketType, class SocketDeleter>
void NetworkClient<SocketType, SocketDeleter>::receive()
{
    using namespace std;

    // Do receive loop until client is stopped
    string buffer;
    while (1)
    {
        // Wait for incoming data from the server
        // This method blocks until data is received
        // An empty string is returned if the connection is crashed
        string msg{readMsg()};
        if (msg.empty())
        {
#ifdef DEVELOP
            cout << typeid(this).name() << "::" << __func__ << ": Connection to server lost" << endl;
#endif // DEVELOP

            // Stop the client
            running = false;

            // Wait for all work handlers to finish
            for (auto &it : workHandlers)
                it.join();

            // Block the TCP socket to abort receiving process
            // If shutdown failed, abort stop here
            connectionDeinit();
            if (shutdown(tcpSocket, SHUT_RDWR))
                return;

            // Close the TCP socket
            close(tcpSocket);

            return;
        }

        // If stream shall be fragmented ...
        if (MESSAGE_FRAGMENTATION_ENABLED)
        {
            // Get raw message separated by delimiter
            // If delimiter is found, the message is split into two parts
            size_t delimiter_pos{msg.find(DELIMITER_FOR_FRAGMENTATION)};
            while (string::npos != delimiter_pos)
            {
                string msg_part{msg.substr(0, delimiter_pos)};
                msg = msg.substr(delimiter_pos + 1);
                delimiter_pos = msg.find(DELIMITER_FOR_FRAGMENTATION);

                // Check if the message is too long
                if (buffer.size() + msg_part.size() > MAXIMUM_MESSAGE_LENGTH_FOR_FRAGMENTATION)
                {
#ifdef DEVELOP
                    cerr << typeid(this).name() << "::" << __func__ << ": Message from server is too long" << endl;
#endif // DEVELOP

                    buffer.clear();
                    continue;
                }

                buffer += msg_part;

#ifdef DEVELOP
                cout << typeid(this).name() << "::" << __func__ << ": Received message from server: " << buffer << endl;
#endif // DEVELOP

                unique_ptr<RunningFlag> workRunning{new RunningFlag{true}};
                thread work_t{[this](RunningFlag *workRunning_p, string buffer)
                              {
                                  // Mark thread as running
                                  NetworkClient_running_manager running_mgr{*workRunning_p};

                                  // Run code to handle the incoming message
                                  if (workOnMessage)
                                      workOnMessage(move(buffer));

                                  return;
                              },
                              workRunning.get(), move(buffer)};

                // Remove all finished work handlers from the vector
                size_t workHandlers_s{workHandlersRunning.size()};
                for (size_t i{0}; i < workHandlers_s; i += 1)
                {
                    if (!*workHandlersRunning[i].get())
                    {
                        workHandlers[i].join();
                        workHandlers.erase(workHandlers.begin() + i);
                        workHandlersRunning.erase(workHandlersRunning.begin() + i);
                        i -= 1;
                        workHandlers_s -= 1;
                    }
                }

                workHandlers.push_back(move(work_t));
                workHandlersRunning.push_back(move(workRunning));
            }
            buffer += msg;
        }

        // If stream shall be forwarded to continuous out stream ...
        else
        {
            // Just forward incoming message to output stream
            CONTINUOUS_OUTPUT_STREAM << msg;
        }
    }
}

template <class SocketType, class SocketDeleter>
bool NetworkClient<SocketType, SocketDeleter>::isRunning() const
{
    return running;
}
