#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# include "utils.hpp"
# include "Client.hpp"
# include "Chess.hpp"
# include "Text.hpp"
# include <vector>
# include <array>

# define GOLDEN_RATIO 1.6180339887
# define WIN_HEIGHT 600
# define WIN_WIDTH WIN_HEIGHT * GOLDEN_RATIO
# define ALPHABETA " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"

namespace STATE
{
	enum {
		MENU,
		INPUT,
		WAITING_ROOM,
		INGAME,
		ENDGAME
	};
}

namespace SELECT
{
	enum {
		NONE,
		USERNAME,
		PLAY
	};
}

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _vao, _vbo, _shaderProgram;
		GLint _winWidth, _winHeight, _squareSize, _uniWidth, _uniHeight;
		GLuint _texture;
		Client *_client;
		Chess *_chess;
		Text *_text;
		int _state, _port, _selection, _highlight, _input_released;
		std::string _username, _opponent_username;
		bool _mouse_left, _mouse_right;
		std::array<int, 3> _selected_piece; // {piece::value, starting_square, draw piece square}
		std::string _ip;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void load_texture( void );

		void handleMenuInputs( void );
		void handleInputs( void );
		void drawRectangle( std::vector<int> &vertices, int type, int startX, int startY, int width, int height );
		void draw( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void setSelection( float posX, float posY );
		void setIP( std::string ip );
		void setPort( int port );
		void setWindowSize( int width, int height );
		void parseServerInput( std::string str );
		void start( void );
};

#endif
