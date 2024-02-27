#ifndef TEXT_HPP
# define TEXT_HPP

# include "utils.hpp"

# include <vector>

class Text
{
	private:
        GLuint _vao, _vbo, _shaderProgram;
		GLint _uniWidth, _uniHeight;
		GLuint _textures;
		std::vector<int> _texts;

	public:
		Text( void );
		~Text( void );

        void setup_shader( void );
		void setup_communication_shaders( void );
		void load_texture( void );
		void setWindowSize( int width, int height );
        void addText( int posX, int posY, int font_size, bool white, std::string str );
		void toScreen( void );
};

#endif
