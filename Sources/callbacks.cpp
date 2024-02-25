#include "Display.hpp"

Display *display;

void set_window_size_callback( Display *dis )
{
	display = dis;
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
