#ifndef CALLBACKS_HPP
# define CALLBACKS_HPP

void set_display_callback( Display *dis );
void cursor_position_callback( GLFWwindow* window, double xpos, double ypos );
void window_size_callback( GLFWwindow *window, int width, int height );
void error_callback( int error, const char *msg );

namespace INPUT
{
	void character_callback( GLFWwindow* window, unsigned int codepoint );
	void moveCursor( bool right, bool control );
	void rmLetter( void );
	bool validUsername( void );
	std::string getCurrentMessage( void );
	std::string getCurrentInputStr( char c );
}

#endif
