#include "Display.hpp"
#include "utils.hpp"

#include "SOIL/SOIL.h"
typedef struct {
	unsigned char *content;
	int width;
	int height;
}				t_tex;

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT),
		_texture(NULL), _client(NULL), _state(STATE::MENU), _ip("localhost")
{
	_chess = new Chess();
}

Display::~Display( void )
{
	std::cout << "Destructor of display called" << std::endl;

	if (_texture) {
		glDeleteTextures(1, _texture);
		delete [] _texture;
	}

	glDeleteProgram(_shaderProgram);

	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);

	glfwMakeContextCurrent(NULL);
    glfwTerminate();

	delete _chess;
	delete _client;
	check_glstate("Display successfully destructed", true);
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

static void compile_shader( GLuint ptrShader, std::string name )
{
	glCompileShader(ptrShader);

    GLint status;
    glGetShaderiv(ptrShader, GL_COMPILE_STATUS, &status);
	if (status) {
		std::cout << name << " shader compiled successfully" << std::endl;
	} else {
		char buffer[512];
		glGetShaderInfoLog(ptrShader, 512, NULL, buffer);

		std::cerr << name << " shader did not compile, error log:" << std::endl << buffer << std::endl;
		exit(1);
	}
}

GLuint Display::createShaderProgram( std::string vertex, std::string geometry, std::string fragment )
{
	std::string vertex_shader_data = get_file_content("Sources/Shaders/" + vertex + ".glsl");
	char *vertexSource = &vertex_shader_data[0];

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	compile_shader(vertexShader, vertex);

	GLuint geometryShader;
	if (geometry[0]) {
		std::string geometry_shader_data = get_file_content("Sources/Shaders/" + geometry + ".glsl");
		char *geometrySource = &geometry_shader_data[0];

		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		compile_shader(geometryShader, geometry);
	}

	std::string fragment_shader_data = get_file_content("Sources/Shaders/" + fragment + ".glsl");
	char *fragmentSource = &fragment_shader_data[0];

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	compile_shader(fragmentShader, fragment);

	// Combining shaders into a program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	if (geometry[0]) {
		glAttachShader(shaderProgram, geometryShader);
	}
	glAttachShader(shaderProgram, fragmentShader);

	glDeleteShader(fragmentShader);
	if (geometry[0]) {
		glDeleteShader(geometryShader);
	}
    glDeleteShader(vertexShader);

	return (shaderProgram);
}

void Display::check_glstate( std::string str, bool displayDebug )
{
	GLenum error_check = glGetError();	
	if (error_check) {
		std::cerr << "glGetError set to " << error_check << " when trying to " << str << ", quitting now" << std::endl;
		exit(1);
	}
	if (displayDebug) {
		std::cout << str << std::endl;
	}
}

void error_callback( int error, const char *msg ) {
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}

void Display::setup_window( void )
{
	glfwSetErrorCallback( error_callback );
	if (!glfwInit()) {
    	std::cerr << "glfwInit failure" << std::endl;
        exit(1);
    }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	// glfwWindowHint(GLFW_CENTER_CURSOR, GL_TRUE);

	std::cout << "win size is set to " << _winWidth << ", " << _winHeight << std::endl;
	// (IS_LINUX)
	// 	? _window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "multiChess", nullptr, nullptr)
	// 	: _window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "multiChess", glfwGetPrimaryMonitor(), nullptr);
	_window = glfwCreateWindow(_winWidth, _winHeight, "multiChess", nullptr, nullptr);
	if (_window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

	// activate opengl context
	glfwMakeContextCurrent(_window);

	// glew is there to use the correct version for all functions
	glewExperimental = GL_TRUE;
	glewInit();

	check_glstate("Window successfully created", true);
}

void Display::create_shaders( void )
{
	_shaderProgram = createShaderProgram("vertex", "", "fragment");

	glBindFragDataLocation(_shaderProgram, 0, "outColor");

	glBindAttribLocation(_shaderProgram, SPECATTRIB, "specifications");
	glBindAttribLocation(_shaderProgram, POSATTRIB, "position");

	glLinkProgram(_shaderProgram);
	glUseProgram(_shaderProgram);

	check_glstate("Shader program successfully created", true);
}

void Display::setup_communication_shaders( void )
{
	_uniWidth = glGetUniformLocation(_shaderProgram, "win_width");
	glUniform1i(_uniWidth, _winWidth);
	_uniHeight = glGetUniformLocation(_shaderProgram, "win_height");
	glUniform1i(_uniHeight, _winHeight);

	check_glstate("\nCommunication with shader program successfully established", true);

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	check_glstate("VAO and VBO", false);
}

void Display::loadSubTextureArray( int layer, std::string texture_file )
{
	// load image
	t_tex img;
	img.content = SOIL_load_image(texture_file.c_str(), &img.width, &img.height, 0, SOIL_LOAD_RGBA);
	if (!img.content) {
		std::cerr << "failed to load image " << texture_file << " because:" << std::endl << SOIL_last_result() << std::endl;
		exit(1);
	}

	if (img.width != 300 || img.height != 300) {
		std::cerr << texture_file << ": image size not 300x300 but " << img.width << "x" << img.height << std::endl;
		exit(1);
	}
	// Upload pixel data.
	// The first 0 refers to the mipmap level (level 0, since there's only 1)
	// The following 2 zeroes refers to the x and y offsets in case you only want to specify a subrectangle.
	// 300x300 size of rect, 1 = depth of layer
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, img.width, img.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, img.content);
			
	// set settings for texture wraping and size modif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SOIL_free_image_data(img.content);

	check_glstate("Succesfully loaded " + texture_file + " to shader", true);
}

void Display::load_texture( void )
{
	_texture = new GLuint[1];
	glGenTextures(1, _texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture[0]);

	// alocate pixel data
	// mipmap level set to 1
	// works because all images are 300x300
	// layerCount is 12 (6 whites and 6 black pieces)
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 300, 300, 14);
	loadSubTextureArray(0, "Resources/back_black.png");
	loadSubTextureArray(1, "Resources/back_white.png");
	loadSubTextureArray(2, "Resources/bk.png");
	loadSubTextureArray(3, "Resources/bq.png");
	loadSubTextureArray(4, "Resources/br.png");
	loadSubTextureArray(5, "Resources/bb.png");
	loadSubTextureArray(6, "Resources/bn.png");
	loadSubTextureArray(7, "Resources/bp.png");
	loadSubTextureArray(8, "Resources/wk.png");
	loadSubTextureArray(9, "Resources/wq.png");
	loadSubTextureArray(10, "Resources/wr.png");
	loadSubTextureArray(11, "Resources/wb.png");
	loadSubTextureArray(12, "Resources/wn.png");
	loadSubTextureArray(13, "Resources/wp.png");
	glUniform1i(glGetUniformLocation(_shaderProgram, "pieces"), 0);
	check_glstate("texture_2D_array done", true);
}

void Display::draw_rectangles( void )
{
	if (_rectangles.size()) {
		std::vector<int> vertices;
		_chess->drawBoard(vertices, 30, 30, 240, 240);
		int index = 0;
		for (auto pos : _rectangles) {
			vertices.push_back(0 + (0 << 1) + (index << 2));
			vertices.push_back(pos[0]);
			vertices.push_back(pos[1]);
			vertices.push_back(1 + (0 << 1) + (index << 2));
			vertices.push_back(pos[0] + RECT_SIZE);
			vertices.push_back(pos[1]);
			vertices.push_back(0 + (1 << 1) + (index << 2));
			vertices.push_back(pos[0]);
			vertices.push_back(pos[1] + RECT_SIZE);

			vertices.push_back(1 + (0 << 1) + (index << 2));
			vertices.push_back(pos[0] + RECT_SIZE);
			vertices.push_back(pos[1]);
			vertices.push_back(1 + (1 << 1) + (index << 2));
			vertices.push_back(pos[0] + RECT_SIZE);
			vertices.push_back(pos[1] + RECT_SIZE);
			vertices.push_back(0 + (1 << 1) + (index << 2));
			vertices.push_back(pos[0]);
			vertices.push_back(pos[1] + RECT_SIZE);
			++index;
			// std::cout << "rect at " << pos[0] << ", " << pos[1] << std::endl;
		}
		glUseProgram(_shaderProgram);
		glBindVertexArray(_vao);

		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLint), &(vertices[0]), GL_STATIC_DRAW);

		glEnableVertexAttribArray(SPECATTRIB);
		glVertexAttribIPointer(SPECATTRIB, 1, GL_INT, 3 * sizeof(GLint), 0);
		glEnableVertexAttribArray(POSATTRIB);
		glVertexAttribIPointer(POSATTRIB, 2, GL_INT, 3 * sizeof(GLint), (void *)(1 * sizeof(GLint)));

		check_glstate("Display::draw_rectangles", false);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
		// std::cout << "displaying resctangless " << vertices.size() << std::endl;
	}
}

void Display::main_loop( void )
{
	glfwSwapInterval(1);
	glClearColor(0, 0, 0, 1.0f);

	check_glstate("setup done, entering main loop\n", true);

	while (!glfwWindowShouldClose(_window)) {
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(_window, GL_TRUE);
			continue ;
		}
		switch (_state) {
			case STATE::MENU:
				if (glfwGetKey(_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
					_client = new Client();
					_client->setDisplay(this);
					_client->connectSocket(_ip);
					// std::cout << "debug after connect" << std::endl;
					_state = STATE::GAME;
				}
				break ;
			case STATE::GAME:
				// std::cout << "debug time" << std::endl;
				_client->setInputs(glfwGetKey(_window, GLFW_KEY_RIGHT) - glfwGetKey(_window, GLFW_KEY_LEFT),
						glfwGetKey(_window, GLFW_KEY_UP) - glfwGetKey(_window, GLFW_KEY_DOWN));
				if (!_client->handleMessages()) {
					delete _client;
					_client = NULL;
					_state = STATE::GAME;
					_rectangles.clear();
				}
				// std::cout << "over" << std::endl;
				break ;
		}
		glClear(GL_COLOR_BUFFER_BIT);
		draw_rectangles();
		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Display::setIP( std::string ip )
{
	_ip = ip;
}

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
	load_texture();
	main_loop();
}

void Display::parseServerInput( std::string str )
{
	int index = 0, x, y;
	bool xSign, ySign;
	_rectangles.clear();
	for (; str[index] && str[index] != '\n'; ++index) {
		x = 0;
		y = 0;
		xSign = false;
		if (str[index] == '-') {
			xSign = true;
			++index;
		}
		for (; isdigit(str[index]); ++index) {
			x = x * 10 + str[index] - '0';
		}
		++index;
		ySign = false;
		if (str[index] == '-') {
			ySign = true;
			++index;
		}
		for (; isdigit(str[index]); ++index) {
			y = y * 10 + str[index] - '0';
		}
		_rectangles.push_back({(xSign) ? -x : x, (ySign) ? -y : y});
	}
}
