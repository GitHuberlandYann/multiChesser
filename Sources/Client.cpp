#include "Client.hpp"
#include "Display.hpp"

#include <arpa/inet.h> // htons
#include <sys/socket.h> // connect, send, recv
#include <netdb.h> // gethostbyname
#include <unistd.h> // close
#include <strings.h> // bcopy

Client::Client( void ) : _msg(""), _display(NULL)
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

void Client::setMsg( int src, int dst )
{
	if (src < 0 || src >= 64 || dst < 0 || dst >= 64 || src == dst) {
		return ;
	}
	_msg = std::to_string(src) + " " + std::to_string(dst) + " \n";
}

bool Client::handleMessages( void )
{
	fd_set rfds = _fds, wfds = _fds;

	select(FD_SETSIZE, &rfds, &wfds, NULL, NULL);

	if (_msg[0] && FD_ISSET(_socket_fd, &wfds)) {
		send(_socket_fd, _msg.c_str(), _msg.size(), 0);
		// std::cout << "sent to server: " << _msg << std::flush;
		_msg = "";
	}

	if (FD_ISSET(_socket_fd, &rfds)) { // ping from server
		// std::cout << "PING FROM SERVER" << std::endl;
		char buff[1024];
		ssize_t n = recv(_socket_fd, buff, sizeof(buff), 0);

		if (n == -1 || n == 0) {
			error("Fatal error recv");
		}
		if (n != 65) {
			error("Fatal error Client::handleMessage recv size if " + std::to_string(n));
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
