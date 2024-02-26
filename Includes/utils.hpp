#ifndef UTILS_HPP
# define UTILS_HPP

# define GLEW_STATIC
# include <GL/glew.h> // must be before glfw
# include "GLFW/glfw3.h"

# include "common.hpp"

enum {
	SPECATTRIB,
	POSATTRIB
};

std::string get_file_content( std::string file_name );

// shaders
GLuint createShaderProgram( std::string vertex, std::string geometry, std::string fragment );
void check_glstate( std::string str, bool displayDebug );

// textures
void loadSubTextureArray( int layer, std::string texture_file );
void loadTextureShader( int index, GLuint texture, std::string texture_file );

#endif
