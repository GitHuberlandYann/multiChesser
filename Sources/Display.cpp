#include "Display.hpp"
#include "utils.hpp"

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT),
		_state(STATE::MENU)
{

}

Display::~Display( void )
{
	std::cout << "Destructor of display called" << std::endl;

	glDeleteProgram(_shaderProgram);

	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);

	glfwMakeContextCurrent(NULL);
    glfwTerminate();
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

	glBindAttribLocation(_shaderProgram, 0, "position");

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

void Display::draw_rectangles( void )
{
	size_t rSize = _rectangles.size();
	if (rSize) {
		std::vector<int> vertices;
		for (auto pos : _rectangles) {
			vertices.push_back(pos[0]);
			vertices.push_back(pos[1]);
			vertices.push_back(pos[0] + RECT_SIZE);
			vertices.push_back(pos[1]);
			vertices.push_back(pos[0]);
			vertices.push_back(pos[1] + RECT_SIZE);

			vertices.push_back(pos[0] + RECT_SIZE);
			vertices.push_back(pos[1]);
			vertices.push_back(pos[0] + RECT_SIZE);
			vertices.push_back(pos[1] + RECT_SIZE);
			vertices.push_back(pos[0]);
			vertices.push_back(pos[1] + RECT_SIZE);
		}
		glUseProgram(_shaderProgram);
		glBindVertexArray(_vao);

		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, rSize * 12 * sizeof(GLint), &(vertices[0]), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribIPointer(0, 2, GL_INT, 2 * sizeof(GLint), 0);

		check_glstate("Display::draw_rectangles", false);
		glDrawArrays(GL_TRIANGLES, 0, rSize * 12);
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
					_client->connectSocket("localhost");
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

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
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
