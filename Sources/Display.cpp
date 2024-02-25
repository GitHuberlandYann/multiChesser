#include "Display.hpp"
#include "utils.hpp"
#include "callbacks.hpp"

#include "SOIL/SOIL.h"
typedef struct {
	unsigned char *content;
	int width;
	int height;
}				t_tex;

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT),
		_texture(NULL), _client(NULL), _state(STATE::MENU), _port(PORT), _mouse_pressed(false),
		_selected_piece({PIECES::EMPTY, -1, -1}), _ip("localhost")
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
	// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
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
	_uniHeight = glGetUniformLocation(_shaderProgram, "win_height");
	setWindowSize(_winWidth, _winHeight);

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
	loadSubTextureArray(2, "Resources/wk.png");
	loadSubTextureArray(3, "Resources/wq.png");
	loadSubTextureArray(4, "Resources/wr.png");
	loadSubTextureArray(5, "Resources/wb.png");
	loadSubTextureArray(6, "Resources/wn.png");
	loadSubTextureArray(7, "Resources/wp.png");
	loadSubTextureArray(8, "Resources/bk.png");
	loadSubTextureArray(9, "Resources/bq.png");
	loadSubTextureArray(10, "Resources/br.png");
	loadSubTextureArray(11, "Resources/bb.png");
	loadSubTextureArray(12, "Resources/bn.png");
	loadSubTextureArray(13, "Resources/bp.png");
	glUniform1i(glGetUniformLocation(_shaderProgram, "pieces"), 0);
	check_glstate("texture_2D_array done", true);
}

void Display::handleInputs( void )
{
	double mouseX, mouseY;
	glfwGetCursorPos(_window, &mouseX, &mouseY);
	if (!_mouse_pressed && glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		_mouse_pressed = true;
		_chess->setCaptures(-1);
		if (_selected_piece[0] == PIECES::EMPTY) {
			_selected_piece = _chess->getSelectedSquare(mouseX, mouseY, _squareSize);
			if (_selected_piece[0] == PIECES::EMPTY) {
				_selected_piece[2] = -1;
			}
		} else {
			std::array<int, 3> dst = _chess->getSelectedSquare(mouseX, mouseY, _squareSize);
			if (dst[1] == _selected_piece[1]) {
			} else if (_chess->forceMovePiece(_selected_piece[1], dst[1])) {
				_client->setMsg(_selected_piece[1], dst[1]);
			} else {
				_selected_piece = dst;
				_chess->setCaptures(-1);
				return ;
			}
			_chess->setCaptures(-1);
			_selected_piece = {PIECES::EMPTY, -1, -1};
		}
		// std::cout << "piece " << _selected_piece[0] << " at " << _selected_piece[1] << std::endl;
	} else if (_mouse_pressed && glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		_mouse_pressed = false;
		if (_selected_piece[0] != PIECES::EMPTY) {
			int dst = _chess->getSelectedSquare(mouseX, mouseY, _squareSize)[1];
			if (dst == _selected_piece[1]) {
				_selected_piece[2] = -1;
				_chess->setCaptures(_selected_piece[1]);
				return ;
			}
			if (_chess->forceMovePiece(_selected_piece[1], dst)) {
				_client->setMsg(_selected_piece[1], dst);
			}
			_selected_piece = {PIECES::EMPTY, -1, -1};
		}
	}

	if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		_selected_piece = {PIECES::EMPTY, -1, -1};
		_chess->setCaptures(-1);
	}
}

void Display::draw_rectangles( void )
{
	if (_client) {
		std::vector<int> vertices;
		switch (_state) {
			case STATE::WAITING_ROOM:
				double mouseX, mouseY;
				glfwGetCursorPos(_window, &mouseX, &mouseY);
				_chess->drawWaitingRoom(vertices, mouseX, mouseY, _squareSize);
				break ;
			case STATE::INGAME:
				_chess->drawBoard(vertices, _selected_piece[2], _squareSize);
				if (_selected_piece[2] != -1) {
					double mouseX, mouseY;
					glfwGetCursorPos(_window, &mouseX, &mouseY);
					_chess->drawSquare(vertices, _chess->texIndex(_selected_piece[0]), mouseX - (_squareSize >> 1), mouseY - (_squareSize >> 1), _squareSize);
				}
				break ;
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
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(1);
	glClearColor(0, 0, 0, 1.0f);
	set_window_size_callback(this);
	glfwSetWindowSizeCallback(_window, window_size_callback);

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
					_client->connectSocket(_ip, _port);
					// std::cout << "debug after connect" << std::endl;
					_state = STATE::WAITING_ROOM;
				}
				break ;
			case STATE::WAITING_ROOM:
				_client->handleMessages();
				break ;
			case STATE::INGAME:
				// std::cout << "debug time" << std::endl;
				handleInputs();
				_client->handleMessages();
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

void Display::setPort( int port )
{
	_port = port;
}

void Display::setWindowSize( int width, int height )
{
	_winWidth = width;
	glUniform1i(_uniWidth, width);
	_winHeight = height;
	glUniform1i(_uniHeight, height);
	_squareSize = (width / GOLDEN_RATIO < height) ? width / GOLDEN_RATIO : height;
	_squareSize /= 10;
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
	std::cout << str << std::flush;
	if (!str.compare(0, 5, "FEN: ")) {
		if (_state == STATE::WAITING_ROOM) {
			_state = STATE::INGAME;
		}
		_chess->setBoard(str.substr(5));
		_chess->setCaptures(-1);
	} else if (!str.compare(0, 5, "col: ")) {
		_chess->setColor(str[5]);
	}
	// else if (!str.compare(0, 5, "PGN: ")) {
	// 	_chess->setPGN(str);
	// } else if (!str.compare(0, 5, "MSG: ")) { // msg from opponent
	// } else if  (!str.compare(0, 5, "END: ")) { // game ended (checkmate / draw / pat / repetition / resign / deconnection)
	// }
}
