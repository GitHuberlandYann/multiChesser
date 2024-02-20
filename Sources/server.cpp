// https://www.bogotobogo.com/cplusplus/sockets_server_client.php

#include "utils.hpp"
#include <cstdlib> // atoi

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <unistd.h> // close

typedef struct s_client {
	int id = 0;
	std::string str = "";
}				t_client;

t_client client_create( void ) {
	static int	id;
	t_client res;
	res.id = id++;
	return (res);
}

// broadcast msg to everyone on write set except sender
void broadcast( int fd, std::string msg, fd_set *wfds ) {
	for (int i = 0; i < FD_SETSIZE; i++) {
		if (i != fd && FD_ISSET(i, wfds)) {
			send(i, &msg[0], msg.size(), 0);
		}
	}
	std::cout << msg << std::endl;
}

int main( int ac, char **av )
{
	if (ac != 2) {
		error("Error format: ./server <port>");
	}

	std::cout << "Hello World!" << std::endl;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		error("Fatal error socket");
	}

    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(std::atoi(av[1]));
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		error("Fatal error bind");
	}

	if (listen(fd, 100) == -1) {
		error("Fatal error listen");
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds); // put fd in fd set
	std::array<t_client, FD_SETSIZE> clients;
	std::string msg;

	while (true) {
		fd_set rfds = fds; // set read and write fds to current fds
		fd_set wfds = fds;
		FD_CLR(fd, &wfds); // rm fd from write fds (we don't send ourselve a message)

		select(FD_SETSIZE, &rfds, &wfds, NULL, NULL);
		
		if (FD_ISSET(fd, &rfds)) { // new client, our socket received a ping
			int cfd = accept(fd, NULL, NULL);
			clients[cfd] = client_create();
			FD_SET(cfd, &fds);

			msg = "server: client " + std::to_string(clients[cfd].id) + " just arrived\n";
			broadcast(cfd, msg, &wfds);
			continue;
		}

		// we skip ourself and fds not part of read fds
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (i == fd || !FD_ISSET(i, &rfds)) continue;

			char buff[1024];
			ssize_t n = recv(i, buff, sizeof(buff), 0);

			if (n == -1) { // client leaves
				msg = "server: client " + std::to_string(clients[i].id) + " just left\n";
				broadcast(i, msg, &wfds);
				clients[i].str = "";
				close(i);
				FD_CLR(i, &fds); // rm fd from fd set
				continue;
			}

			buff[n] = '\0';
			// broadcast received msg after cutting the \n
			for (ssize_t j = 0; j < n; j++) {
				t_client &c = clients[i];
				c.str += buff[j];
				if (buff[j] == '\n') {
					msg = "client " + std::to_string(c.id) + ": " + c.str;
					std::cout << msg << std::endl;
					broadcast(i, msg, &wfds),
					c.str = "";
				}
			}
		}
	}
	return (0);
}
