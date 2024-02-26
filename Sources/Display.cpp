#include "Display.hpp"
#include "callbacks.hpp"

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT),
		_texture(NULL), _client(NULL), _state(STATE::MENU), _port(PORT),
		_selection(0), _input_released(0), _mouse_pressed(false),
		_selected_piece({PIECES::EMPTY, -1, -1}), _ip("localhost")
{
	_chess = new Chess();
	_text = new Text();
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

	delete _client;
	delete _chess;
	delete _text;
	check_glstate("Display successfully destructed", true);
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

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
	_text->setup_shader();

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
	_text->setup_communication_shaders();

	_uniWidth = glGetUniformLocation(_shaderProgram, "win_width");
	_uniHeight = glGetUniformLocation(_shaderProgram, "win_height");
	setWindowSize(_winWidth, _winHeight);

	check_glstate("\nCommunication with shader program successfully established", true);

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	check_glstate("VAO and VBO", false);
}

void Display::load_texture( void )
{
	_text->load_texture();
	glUseProgram(_shaderProgram);

	_texture = new GLuint[1];
	glGenTextures(1, _texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture[0]);

	// alocate pixel data
	// mipmap level set to 1
	// works because all images are 300x300
	// layerCount is 15 (black + white + 6 white pieces + 6 black pieces + highlight)
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 300, 300, 15);
	loadSubTextureArray(TEXTURE::BLACK_SQUARE, "Resources/back_black.png");
	loadSubTextureArray(TEXTURE::WHITE_SQUARE, "Resources/back_white.png");
	loadSubTextureArray(TEXTURE::WHITE_KING, "Resources/wk.png");
	loadSubTextureArray(TEXTURE::WHITE_QUEEN, "Resources/wq.png");
	loadSubTextureArray(TEXTURE::WHITE_ROOK, "Resources/wr.png");
	loadSubTextureArray(TEXTURE::WHITE_BISHOP, "Resources/wb.png");
	loadSubTextureArray(TEXTURE::WHITE_KNIGHT, "Resources/wn.png");
	loadSubTextureArray(TEXTURE::WHITE_PAWN, "Resources/wp.png");
	loadSubTextureArray(TEXTURE::BLACK_KING, "Resources/bk.png");
	loadSubTextureArray(TEXTURE::BLACK_QUEEN, "Resources/bq.png");
	loadSubTextureArray(TEXTURE::BLACK_ROOK, "Resources/br.png");
	loadSubTextureArray(TEXTURE::BLACK_BISHOP, "Resources/bb.png");
	loadSubTextureArray(TEXTURE::BLACK_KNIGHT, "Resources/bn.png");
	loadSubTextureArray(TEXTURE::BLACK_PAWN, "Resources/bp.png");
	loadSubTextureArray(TEXTURE::MOVE_HIGHLIGHT, "Resources/highlight.png");
	glUniform1i(glGetUniformLocation(_shaderProgram, "pieces"), 0);
	check_glstate("texture_2D_array done", true);
}

void Display::handleMenuInputs( void )
{
	std::string username = INPUT::getCurrentMessage();
	if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		double mouseX, mouseY;
		glfwGetCursorPos(_window, &mouseX, &mouseY);
		if (_selection == SELECT::USERNAME) {
			_state = STATE::INPUT;
			glfwSetCharCallback(_window, INPUT::character_callback);
		} else if (_selection == SELECT::PLAY && username[0]) {
			goto WAITING_ROOM;
		} else {
			_state = STATE::MENU;
			glfwSetCharCallback(_window, NULL);
		}
	}

	if (_state == STATE::INPUT) {
		if (++_input_released == 1 && glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			INPUT::rmLetter();
		}
		if (_input_released == 1 && glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			INPUT::moveCursor(true, glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
		}
		if (_input_released == 1 && glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			INPUT::moveCursor(false, glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
		}
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE
			&& glfwGetKey(_window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
			_input_released = 0;
		}
		if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			_state = STATE::MENU;
			glfwSetCharCallback(_window, NULL);
		} else if (glfwGetKey(_window, GLFW_KEY_ENTER) == GLFW_PRESS && username[0]) {
			goto WAITING_ROOM;
		}
	}
	return ;
	WAITING_ROOM:
	_username = username;
	glfwSetCharCallback(_window, NULL);
	glfwSetCursorPosCallback(_window, NULL);
	_client = new Client();
	_client->setDisplay(this);
	_client->connectSocket(_ip, _port, username);
	// std::cout << "debug after connect" << std::endl;
	_state = STATE::WAITING_ROOM;
	glfwSetWindowTitle(_window, "Waiting room.");
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
				_selected_piece = dst;
				return ;
			} else if (_chess->forceMovePiece(_selected_piece[1], dst[1])) {
				_client->setMsg(_selected_piece[1], dst[1]);
			} else {
				_selected_piece = dst;
				return ;
			}
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

void Display::drawRectangle( std::vector<int> &vertices, int type, int startX, int startY, int width, int height )
{
	// std::cout << "rectangle with layer " << type << std::endl;
	vertices.push_back(0 + (0 << 1) + (type << 2));
	vertices.push_back(startX);
	vertices.push_back(startY);
	vertices.push_back(1 + (0 << 1) + (type << 2));
	vertices.push_back(startX + width);
	vertices.push_back(startY);
	vertices.push_back(0 + (1 << 1) + (type << 2));
	vertices.push_back(startX);
	vertices.push_back(startY + height);

	vertices.push_back(1 + (0 << 1) + (type << 2));
	vertices.push_back(startX + width);
	vertices.push_back(startY);
	vertices.push_back(1 + (1 << 1) + (type << 2));
	vertices.push_back(startX + width);
	vertices.push_back(startY + height);
	vertices.push_back(0 + (1 << 1) + (type << 2));
	vertices.push_back(startX);
	vertices.push_back(startY + height);
}

void Display::draw( void )
{
	std::vector<int> vertices;
	std::string username;
	int gui_size;
	switch (_state) {
		case STATE::INPUT:
			username = INPUT::getCurrentInputStr('|');
		case STATE::MENU:
			if (!username[0]) username = INPUT::getCurrentMessage();
			gui_size = _squareSize / 3;
			_text->addText(_winWidth / 4 + gui_size, _winHeight / 4 - gui_size * 1.25f, 2 * gui_size / 3, true, "Username");
			_text->addText(_winWidth / 2 - 0.5f * gui_size * username.size(), _winHeight / 4 + (_squareSize - gui_size) / 2, gui_size, true, username);
			_text->addText(_winWidth / 2 - 2 * gui_size - 1, 3 * _winHeight / 4 - (_squareSize + 4 * gui_size / 3) / 2 - 1, 4 * gui_size / 3, false, "PLAY");
			_text->addText(_winWidth / 2 - 2 * gui_size, 3 * _winHeight / 4 - (_squareSize + 4 * gui_size / 3) / 2, 4 * gui_size / 3, true, "PLAY");
			drawRectangle(vertices, 0, _winWidth / 4, _winHeight / 4, _winWidth / 2, _squareSize);
			drawRectangle(vertices, _selection == SELECT::PLAY && username.size() > 1, _winWidth / 4, 3 * _winHeight / 4 - _squareSize, _winWidth / 2, _squareSize);
			break ;
		case STATE::WAITING_ROOM:
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			_chess->drawWaitingRoom(vertices, mouseX, mouseY, _squareSize, _winWidth, _winHeight);
			break ;
		case STATE::INGAME:
			_text->addText(_squareSize * 5 - _squareSize / 6 * _username.size(), _winHeight - 4 * _squareSize / 5, _squareSize / 3, true, _username);
			_text->addText(_squareSize * 5 - _squareSize / 6 * _opponent_username.size(), 4 * _squareSize / 5 - _squareSize / 3, _squareSize / 3, true, _opponent_username);
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
	
	check_glstate("Display::draw", false);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
	// std::cout << "displaying resctangless " << vertices.size() << std::endl;
}

void Display::main_loop( void )
{
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(1);
	glClearColor(0, 0, 0, 1.0f);
	set_display_callback(this);
	glfwSetWindowSizeCallback(_window, window_size_callback);
	glfwSetCursorPosCallback(_window, cursor_position_callback);

	check_glstate("setup done, entering main loop\n", true);

	while (!glfwWindowShouldClose(_window)) {
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && _state != STATE::INPUT) {
			glfwSetWindowShouldClose(_window, GL_TRUE);
			continue ;
		}
		switch (_state) {
			case STATE::MENU:
			case STATE::INPUT:
				handleMenuInputs();
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
		draw();
		_text->toScreen();
		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

static bool inRectangle( float posX, float posY, int rx, int ry, int width, int height )
{
	return (posX >= rx && posX <= rx + width && posY >= ry && posY <= ry + height);
}

void Display::setSelection( float posX, float posY )
{
	if (inRectangle(posX, posY, _winWidth / 4, _winHeight / 4, _winWidth / 2, _squareSize)) {
		_selection = SELECT::USERNAME;
	} else if (inRectangle(posX, posY, _winWidth / 4, 3 * _winHeight / 4 - _squareSize, _winWidth / 2, _squareSize)) {
		_selection = SELECT::PLAY;
	} else {
		_selection = SELECT::NONE;
	}
}

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
	_text->setWindowSize(width, height);
	glUseProgram(_shaderProgram);

	_winWidth = width;
	glUniform1i(_uniWidth, width);
	_winHeight = height;
	glUniform1i(_uniHeight, height);
	_squareSize = (width / GOLDEN_RATIO < height) ? width / GOLDEN_RATIO : height;
	_squareSize /= 10;
}

void Display::parseServerInput( std::string str )
{
	std::cout << str << std::flush;
	if (!str.compare(0, 5, "FEN: ")) {
		if (_state == STATE::WAITING_ROOM) {
			_state = STATE::INGAME;
			glfwSetWindowTitle(_window, "multiChesser");
		}
		_chess->setBoard(str.substr(5));
		_chess->setCaptures(-1);
	} else if (!str.compare(0, 5, "col: ")) {
		_chess->setColor(str[5]);
	} else if (!str.compare(0, 5, "OPP: ")) {
		_opponent_username = str.substr(5, str.size() - 6);
	}
	// else if (!str.compare(0, 5, "PGN: ")) {
	// 	_chess->setPGN(str);
	// } else if (!str.compare(0, 5, "MSG: ")) { // msg from opponent
	// } else if  (!str.compare(0, 5, "END: ")) { // game ended (checkmate / draw / pat / repetition / resign / deconnection)
	// }
}

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
	load_texture();
	main_loop();
}
