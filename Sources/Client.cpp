#include "Client.hpp"

#include <arpa/inet.h> // htons
#include <sys/socket.h> // connect, send, recv
#include <netdb.h> // gethostbyname
#include <unistd.h> // close
#include <strings.h> // bcopy

Client::Client( void )
{
	std::cout << "Hello World! - client" << std::endl;
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1) {
		error("Fatal error socket");
	}
}

Client::~Client( void )
{
	close(_socket_fd);
	std::cout << "Goodbye" << std::endl;
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Client::connectSocket( std::string ip, int port )
{
	struct hostent *server = gethostbyname(ip.c_str());
	if (!server) {
		error("Fatal error hostname");
	}

    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);
	std::cout << "serv addr " << inet_ntoa(addr.sin_addr) << " port " << ntohs(addr.sin_port) << std::endl;

	if (connect(_socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		error("Fatal error connect");
	}

	FD_ZERO(&_fds);
	FD_SET(_socket_fd, &_fds); // put fd in fd set
	FD_SET(STDIN_FILENO, &_fds); // put std::in in fd set
}

bool Client::handleMessages( void )
{
	fd_set rfds = _fds;

	select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

	if (FD_ISSET(STDIN_FILENO, &rfds)) { // ping from std::cin
		char buff[1024];
		ssize_t n = read(STDIN_FILENO, buff, sizeof(buff));

		if (n == -1 || n == 0) {
			error("Fatal error input");
		}
		if (n == 1) {
			return (false);
		}

		buff[n] = '\0';
		// broadcast input to server
		for (ssize_t j = 0; j < n; j++) {
			_input_line += buff[j];
			if (buff[j] == '\n') {
				send(_socket_fd, &_input_line[0], _input_line.size(), 0);
				_input_line = "";
			}
		}
	}
	if (FD_ISSET(_socket_fd, &rfds)) { // ping from server
		char buff[1024];
		ssize_t n = recv(_socket_fd, buff, sizeof(buff), 0);

		if (n == -1 || n == 0) {
			error("Fatal error recv");
		}

		buff[n] = '\0';
		// broadcast server msg to output
		for (ssize_t j = 0; j < n; j++) {
			_server_line += buff[j];
			if (buff[j] == '\n') {
				std::cout << _server_line << std::flush;
				_server_line = "";
			}
		}
	}
	return (true);
}
