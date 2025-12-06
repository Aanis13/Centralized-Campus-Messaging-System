# Centralized-Campus-Messaging-System
A multithreaded C++ socket application implementing a hybrid TCP/UDP architecture for real-time campus communication, heartbeat monitoring, and administrative broadcasting.

Project Overview
The system simulates a centralized network where multiple Campus clients connect to a central server. It ensures real-time communication while preventing the server from blocking when multiple users are connected simultaneously.

Technical Implementation,Concurrency
This project solves the blocking I/O problem inherent in socket programming by using C++ multithreading.

1. Handling Multiple Clients 
We implemented a Thread-Per-Client model to handle concurrency:
The main thread runs an infinite loop waiting for accept() calls.
When a new client connects, the server spawns a new std::thread and detaches it.
This detached thread handles the TCP communication for that specific client independently, allowing the main thread to go back to listening for new connections immediately.

2. Preventing Race Conditions 
Since multiple threads need to access the shared list of connected clients std::map, there was a risk of data corruption . 
* We used std::mutex to create critical sections.
* Whenever the map is read or modified , the mutex locks the resource to ensure thread safety.

Features
* Hybrid Architecture: TCP and UDP sockets run simultaneously.
* Direct Messaging: Authenticated clients can send private messages to specific campuses.
* Heartbeat Monitor: Clients send background UDP packets every 5 seconds. The server tracks these to determine if a client has gone offline 
* System Broadcast: The Admin can use hardware broadcasting to send alerts to all active listeners on the subnet.
* Authentication: A custom handshake protocol validates credentials before allowing access.

 How to Compile and Run

This project requires a Linux environment  and the -pthread flag for compilation.

 1. Compile
 2. g++ server.cpp -o server -pthread
g++ client.cpp -o client -pthread
./server
./client


g++ server.cpp -o server -pthread
g++ client.cpp -o client -pthread
