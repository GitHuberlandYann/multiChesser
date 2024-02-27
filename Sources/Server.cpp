#include "Server.hpp"

#include <arpa/inet.h> // htons
#include <sys/socket.h> // listen, accept, send, recv
#include <unistd.h> // close

Server::Server( void ) : _port(PORT)
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

// look for empty place in current rooms, if no place found, create new room
void Server::setClientInRoom( int id )
{
	for (auto &r : _rooms) {
		if (!r.black) { // room found, game can start
			r.black = id;
			send(id, "col: b\n", 7, 0);
			std::cout << id << " joined room with " << r.white << std::endl;
			return ;
		}
	}
	_rooms.push_back({id, 0, "", "", false, new Chess});
	send(id, "col: w\n", 7, 0);
	std::cout << id << " joined new room" << std::endl;
}

void Server::setClientUsername( int id, std::string username )
{
	for (auto &r : _rooms) {
		if (id == r.white) {
			r.w_username = username;
			if (r.b_username[0]) {
				std::string msg = "OPP: " + r.b_username + '\n';
				send(id, &msg[0], msg.size(), 0);
				msg = "OPP: " + r.w_username + '\n';
				send(r.black, &msg[0], msg.size(), 0);
				r.modif = true; // game can start
			}
			return ;
		} else if (id == r.black) {
			r.b_username = username;
			if (r.w_username[0]) {
				std::string msg = "OPP: " + r.b_username + '\n';
				send(r.white, &msg[0], msg.size(), 0);
				msg = "OPP: " + r.w_username + '\n';
				send(id, &msg[0], msg.size(), 0);
				r.modif = true; // game can start
			}
			return ;
		}
	}
}

void Server::rmClientFromRoom( int id )
{
	for (auto it = _rooms.begin(); it != _rooms.end(); ++it) {
		if (it->white == id) { // TODO send msg to other client
			it->white = 0;
			std::cout << id << " left his room" << std::endl;
			if (!it->black) {
				delete it->chess;
				_rooms.erase(it);
				std::cout << "room destroyed" << std::endl;
			}
			return ;
		}
		if (it->black == id) { // TODO send msg to other client
			it->black = 0;
			std::cout << id << " left his room" << std::endl;
			if (!it->white) {
				delete it->chess;
				_rooms.erase(it);
				std::cout << "room destroyed" << std::endl;
			}
			return ;
		}
	}
}
/*
// broadcast msg to everyone on write set except sender
void Server::broadcast( int fd, std::string msg, fd_set *wfds )
{
	for (int i = 0; i < FD_SETSIZE; i++) {
		if (i != fd && FD_ISSET(i, wfds)) {
			send(i, &msg[0], msg.size(), 0);
		}
	}
	// std::cout << "Boadcasted " << msg << std::flush;
}*/

// loop through rooms and broadcast msg from rooms who need it
void Server::roomsBroadcast( fd_set *wfds )
{
	for (auto &r : _rooms) {
		if (r.modif) {
			std::string msg = r.chess->getFEN(true);
			if (FD_ISSET(r.white, wfds)) {
				send(r.white, &msg[0], msg.size(), 0);
			}
			if (FD_ISSET(r.black, wfds)) {
				send(r.black, &msg[0], msg.size(), 0);
			}
			r.modif = false;
		}
	}
}

void Server::parseClientInput( int client_id, std::string str )
{
	// if (!str.compare(0, 8, "button: ")) { // will be used for resign/draw offer/takebacks buttons
	// 	broadcast(str.substr(8));
	// } else {
	if (!str.compare(0, 10, "username: ")) {
		setClientUsername(client_id, str.substr(10, str.size() - 11));
	} else {
		int index = 0, x = 0, y = 0;
		bool xSign = false, ySign = false;
		if (str[index] == '-') {
			xSign = true;
			++index;
		}
		for (; isdigit(str[index]); ++index) x = x * 10 + str[index] - '0';
		++index;
		if (str[index] == '-') {
			ySign = true;
			++index;
		}
		for (; isdigit(str[index]); ++index) y = y * 10 + str[index] - '0';
		int src = (xSign) ? -x : x;
		int dst = (ySign) ? -y : y;
		// std::cout << "move piece " << src << ", " << dst << std::endl;
		for (auto &r : _rooms) {
			if (client_id == r.white || client_id == r.black) {
				if (r.chess->tryMovePiece(src, dst)) {
					r.modif = true;
				} else {
					std::string msg = r.chess->getFEN(false);
					send(client_id, &msg[0], msg.size(), 0);
				}
				break ;
			}
		}
	}
}

void Server::bindSocket( void )
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
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

		setClientInRoom(cfd);
		std::cout << "server: client " << _clients[cfd].id << ": got connection from " << inet_ntoa(addr.sin_addr) << " port " << ntohs(addr.sin_port) << std::endl;
		return ;
	}

	// we skip ourself and fds not part of read set
	for (int i = 0; i < FD_SETSIZE; i++) {
		if (i == _socket_fd || !FD_ISSET(i, &rfds)) continue;

		char buff[1024];
		ssize_t n = recv(i, buff, sizeof(buff), 0);

		if (n == -1 || n == 0) { // client leaves
			_clients[i] = {0, ""};
			close(i);
			FD_CLR(i, &_fds); // rm client from fd set
			rmClientFromRoom(i);
			std::cout << "server: client " << i << " just left" << std::endl;
			continue;
		}

		buff[n] = '\0';
		// parse received message from client: "xOffset yOffset"
		for (ssize_t j = 0; j < n; j++) {
			t_client &c = _clients[i];
			c.str += buff[j];
			if (buff[j] == '\n') {
				// std::cout << "received from client " << c.str << std::flush;
				parseClientInput(i, c.str);
				c.str = "";
			}
		}
	}

	roomsBroadcast(&wfds);
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Server::setPort( int port )
{
	_port = port;
}

void Server::start( void )
{
	bindSocket();
	listenToClients();
	while (true) {
		handleMessages();
	}
}

// ************************************************************************** //
//                                Common                                      //
// ************************************************************************** //

void error( std::string str )
{
	std::cerr << str << std::endl;
	exit(EXIT_FAILURE);
}
