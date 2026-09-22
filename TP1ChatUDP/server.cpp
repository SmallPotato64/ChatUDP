#include "server.h"

void run_server(int port) {
#ifdef _WIN32
	// Initialisation de la librarie winsock (permet d'utiliser des sockets sur windows)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }
#endif

    // Création d'un socket UDP (AF_INET = IPv4, SOCK_DGRAM = UDP, 0 = default protocol)
    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket.\n";
        return;
    }

	// Configuration de l'adresse du serveur
    sockaddr_in server_addr{};
	// Utilise IPv4
    server_addr.sin_family = AF_INET;
    // Utilise 0.0.0.0 (bind à toutes les interfaces)
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // Convertit le port en format réseau(arrange les bytes dans le bon ordre)
    server_addr.sin_port = htons(port);

	// Bind le socket à l'adresse et au port spécifiés (lien avec le kernel)
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) { // cast du sockaddr_in en sockaddr générique
        std::cerr << "Bind failed on port " << port << ".\n";
        CLOSE_SOCKET(server_socket);
        return;
    }

    std::cout << "UDP Server listening on port " << port << "...\n";

	// Création d'un set pour stocker les clients connectés
    std::set<sockaddr_in, ClientComparator> clients;
	// Buffer pour recevoir les messages
    char buffer[2048];

	// Boucle principale du serveur pour recevoir et relayer les messages
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
		// Attend un message puis le stocke dans buffer et récupère l'adresse du client
        int bytes_received = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0) {
			buffer[bytes_received] = '\0'; // Ajouter un caractère nul à la fin du message pour terminer la chaîne
            std::string msg(buffer);

			// Gérer les commandes de déconnexion des clients
            if (msg == "/quit" || msg == "/exit") {
                clients.erase(client_addr);
                std::cout << "A client disconnected.\n";
                continue;
            }

			// Ajouter les nouveaux clients à la liste si ce n'est pas déjà fait
            if (clients.find(client_addr) == clients.end()) {
                clients.insert(client_addr);
                std::cout << "New client registered!\n";
            }

			// Relayer le message à tous les clients sauf celui qui l'a envoyé
            for (const auto& client : clients) {
				// Verifier que le client actuel n'est pas celui qui a envoyé le message
                if (client.sin_addr.s_addr != client_addr.sin_addr.s_addr || 
                    client.sin_port != client_addr.sin_port) {
                    
                    // Envoi du message au client
                    sendto(server_socket, msg.c_str(), msg.length(), 0, (struct sockaddr*)&client, sizeof(client));
                }
            }
            
			std::cout << "Relayed: " << msg << "\n"; // Affiche le message relayé dans la console du serveur
        }
    }
    
	// Fermeture du socket et nettoyage de Winsock si nécessaire
    CLOSE_SOCKET(server_socket); 
#ifdef _WIN32
    WSACleanup();
#endif
}

// Lancement du serveur avec un port par défaut ou passé en paramètre
int main(int argc, char* argv[]) {
    int port = 9999;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    run_server(port);
    return 0;
}