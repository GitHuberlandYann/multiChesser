#include "Server.hpp"

#include <arpa/inet.h> // htons
#include <sys/socket.h> // listen, accept, send, recv
#include <unistd.h> // close

Server::Server( void ) : _chess(new Chess())
{
	std::cout << "Server started, waiting for connections..." << std::endl;
	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd == -1) {
		error("Fatal error socket");
	}
}

Server::~Server( void )
{
	close(_socket_fd);
	delete _chess;
	std::cout << "Server closed" << std::endl;
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// create new client with increased id
t_client Server::create_client( void )
{
	static int	id;
	return {++id, ""};
}

// broadcast msg to everyone on write set except sender
void Server::broadcast( int fd, std::string msg, fd_set *wfds )
{
	for (int i = 0; i < FD_SETSIZE; i++) {
		if (i != fd && FD_ISSET(i, wfds)) {
			send(i, &msg[0], msg.size(), 0);
		}
	}
	// std::cout << "Boadcasted " << msg << std::flush;
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Server::bindSocket( void )
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	// addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost only
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	std::cout << "serv addr " << inet_ntoa(addr.sin_addr) << " port " << ntohs(addr.sin_port) << std::endl;

	if (bind(_socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		error("Fatal error bind");
	}
}

void Server::listenToClients( void )
{
	if (listen(_socket_fd, 100) == -1) {
		error("Fatal error listen");
	}
	FD_ZERO(&_fds);
	FD_SET(_socket_fd, &_fds); // put fd in fd set
}

void Server::handleMessages( void )
{
	bool modif = false;
	fd_set rfds = _fds; // set read and write fds to current fds
	fd_set wfds = _fds;
	FD_CLR(_socket_fd, &wfds); // rm fd from write fds (we don't send ourselve a message)

	select(FD_SETSIZE, &rfds, &wfds, NULL, NULL);
		
	if (FD_ISSET(_socket_fd, &rfds)) { // new client, our socket received a ping
		struct sockaddr_in addr;
		socklen_t addr_len = sizeof(addr);
		int cfd = accept(_socket_fd, (struct sockaddr *) &addr, &addr_len);
		_clients[cfd] = create_client();
		FD_SET(cfd, &_fds);

		modif = true;
		std::cout << "server: client " << std::to_string(_clients[cfd].id) << ": got connection from " << inet_ntoa(addr.sin_addr) << " port " << ntohs(addr.sin_port) << std::endl;
		return ;
	}

	// we skip ourself and fds not part of read set
	// std::cout << "read size " << sizeof(rfds) << ", write size " << sizeof(wfds) << ", macro still " << FD_SETSIZE << std::endl;
	// might want to loop through rfds and check if i != 0 for opti
	for (int i = 0; i < FD_SETSIZE; i++) {
		if (i == _socket_fd || !FD_ISSET(i, &rfds)) continue;

		char buff[1024];
		ssize_t n = recv(i, buff, sizeof(buff), 0);

		if (n == -1 || n == 0) { // client leaves
			std::cout << "server: client " << std::to_string(_clients[i].id) << " just left" << std::endl;
			// modif = true;
			_clients[i] = {0, "", {0, 0}};
			close(i);
			FD_CLR(i, &_fds); // rm client from fd set
			continue;
		}

		buff[n] = '\0';
		// parse received message from client: "xOffset yOffset"
		for (ssize_t j = 0; j < n; j++) {
			t_client &c = _clients[i];
			c.str += buff[j];
			if (buff[j] == '\n') {
				// std::cout << "received from client " << c.str << std::flush;
				modif = true;
				int index = 0, x = 0, y = 0;
				bool xSign = false, ySign = false;
				if (c.str[index] == '-') {
					xSign = true;
					++index;
				}
				for (; isdigit(c.str[index]); ++index) x = x * 10 + c.str[index] - '0';
				++index;
				if (c.str[index] == '-') {
					ySign = true;
					++index;
				}
				for (; isdigit(c.str[index]); ++index) y = y * 10 + c.str[index] - '0';
				int src = (xSign) ? -x : x;
				int dst = (ySign) ? -y : y;
				// std::cout << "move piece " << src << ", " << dst << std::endl;
				_chess->movePiece(src, dst);
				c.str = "";
			}
		}
	}

	if (!modif) {
		return ;
	}

	broadcast(0, _chess->getBoard(), &wfds);
}
