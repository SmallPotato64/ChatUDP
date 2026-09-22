#include "client.h"

void run_client(const std::string& username, const std::string& server_ip, int server_port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }
#endif

    // Création d'un socket UDP (AF_INET = IPv4, SOCK_DGRAM = UDP, 0 = default protocol)
    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket.\n";
        return;
    }

    // Configuration de l'adresse du serveur
    sockaddr_in server_addr{};
	// Utilise IPv4
    server_addr.sin_family = AF_INET;
	// Convertit le port en format réseau (arrange les bytes dans le bon ordre)
    server_addr.sin_port = htons(server_port);
	// Convertit l'adresse IP du serveur en format binaire et la stocke dans server_addr.sin_addr
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

	// Envoi d'un message de connexion au serveur pour s'enregistrer
    std::string reg_msg = "[" + username + "] joined the chat.";
    sendto(client_socket, reg_msg.c_str(), reg_msg.length(), 0,
        (struct sockaddr*)&server_addr, sizeof(server_addr));

    std::cout << "Connected to " << server_ip << ":" << server_port << " as " << username << "\n";
    std::cout << "Type your messages (or /quit to exit):\n";

	// Booléen thread-safe pour indiquer que le programme est en cours d'exécution
    std::atomic<bool> is_running{ true };

    // Création d'un thread séparé pour recevoir des messages
    std::thread receive_thread([&]() {
        char buffer[2048];
        while (is_running) {
            sockaddr_in from_addr{};
            socklen_t from_len = sizeof(from_addr);
            int bytes = recvfrom(client_socket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&from_addr, &from_len);
            if (bytes > 0) {
                buffer[bytes] = '\0'; 
                // Retour charriot pour continuer à écrire normalement
                std::cout << "\r" << buffer << "\n> " << std::flush;
            }
        }
        });

	// Thread principal pour envoyer des messages
    std::string input;
    std::cout << "> ";
    while (is_running && std::getline(std::cin, input)) {
        if (input.empty()) continue;

		// Regarde pour des commandes de déconnexion
        if (input == "/quit" || input == "/exit") {
			// Envoi d'un message de déconnexion au serveur
            sendto(client_socket, input.c_str(), input.length(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
            is_running = false;
            break;
        }

		// Envoi du message formaté au serveur
        std::string formatted_msg = "[" + username + "] : " + input;
        sendto(client_socket, formatted_msg.c_str(), formatted_msg.length(), 0,
            (struct sockaddr*)&server_addr, sizeof(server_addr));

        std::cout << "> ";
    }

	// Fermeture du socket et nettoyage
    CLOSE_SOCKET(client_socket);
	receive_thread.detach(); // Détache le thread de réception pour qu'il puisse se terminer proprement
#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    // Paramètres par défaut
    std::string username = "Player";
    std::string server_ip = "127.0.0.1";
    int server_port = 9999;

	// Paramètres passés en ligne de commande
    if (argc > 1) username = argv[1];
    if (argc > 2) server_ip = argv[2];
    if (argc > 3) server_port = std::stoi(argv[3]);

    run_client(username, server_ip, server_port);
    return 0;
}