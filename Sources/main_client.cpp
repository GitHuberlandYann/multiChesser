#include "Display.hpp"

int main( int ac, char **av )
{
	if (ac > 2) {
		error("Error format: ./client [ip address]");
	}

	std::cout << "Hello World! - client" << std::endl;

	Display display;
	if (ac == 2) {
		display.setIP(av[1]);
	}
	display.start();
	return (0);
}
