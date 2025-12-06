#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctime>

#define SERVER_IP "127.0.0.1"
#define TCP_PORT 8080       // Chat
#define UDP_PORT 8081       // Heartbeats
#define BCAST_PORT 8082     // Admin Broadcasts
#define BUFFER_SIZE 1024

#endif
