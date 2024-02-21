#ifndef UTILS_HPP
# define UTILS_HPP

# define WIN_WIDTH 300
# define WIN_HEIGHT 300
# define RECT_SIZE 25

# define PORT 8080

# include <iostream>
# include <string>

std::string get_file_content( std::string file_name );
void error( std::string str );

#endif
