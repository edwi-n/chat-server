# Asynchronous Chat Server

A multi-client, real-time TCP chat server using Asio library.

## Features

Asynchronous: Built using Asio's io_context and asynchronous callbacks.

Multithreaded Event Loop: Runs the io_context across a small worker pool so multiple clients can be serviced concurrently.

Multi-Client: Handles numerous concurrent client connections.

Real-time Broadcasting: Messages sent by one client are instantly broadcast to all other connected clients.

Ring Buffers: Uses bounded circular buffers for incoming data and outgoing messages so each session can safely handle async reads and writes.

Thread-Safe Room List: Protects the active-client set with a shared mutex and snapshots recipients before broadcasting.

Session Serialization: Uses a strand per client so its socket state and buffers stay consistent across worker threads.

Username Join: The first non-empty line from a client becomes the username, so a session does not join the room until it has identified itself.



## Building the Server

You must have CMake and a C++17 compatible compiler installed.

### 1. Clone the repository

```bash
git clone https://github.com/edwi-n/chat-server
cd chat-server
```

### 2. Create a build directory
```bash
mkdir build
cd build
```

### 3. Configure the project with CMake
(This will also download Asio)
```bash
cmake ..
```

### 4. Build the executable
```bash
cmake --build .
```


### 5. Running the Server

The executable will be in the build directory.

The server uses a small thread pool internally, so once it starts it can process callbacks on multiple worker threads without additional setup.


```bash
# From the 'build' directory:

# On Linux/macOS
./chat_server 8080

# On Windows
.\chat_server.exe 8080
```


The server will start and listen on the port you specify (e.g., 8080).

How to Connect

You can use any raw TCP client, like telnet or putty, to connect.


```bash
telnet localhost 8080
```

Open several terminal windows and run the command above in each.

The first non-empty line you type in each window will become your username.

Any message you send after that will be broadcast to all other windows!



## To Do

Multiple rooms - be able to join, leave and switch rooms

Server commands - see amount of users, change username ...

Private messages

Encryption

GUI

