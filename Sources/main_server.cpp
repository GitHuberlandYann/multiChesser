#include "Server.hpp"
#include <cstdlib> // atoi

int main( int ac, char ** av )
{
	if (ac != 2) {
		error("Error format: ./server <port>");
	}

	Server server;
	server.bindSocket(std::atoi(av[1]));
	server.listenToClients();
	while (true) {
		server.handleMessages();
	}
	return (0);
}
