#include "common.h"

// Admin Console
struct ClientInfo {
    int socket_fd;
    time_t heartbeat; 
};

std::map<std::string, ClientInfo> clients; 
std::mutex clients_mutex; // Concurrency Handling Mutex for thread safety

//Heartbeat Monitor
void heartbeatListen() {
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //  Socket Initialization
    bind(udp_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        // Receive "ALIVE:CampusName"
        recvfrom(udp_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        
        std::string msg(buffer);
        if (msg.rfind("ALIVE:", 0) == 0) {
            std::string name = msg.substr(6);
            
            // Update "Last Seen" timestamp
            clients_mutex.lock();
            if (clients.count(name)) {
                clients[name].heartbeat = time(0);
            }
            clients_mutex.unlock();
        }
    }
}

// Message Routing 
void handle_tcp_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    std::string name;

    //Client Authentication
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_sock, buffer, BUFFER_SIZE, 0);
    std::string auth_msg(buffer); 

    // Simple Parsing
    size_t first_colon = auth_msg.find(':');
    size_t second_colon = auth_msg.find(':', first_colon + 1);
    
    if (first_colon != std::string::npos && second_colon != std::string::npos) {
        name = auth_msg.substr(first_colon + 1, second_colon - (first_colon + 1));
        std::string pass = auth_msg.substr(second_colon + 1);

        if (pass == "NU-123") { // Hardcoded Check
            std::string ok = "OK";
            send(client_sock, ok.c_str(), ok.size(), 0);
            
            clients_mutex.lock();
            clients[name] = {client_sock, time(0)};
            clients_mutex.unlock();
            std::cout << "[LOG] " << name << " Connected.\n"; //Logging
        } else {
            close(client_sock); return;
        }
    } else {
        close(client_sock); return;
    }

    // 2. Messaging Loop
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;

        std::string raw(buffer);  //"TARGET message"
        size_t space = raw.find(' ');
        if (space != std::string::npos) {
            std::string target = raw.substr(0, space);
            std::string msg_body = raw.substr(space + 1);

            // [Checklist: Message Routing]
            clients_mutex.lock();
            if (clients.count(target)) {
                std::string forward = "[FROM " + name + "]: " + msg_body;
                send(clients[target].socket_fd, forward.c_str(), forward.size(), 0);
            }
            clients_mutex.unlock();
        }
    }

    // Disconnect
    clients_mutex.lock();
    clients.erase(name);
    clients_mutex.unlock();
    close(client_sock);
    std::cout << "[LOG] " << name << " Disconnected.\n";
}

int main() {
    std::cout << "--- CENTRAL SERVER STARTED ---\n";
    
    // Start UDP Heartbeat Thread
    std::thread(heartbeatListen).detach();

    // Start TCP Listener
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(TCP_PORT);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    // Accept Clients in Background Thread
    std::thread([&](){
        while(true) {
            int new_sock = accept(server_fd, NULL, NULL);
            // [Checklist: Concurrency Handling]
            std::thread(handle_tcp_client, new_sock).detach();
        }
    }).detach();

    // : Admin Console] 
    std::string cmd;
    while(true) {
        std::getline(std::cin, cmd);
        if (cmd == "/list") {
            clients_mutex.lock();
            time_t now = time(0);
            std::cout << "--- Active Campuses ---\n";
            for(auto const& [name, info] : clients) {
                double seconds_ago = difftime(now, info.heartbeat);
                std::cout << name << " (Last Seen: " << seconds_ago << "s ago)\n";
            }
            clients_mutex.unlock();
        }
        else if (cmd.rfind("/broadcast ", 0) == 0) {
            //System Broadcast
            std::string msg = cmd.substr(11);
            int bcast_sock = socket(AF_INET, SOCK_DGRAM, 0);
            int bcast_opt = 1;
            setsockopt(bcast_sock, SOL_SOCKET, SO_BROADCAST, &bcast_opt, sizeof(bcast_opt));
            
            sockaddr_in b_addr;
            b_addr.sin_family = AF_INET;
            b_addr.sin_port = htons(BCAST_PORT);
            b_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

            sendto(bcast_sock, msg.c_str(), msg.size(), 0, (struct sockaddr*)&b_addr, sizeof(b_addr));
            close(bcast_sock);
            std::cout << "Broadcast Sent.\n";
        }
    }
}
