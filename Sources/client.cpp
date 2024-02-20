#include "utils.hpp"

#include <cstdlib> // atoi

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h> // gethostbyname

#include <unistd.h> // close
#include <strings.h> // bcopy

int main( int ac, char **av )
{
	if (ac != 3) {
		error("Error format: ./server <hostname> <port>");
	}

	std::cout << "Hello World! - client" << std::endl;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		error("Fatal error socket");
	}

	struct hostent *server = gethostbyname(av[1]);
	if (!server) {
		error("Fatal error hostname");
	}

    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(std::atoi(av[2]));
	bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		error("Fatal error connect");
	}

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds); // put fd in fd set
	FD_SET(STDIN_FILENO, &fds); // put std::in in fd set
	std::string line, server_line;

	while (true) {
		fd_set rfds = fds;

		select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

		if (FD_ISSET(STDIN_FILENO, &rfds)) { // ping from std::cin
			char buff[1024];
			ssize_t n = read(STDIN_FILENO, buff, sizeof(buff));

			if (n == -1 || n == 0) {
				error("Fatal error input");
			}
			if (n == 1) break ;

			buff[n] = '\0';
			// broadcast input to server
			for (ssize_t j = 0; j < n; j++) {
				line += buff[j];
				if (buff[j] == '\n') {
					send(fd, &line[0], line.size(), 0);
					line = "";
				}
			}
		}
		if (FD_ISSET(fd, &rfds)) { // ping from server
			char buff[1024];
			ssize_t n = recv(fd, buff, sizeof(buff), 0);

			if (n == -1 || n == 0) {
				error("Fatal error recv");
			}

			buff[n] = '\0';
			// broadcast input to server
			for (ssize_t j = 0; j < n; j++) {
				server_line += buff[j];
				if (buff[j] == '\n') {
					std::cout << server_line << std::flush;
					server_line = "";
				}
			}
		}
	}
	close(fd);
	std::cout << "Goodbye" << std::endl;
	return (0);
}
