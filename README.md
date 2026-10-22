# Operating Systems — Inter-Process Communication & Chat Systems

A hands-on Operating Systems project built to explore how processes communicate, how concurrent work is coordinated, and how operating-system services support real-time applications. The project brings together two chat-system implementations using different communication mechanisms: **TCP sockets with multithreading** and **shared memory with synchronization**.

Rather than treating OS concepts as theory alone, this project demonstrates them through working communication flows, message handling, and user-facing chat interfaces.

## Project Highlights

- **Two IPC approaches:** network communication through TCP sockets, and local inter-process communication through shared memory.
- **Concurrent client handling:** a multithreaded server can serve multiple connected clients without relying on a single sequential message loop.
- **Synchronization and shared-resource protection:** synchronization mechanisms help coordinate access to shared data and reduce the risk of race conditions.
- **Client–server communication:** clients send messages to a server, which distributes messages to connected participants.
- **Practical OS concepts:** processes, threads, IPC, concurrency, synchronization, shared resources, and communication between independent program instances.
- **GUI-based interaction:** chat interfaces make it possible to send messages and inspect the conversation without relying only on a terminal.

## Implementations

### 1. Network Chat System — TCP Sockets & Multithreading

This implementation supports communication between clients over a network.

**How it works**
1. The server starts and listens on a TCP port.
2. Each client connects to the server using the server machine's IP address and port.
3. The server handles connected clients concurrently using threads.
4. Clients send messages to the server.
5. The server distributes received messages to connected clients.

**Concepts explored**
- TCP client–server architecture
- Socket creation, connection, sending, and receiving
- Reliable, ordered byte-stream communication
- Multithreading and concurrent client handling
- Shared server state and mutex-based synchronization
- Message framing so incoming data can be separated into complete messages
- Network configuration and troubleshooting across machines

### 2. Local Chat System — Shared Memory & Synchronization

This implementation focuses on communication between processes running on the same machine.

**How it works**
1. Separate program instances access a shared memory region.
2. Messages are written to and read from the shared region.
3. Synchronization primitives, including semaphores, coordinate access to shared data.
4. A GUI lets users enter messages and view the conversation.

**Concepts explored**
- Inter-Process Communication (IPC)
- Shared-memory regions and shared data
- Synchronization between independent processes
- Race conditions and the importance of coordinated access
- The difference between sharing data locally and sending data across a network

## What I Learned

This project was an opportunity to connect core Operating Systems theory with implementation details. Working through the two approaches helped me build a clearer understanding of:

- **Processes vs. threads:** how separate processes differ from threads executing within a process, and why threads are useful for handling concurrent work.
- **Concurrency:** how multiple clients or processes can make progress during overlapping periods, and why shared state needs careful handling.
- **Synchronization:** why mutexes and semaphores matter when multiple execution paths access the same resource.
- **Race conditions:** how timing-dependent access to shared data can cause inconsistent behavior, and how synchronization helps prevent it.
- **Inter-Process Communication:** the trade-offs between shared memory for local processes and sockets for communication over a network.
- **TCP and sockets:** how applications establish connections and exchange data through the networking stack.
- **Operating-system responsibilities:** how applications rely on OS-provided abstractions and services for processes, threads, memory, synchronization, and network I/O.
- **Kernel and user space:** the distinction between application code running in user space and OS/kernel services that provide controlled access to resources. The project helped me understand these interactions conceptually through practical programming.
- **Debugging concurrent systems:** how connection state, message boundaries, shared resources, and thread coordination affect real application behavior.

One of the most valuable lessons was that a chat application is not just a GUI and a send button. It is a practical way to study how execution, communication, and coordination fit together in an operating system.

## High-Level Architecture

```text
                 NETWORKED CHAT
┌────────────┐       TCP        ┌─────────────────────┐
│  Client A  │ ◄──────────────► │                     │
└────────────┘                  │   Multithreaded     │
                                │       Server        │
┌────────────┐       TCP        │                     │
│  Client B  │ ◄──────────────► │  Client management  │
└────────────┘                  │  Message broadcast  │
                                └─────────────────────┘

                   LOCAL CHAT
┌─────────────────┐             ┌─────────────────────┐
│ Process / Client│ ◄──────────► │    Shared Memory    │
└─────────────────┘             └─────────────────────┘
                                          ▲
                                          │
                                  Synchronization
                                  (e.g. semaphores)
                                          │
                                ┌─────────────────────┐
                                │ Other local process │
                                └─────────────────────┘
```

## Technologies & Concepts

| Area | Used / Studied |
|---|---|
| Programming language | C++ |
| Network communication | TCP sockets / Winsock in the Windows network implementation |
| Concurrency | Multithreading |
| Synchronization | Mutexes and semaphores, according to the implementation |
| Inter-process communication | Shared memory and sockets |
| Interface | GUI-based chat client |
| Core OS topics | Processes, threads, memory sharing, race conditions, synchronization, user space and kernel services |

## Why This Project Matters

The project compares two fundamentally different ways for independent execution contexts to exchange information:

- **Sockets** are appropriate when communication must happen between machines or through a network stack.
- **Shared memory** can provide efficient local data sharing, but requires synchronization to coordinate access safely.

Implementing and comparing both approaches provides practical experience with IPC design, concurrency, and the trade-offs involved in choosing a communication mechanism.

## Running the Project

The exact build and run commands depend on which implementation and source files are being used.

### Network Chat
1. Build the server and client on Windows with a C++ compiler that supports Winsock.
2. Start the server and note the port it listens on.
3. Allow inbound connections to that port in Windows Firewall if needed.
4. Connect each client using the server machine's local network IPv4 address and port.
5. Keep the server running while clients exchange messages.

### Local Shared-Memory Chat
1. Build the local-chat processes with the compiler and libraries required by the implementation.
2. Start the required process instances.
3. Ensure the shared-memory and synchronization resources are initialized as expected by the implementation.
4. Send messages through the GUI and verify that the other process receives them.


## Repository Contents

The repository contains the source code for the chat-system implementation(s), including the communication logic and user interface. Exact filenames may vary with the final submitted version.

## Academic Context

**Course:** Operating Systems  
**Project:** Inter-Process Communication and Chat Systems  
**Language:** C++

This project strengthened my practical understanding of operating-system concepts by applying them to a concrete application, comparing IPC mechanisms, and working through the challenges of concurrency, synchronization, and communication.

---

*Built as an academic project to turn Operating Systems concepts into practical experience.*
