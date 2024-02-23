#ifndef SERVER_HPP
# define SERVER_HPP

# include "utils.hpp"
# include "Chess.hpp"
# include <array>
# include <sys/select.h> // select, bind, fd_set

typedef struct s_client {
	int id = 0;
	std::string str = "";
	std::array<int, 2> rectangle = {0, 0};
}				t_client;

class Server
{
	private:
		int _socket_fd;
		fd_set _fds;
		std::array<t_client, FD_SETSIZE> _clients;
		Chess *_chess;

		t_client create_client( void );
		void broadcast( int fd, std::string msg, fd_set *wfds );

	public:
		Server( void );
		~Server( void );

		void bindSocket( void );
		void listenToClients( void );
		void handleMessages( void );
};

#endif
