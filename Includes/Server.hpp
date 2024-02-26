#ifndef SERVER_HPP
# define SERVER_HPP

# include "common.hpp"
# include "Chess.hpp"
# include <array>
# include <list>
# include <sys/select.h> // select, bind, fd_set

typedef struct s_client {
	int id = 0;
	std::string str = "";
}				t_client;

// client id of white player, client id of black player, whether game needs to broadcast board, chess instance of their game
typedef struct s_room {
	int white = 0;
	int black = 0;
	std::string w_username = "";
	std::string b_username = "";
	bool modif = false;
	Chess *chess;
}				t_room;

class Server
{
	private:
		int _socket_fd, _port;
		fd_set _fds;
		std::array<t_client, FD_SETSIZE> _clients;
		std::list<t_room> _rooms;

		t_client create_client( void );
		void setClientInRoom( int id );
		void setClientUsername( int id, std::string username );
		void rmClientFromRoom( int id );
		void roomsBroadcast( fd_set *wfds );
		void parseClientInput( int client_id, std::string str );

		void bindSocket( void );
		void listenToClients( void );
		void handleMessages( void );

	public:
		Server( void );
		~Server( void );

		void setPort( int port );
		void start( void );
};

#endif
