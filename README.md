	# Apply DLT Logging

## 1. DLT Architecture

DLT logging includes three main components:

```text
Application
   |
   | uses libdlt / DLT API
   v
dlt-daemon
   |
   | TCP connection
   v
DLT Viewer / DLT client
```

>(thì dlt-daemon nó như là background service, nó collect log từ mọi applications
sau đó nó gửi đến dlt client(ở đây là DLT Viewer), 
application cần sử dụng libdlt và dlt api để có thể gửi log xuống cho dlt daemon)

- **Application**: the program that generates logs.
- **dlt-daemon**: a background service that collects logs from applications.
- **DLT Viewer**: a DLT client that receives and displays logs from `dlt-daemon`.

The application uses **libdlt** and the **DLT API** to send logs to `dlt-daemon`. Then, `dlt-daemon` provides those logs to a DLT client, such as **DLT Viewer**, through a TCP connection.

If `dlt-daemon` and DLT Viewer run on different devices, both devices must be able to connect to each other through the network.

---

## 2. Steps to Apply DLT Logging

### Step 1: Define a DLT context

Each module can use a separate context to make logs easier to identify and filter.

```cpp
DltContext main_dltCxt;
```
>khai báo context trong 1 file .cpp
có thể mỗi module sử dụng 1 context riêng để phân biệt log từ module nào

### Step 2: Register the application with `dlt-daemon`

This lets `dlt-daemon` know that the application exists.

```cpp
DLT_REGISTER_APP("CTCP", "TCPClient Application");
```

### Step 3: Register the context

```cpp
DLT_REGISTER_CONTEXT(main_dltCxt, "MAIN", "Main application context");
```

### Step 4: Write logs using the DLT API

```cpp
DLT_LOG(main_dltCxt, DLT_LOG_INFO, DLT_CSTRING(logBuffer));
```

This writes a log with level `DLT_LOG_INFO` and associates it with the `main_dltCxt` context.

---

## 3. Common Logging Macro

To avoid repeating the same logging format everywhere, define a common log macro:

```cpp
extern DltContext main_dltCxt;

#define DEBUG_LOG(fmt, ...)                                                   \
    do {                                                                      \
        char logBuffer[1024];                                                 \
        std::snprintf(logBuffer, sizeof(logBuffer),                           \
            "[%d][%s][%s:%d][%s()] " fmt,                                    \
            static_cast<int>(gettid()),                                       \
            getCurrentThreadName(),                                           \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                    \
                                                                              \
        std::fprintf(stderr, "%s\n", logBuffer);                            \
        DLT_LOG(main_dltCxt, DLT_LOG_INFO, DLT_CSTRING(logBuffer));           \
    } while (0)
```
>đã khai báo rồi, chỗ này extern ra đây để làm common

This macro writes logs to both:

- `stderr`, for quick debugging in the terminal.
- `dlt-daemon`, using `DLT_LOG` with level `DLT_LOG_INFO`.

---

## 4. Summary

To apply DLT logging in the project:

1. Install and run `dlt-daemon`.
2. Link the application with `libdlt`.
3. Register the application using `DLT_REGISTER_APP`.
4. Register one or more contexts using `DLT_REGISTER_CONTEXT`.
5. Use `DLT_LOG` or a common logging macro to send logs to `dlt-daemon`.
6. Use DLT Viewer to connect to `dlt-daemon` and view the logs.


# TCPIP-CLIENT

A lightweight multi-threaded TCP client written in C++.

This project creates a TCP client application that opens multiple TCP connections to a configured TCP server, receives packet data from the server, and stores received packets in a shared thread-safe queue for further processing.

## Features

- TCP client implementation using POSIX sockets
- Multiple client connection support
- Dedicated RX worker thread for each connection
- Automatic reconnect-style state flow
- Thread-safe receive queue
- Basic packet parsing with PDU ID and payload length
- DLT-based logging support
- CMake-based build system

## Tech Stack

- C++14
- CMake
- POSIX socket API
- pthread
- DLT logging library

## Project Structure

```text
TCPIP-CLIENT/
├── include/
│   ├── Application.h
│   ├── CommonDef.h
│   ├── CommonFuction.h
│   ├── Logger.h
│   ├── TCPCommunicator.h
│   ├── TCPConnection.h
│   └── ThreadSafeQueue.h
├── src/
│   ├── Application.cpp
│   ├── TCPCommunicator.cpp
│   └── TCPConnection.cpp
├── CMakeLists.txt
├── rebuild.sh
├── sequence.png
└── sequence_detail.png
```

## How It Works

The application starts from `src/Application.cpp`.

1. The application registers a DLT app and logging context.
2. `Application::init()` starts the TCP communicator.
3. `TCPCommunicator` creates TCP connection objects based on `ConnectionInfoTable`.
4. Each `TCPConnection` starts an RX worker thread.
5. The RX worker opens a TCP socket, binds it to a configured local client IP/port, and connects to the configured server IP/port.
6. After the connection is established, the RX worker continuously reads packets from the server.
7. Received packets are pushed into a global receive queue.
8. `Application::receive_from_server()` pulls packets from the queue so the application can process them.

## Client and Server Configuration

The client and server IP/port mappings are configured in `include/CommonDef.h`.

```cpp
#define CLIENT_IP "192.168.24.128"
#define SERVER_IP "192.168.24.128"

const std::unordered_map<ConnectionID, ConnectionInfo> ConnectionInfoTable = {
    {ConnectionID_ONE,   {SERVER_IP, 10001, CLIENT_IP, 2001}},
    {ConnectionID_TWO,   {SERVER_IP, 10002, CLIENT_IP, 2002}},
    {ConnectionID_THREE, {SERVER_IP, 10003, CLIENT_IP, 2003}}
};
```

By default, the client creates three connections:

| Connection ID | Server Address | Server Port | Client Address | Client Port |
| --- | --- | ---: | --- | ---: |
| `ConnectionID_ONE` | `192.168.24.128` | `10001` | `192.168.24.128` | `2001` |
| `ConnectionID_TWO` | `192.168.24.128` | `10002` | `192.168.24.128` | `2002` |
| `ConnectionID_THREE` | `192.168.24.128` | `10003` | `192.168.24.128` | `2003` |

Update `CLIENT_IP`, `SERVER_IP`, and the port values before running the client in your own environment.

## Packet Format

The client expects packets from the server to use the following binary format:

```text
+----------------+----------------+------------------+
| PDU ID         | Payload Length | Payload          |
| 4 bytes        | 4 bytes        | N bytes          |
+----------------+----------------+------------------+
```

The packet header is 8 bytes:

- First 4 bytes: PDU ID
- Next 4 bytes: payload length
- Remaining bytes: payload data

`TCPConnection::read_pdu()` first reads the 8-byte header, then resizes the packet buffer and reads the payload based on the payload length.

## Connection State Flow

Each connection uses a simple state machine:

```text
CLOSED -> INIT -> CONNECTED
   ^                    |
   |                    |
   +--------------------+
```

- `CLOSED`: closes the socket and moves to `INIT`
- `INIT`: tries to open and connect the socket
- `CONNECTED`: receives packets from the server
- If receiving fails, the connection moves back to `CLOSED`

## Requirements

Before building the project, make sure the following tools/libraries are installed:

- C++ compiler with C++14 support
- CMake 3.10 or newer
- pthread
- DLT development library

On Ubuntu/Debian-based systems:

```bash
sudo apt update
sudo apt install build-essential cmake
```

If the DLT library is missing, install the DLT development package for your Linux distribution.

## Build

Build the project using the provided script:

```bash
chmod +x rebuild.sh
./rebuild.sh
```

The script removes the old `build/` directory, creates a new one, runs CMake, and builds the executable.

You can also build manually:

```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

## Run

Start the TCP server first, then run the client:

```bash
./build/tcpclient
```

Make sure the configured IP addresses exist on your machine. If binding or connecting fails, update `CLIENT_IP`, `SERVER_IP`, and the ports in `include/CommonDef.h`.

## Logging

This project uses DLT logging. The application registers:

```cpp
DLT_REGISTER_APP("CTCP", "TCPClient Application");
DLT_REGISTER_CONTEXT(main_dltCxt, "MAIN", "Main application context");
```

Configure your DLT daemon/environment if you want to view runtime logs.

## Main Components

### Application

Initializes the TCP client, starts the communicator, and periodically reads packets from the receive queue.

### TCPCommunicator

Creates and manages all TCP connection objects. It also owns the shared receive queue used by all connections.

### TCPConnection

Handles one TCP connection to the server. It creates the socket, binds the client address, connects to the server, receives packets, and manages connection state.

### ThreadSafeQueue

A generic thread-safe queue used to store received packets from all TCP connections.

## Notes

- Start the server before running this client.
- The application currently contains a placeholder area for packet handling in `Application::receive_from_server()`.
- The RX queue has a maximum size of 50 elements.
- The project currently focuses on receiving packets from the server; sending data from client to server is not implemented yet.
- `CommonFuction.h` appears to keep helper functions such as socket receive utilities and thread naming.

## Related Project

This client is designed to work with the matching TCP server project:

```text
TCPIP-SERVER
```

Make sure the server ports match the client configuration.

## License

No license has been provided yet. Add a license file if you plan to distribute or open-source this project.
