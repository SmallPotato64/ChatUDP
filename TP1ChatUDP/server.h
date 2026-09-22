#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <set>
#include <vector>

// Includes pour les sockets selon le système d'exploitation
#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    using socklen_t = int;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using SOCKET = int;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET close
#endif

// Comparateur de clients custom pour stocker les sockaddr_in dans un std::set
// UDP n'a pas de connexions, donc on identifie les clients par leur combinaison IP + Port
struct ClientComparator {
    bool operator()(const sockaddr_in& a, const sockaddr_in& b) const {
        if (a.sin_addr.s_addr != b.sin_addr.s_addr)
            return a.sin_addr.s_addr < b.sin_addr.s_addr;
        return a.sin_port < b.sin_port;
    }
};

void run_server(int port);

#endif