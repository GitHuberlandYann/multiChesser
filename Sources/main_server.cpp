#include "Server.hpp"
#include <cstdlib> // atoi

int main( int ac, char ** av )
{
	if (ac > 2) {
		error("Error format: ./server [port]");
	}

	Server server;
	if (ac == 2) {
		server.setPort(std::atoi(av[1]));
	}
	server.start();
	return (0);
}
