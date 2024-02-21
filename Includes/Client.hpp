#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "utils.hpp"
# include <sys/select.h> // select, bind, fd_set

class Client
{
	private:
		int _socket_fd;
		fd_set _fds;
		std::string _input_line, _server_line;

	public:
		Client( void );
		~Client( void );

		void connectSocket( std::string ip, int port );
		bool handleMessages( void );
};

#endif
