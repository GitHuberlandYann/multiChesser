#include "Client.hpp"

int main( int ac, char **av )
{
	if (ac != 3) {
		error("Error format: ./server <hostname> <port>");
	}

	Client client;
	client.connectSocket(av[1], std::atoi(av[2]));
	while (true) {
		if (!client.handleMessages()) {
			break ;
		}
	}
	return (0);
}
