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
		std::string _input_line, _server_line;
		std::array<int, 2> _inputs;
		Display *_display;

	public:
		Client( void );
		~Client( void );

		void setDisplay( Display *display );
		void setInputs( int horizontal, int vertical );
		void connectSocket( std::string ip );
		bool handleMessages( void );
};

#endif
