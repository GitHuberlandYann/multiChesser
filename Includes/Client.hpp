#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "utils.hpp"
# include <array>
# include <sys/select.h> // select, bind, fd_set

class Display;

class Client
{
	private:
		int _socket_fd;
		fd_set _fds;
		std::string _server_line, _msg;
		Display *_display;

	public:
		Client( void );
		~Client( void );

		void setDisplay( Display *display );
		void connectSocket( std::string ip, int port, std::string username );
		void setMsg( int src, int dst );
		void handleMessages( void );
};

#endif
