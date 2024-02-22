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
		std::vector<std::array<int, 2>> _rectangles;
		int _state;

		GLuint createShaderProgram( std::string vertex, std::string geometry, std::string fragment );
		void check_glstate( std::string str, bool displayDebug );

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void loadSubTextureArray( int layer, std::string texture_file );
		void load_texture( void );
		void draw_rectangles( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void start( void );
		void parseServerInput( std::string str );
};

#endif
