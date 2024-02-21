#include "Server.hpp"
#include <cstdlib> // atoi

int main( int ac, char ** av )
{
	(void)av;
	if (ac != 1) {
		error("Error format: ./server");
	}

	Server server;
	server.bindSocket();
	server.listenToClients();
	while (true) {
		server.handleMessages();
	}
	return (0);
}
