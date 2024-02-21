#include "Client.hpp"
#include "Display.hpp"

#include <arpa/inet.h> // htons
#include <sys/socket.h> // connect, send, recv
#include <netdb.h> // gethostbyname
#include <unistd.h> // close
#include <strings.h> // bcopy

Client::Client( void ) : _inputs({0, 0}), _display(NULL)
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

void Client::setDisplay( Display *display )
{
	_display = display;
}

void Client::setInputs( int horizontal, int vertical )
{
	_inputs = {horizontal, -vertical};
}

void Client::connectSocket( std::string ip )
{
	struct hostent *server = gethostbyname(ip.c_str());
	if (!server) {
		error("Fatal error hostname");
	}

    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);
	std::cout << "serv addr " << inet_ntoa(addr.sin_addr) << " port " << ntohs(addr.sin_port) << std::endl;

	if (connect(_socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		error("Fatal error connect");
	}

	FD_ZERO(&_fds);
	FD_SET(_socket_fd, &_fds); // put fd in fd set
}

bool Client::handleMessages( void )
{
	// first send info, then server answers and we are not stuck on select
	std::string msg = std::to_string(_inputs[0]) + " " + std::to_string(_inputs[1]) + " \n";
	send(_socket_fd, msg.c_str(), msg.size(), 0);

	fd_set rfds = _fds;

	select(FD_SETSIZE, &rfds, NULL, NULL, NULL);

	if (FD_ISSET(_socket_fd, &rfds)) { // ping from server
		char buff[1024];
		ssize_t n = recv(_socket_fd, buff, sizeof(buff), 0);

		if (n == -1 || n == 0) {
			error("Fatal error recv");
		}

		// std::cout << "handleMessages n is " << n << std::endl;
		buff[n] = '\0';
		// parse server input
		for (ssize_t j = 0; j < n; j++) {
			_server_line += buff[j];
			if (buff[j] == '\n') {
				if (_display) {
					_display->parseServerInput(_server_line);
				}
				_server_line = "";
			}
		}
	}

	return (true);
}
