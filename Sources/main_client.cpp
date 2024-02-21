#include "Display.hpp"

int main( int ac, char **av )
{
	(void)av;
	if (ac != 1) {
		error("Error format: ./client");
	}

	std::cout << "Hello World! - client" << std::endl;

	Display display;
	display.start();
	return (0);
}
