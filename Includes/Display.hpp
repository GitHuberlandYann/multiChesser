#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# define GLEW_STATIC
# include <GL/glew.h> // must be before glfw
# include "GLFW/glfw3.h"

# include "Client.hpp"
# include "Chess.hpp"
# include <vector>
# include <array>

namespace STATE
{
	enum {
		MENU,
		GAME
	};
}

enum {
	SPECATTRIB,
	POSATTRIB
};

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _vao, _vbo, _shaderProgram;
		GLint _winWidth, _winHeight, _uniWidth, _uniHeight;
		GLuint *_texture;
		Client *_client;
		Chess *_chess;
		int _state, _port;
		bool _mouse_pressed;
		std::array<int, 2> _selected_piece; // {piece::value, starting_square}
		std::string _ip;

		GLuint createShaderProgram( std::string vertex, std::string geometry, std::string fragment );
		void check_glstate( std::string str, bool displayDebug );

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void loadSubTextureArray( int layer, std::string texture_file );
		void load_texture( void );

		void handleInputs( void );
		void draw_rectangles( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void setIP( std::string ip );
		void setPort( int port );
		void start( void );

		void parseServerInput( std::string str );
};

#endif
