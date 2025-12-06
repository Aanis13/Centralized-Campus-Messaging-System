#include "common.h"

std::string my_name;
bool running = true;

//Hybrid Protocol 
void snedHeartbeats() {
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    while (running) {
        std::string msg = "ALIVE:" + my_name;
        sendto(udp_sock, msg.c_str(), msg.size(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        sleep(5); // Send every 5 seconds
    }
}

//Broadcast Listener]
void ListenHeartbeats() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)); // Allow multiple apps to listen

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));

    char buffer[BUFFER_SIZE];
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = recv(sock, buffer, BUFFER_SIZE, 0);
        if (len > 0) {
            std::cout << "\n\n[ADMIN BROADCAST]: " << buffer << "\nCommand> ";
            std::cout.flush();
        }
    }
}

//Message Reception
void receive_tcp(int sock) {
    char buffer[BUFFER_SIZE];
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = recv(sock, buffer, BUFFER_SIZE, 0);
        if (len <= 0) {
            std::cout << "\nServer Disconnected.\n";
            exit(0);
        }
        std::cout << "\n" << buffer << "\nCommand> ";
        std::cout.flush();
    }
}

int main() {
    // Connection Setup
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // Temporary fix: Should be SOCK_STREAM for TCP
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed.\n";
        return -1;
    }

    // 2. Authentication
   std::string password;
    std::cout << "Enter Campus Name: ";
    std::cin >> my_name;
    
    std::cout << "Enter Password: ";
    std::cin >> password; // Now you must type "NU-123" manually
    
    std::string auth = "AUTH:" + my_name + ":" + password;
    send(sock, auth.c_str(), auth.size(), 0);

    char buffer[1024] = {0};
    recv(sock, buffer, 1024, 0); // Wait for "OK"
    if (std::string(buffer) != "OK") {
        std::cout << "Auth Failed.\n"; return 0;
    }

    // 3. Start Background Threads
    std::thread(snedHeartbeats).detach();
    std::thread(ListenHeartbeats).detach();
    std::thread(receive_tcp, sock).detach();

    //User Interface
    std::string target, msg;
    std::cin.ignore(); // Clear buffer
    while (running) {
        std::cout << "Command (Format: Target Msg): ";
        std::string input;
        std::getline(std::cin, input);
        
        if(input == "exit") break;
        send(sock, input.c_str(), input.size(), 0);
    }

    close(sock);
    return 0;
}
