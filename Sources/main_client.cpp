#include "Display.hpp"

int main( int ac, char **av )
{
	if (ac > 3) {
		error("Error format: ./client [ip address] [port]");
	}

	std::cout << "Hello World! - display" << std::endl;

	Display display;
	if (ac == 2) {
		display.setIP(av[1]);
	}
	if (ac == 3) {
		display.setPort(std::atoi(av[2]));
	}
	display.start();
	return (0);
}
