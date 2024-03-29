#ifndef TEXT_HPP
# define TEXT_HPP

# include "utils.hpp"

# include <vector>

namespace TEXT
{
	const int WHITE = 255;
	const int BLACK = 0;
}

class Text
{
	private:
        GLuint _vao, _vbo, _shaderProgram;
		GLint _uniWidth, _uniHeight;
		GLuint _texture;
		std::vector<int> _texts;

	public:
		Text( void );
		~Text( void );

        void setup_shader( void );
		void setup_communication_shaders( void );
		void load_texture( void );
		void setWindowSize( int width, int height );
        void addText( int posX, int posY, int font_size, int grey_level, std::string str );
		void toScreen( void );
};

#endif
