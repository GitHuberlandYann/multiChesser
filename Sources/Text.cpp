#include "Text.hpp"

Text::Text( void ) : _texture(0)
{
}

Text::~Text( void )
{
	std::cout << "Destructor of Text called" << std::endl;
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);

	if (_texture) {
		glDeleteTextures(1, &_texture);
	}
	glDeleteProgram(_shaderProgram);
}


// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Text::setup_shader( void )
{
	_shaderProgram = createShaderProgram("text_vertex", "", "text_fragment");

	glBindFragDataLocation(_shaderProgram, 0, "outColor");

	glBindAttribLocation(_shaderProgram, SPECATTRIB, "position");
	glBindAttribLocation(_shaderProgram, POSATTRIB, "position");

	glLinkProgram(_shaderProgram);
	glUseProgram(_shaderProgram);

	check_glstate("text_Shader program successfully created", true);
}

void Text::setup_communication_shaders( void )
{
	_uniWidth = glGetUniformLocation(_shaderProgram, "win_width");
	_uniHeight = glGetUniformLocation(_shaderProgram, "win_height");

	check_glstate("\nCommunication with text shader program successfully established", false);

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	check_glstate("text VAO and VBO", false);
}

void Text::load_texture( void )
{
	glUseProgram(_shaderProgram);

	glGenTextures(1, &_texture);

	loadTextureShader(1, _texture, "Resources/asciiAtlas.png");
	glUniform1i(glGetUniformLocation(_shaderProgram, "asciiAtlas"), 1); // sampler2D #index in fragment shader
}

void Text::setWindowSize( int width, int height )
{
	glUseProgram(_shaderProgram);

	glUniform1i(_uniWidth, width);
	glUniform1i(_uniHeight, height);
}

void Text::addText( int posX, int posY, int font_size, bool white, std::string str )
{
	int startX = posX;
	for (size_t i = 0, charLine = 0; i < str.size(); i++) {
		if (str[i] == '\n') {
			posY += 1.2f * font_size;
			posX = startX;
			charLine = 0;
		} else if (str[i] == ' ') {
			posX += font_size;
			++charLine;
		} else if (str[i] == '\t') {
			charLine += 4 - (charLine & 3);
			posX = startX + charLine * font_size;
		} else {
			char c = str[i];
			int spec = c + (white << 10);
			_texts.push_back(spec + (0 << 8) + (0 << 9));
			_texts.push_back(posX);
			_texts.push_back(posY);
			_texts.push_back(spec + (1 << 8) + (0 << 9));
			_texts.push_back(posX + font_size);
			_texts.push_back(posY);
			_texts.push_back(spec + (0 << 8) + (1 << 9));
			_texts.push_back(posX);
			_texts.push_back(posY + font_size);

			_texts.push_back(spec + (1 << 8) + (0 << 9));
			_texts.push_back(posX + font_size);
			_texts.push_back(posY);
			_texts.push_back(spec + (1 << 8) + (1 << 9));
			_texts.push_back(posX + font_size);
			_texts.push_back(posY + font_size);
			_texts.push_back(spec + (0 << 8) + (1 << 9));
			_texts.push_back(posX);
			_texts.push_back(posY + font_size);
			if (c == 'i' || c == '.' || c == ':' || c == '!' || c == '\'' || c == ',' || c == ';' || c == '|' || c == '`') {
				posX += font_size * 0.5;
			} else if (c == 'I' || c == '[' || c == ']' || c == '"' || c == '*') {
				posX += font_size * 0.6;	
			} else if (c == 'l' || c == 't' || c == '(' || c == ')' || c == '<' || c == '>' || c == '{' || c == '}') {
				posX += font_size * 0.7;
			} else {
				posX += font_size;
			}
			++charLine;
		}
	}
}

void Text::toScreen( void )
{
	size_t tSize = _texts.size();
	if (tSize == 0) {
		return ;
	}

    glBindVertexArray(_vao);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, tSize * sizeof(GLint), &_texts[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(SPECATTRIB);
	glVertexAttribIPointer(SPECATTRIB, 1, GL_INT, 3 * sizeof(GLint), 0);
	glEnableVertexAttribArray(POSATTRIB);
	glVertexAttribIPointer(POSATTRIB, 2, GL_INT, 3 * sizeof(GLint), (void *)(1 * sizeof(GLint)));

	check_glstate("Text::toScreen", false);

	glUseProgram(_shaderProgram);
	glDrawArrays(GL_TRIANGLES, 0, tSize / 3);

	_texts.clear();
}
