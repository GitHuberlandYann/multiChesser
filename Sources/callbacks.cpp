#include "Display.hpp"

Display *display = NULL;

void set_display_callback( Display *dis )
{
	display = dis;
}

void cursor_position_callback( GLFWwindow* window, double xpos, double ypos )
{
	(void)window;
	if (display) {
		display->setSelection(xpos, ypos);
	}
}

void window_size_callback( GLFWwindow *window, int width, int height )
{
	(void)window;
	// std::cout << "window resized to " << width << ", " << height << std::endl;
	if (display) {
		display->setWindowSize(width, height);
		glViewport(0, 0, width, height);
	}
	// glfwGetFramebufferSize(window, &width, &height);
	// std::cout << "frameBufferSize is " << width << ", " << height << std::endl;
	// // glViewport(0, 0, width, height);
}

void error_callback( int error, const char *msg )
{
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}

namespace INPUT
{
	std::string message;
	int cursor = 0;

	void character_callback( GLFWwindow* window, unsigned int codepoint )
	{
		(void)window;
		if (codepoint < 32 || codepoint > 126) {
			std::cout << __func__ << ": codepoint out of range: " << codepoint << std::endl;
			return ;
		}
		// std::cout << "codepoint you just pressed: " << codepoint << " => " << ALPHABETA[codepoint - 32] << std::endl;
		if (cursor == static_cast<int>(message.size())) {
			message += ALPHABETA[codepoint - 32];
		} else {
			message = message.substr(0, cursor) + ALPHABETA[codepoint - 32] + message.substr(cursor);
		}
		++cursor;
	}

	void moveCursor( bool right, bool control )
	{
		cursor += (right) ? 1 : -1;
		if (cursor > static_cast<int>(message.size())) {
			cursor = message.size();
		} else if (cursor < 0) {
			cursor = 0;
		} else if (control && message[cursor] != ' ') {
			moveCursor(right, control);
		}
	}

	void rmLetter( void )
	{
		if (!cursor) return ;
		message = message.substr(0, cursor - 1) + message.substr(cursor);
		--cursor;
	}

	std::string getCurrentMessage( void )
	{
		return (message);
	}

	std::string getCurrentInputStr( char c )
	{
		return (message.substr(0, cursor) + c + message.substr(cursor));
	}
}
