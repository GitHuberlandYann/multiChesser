SER_NAME		= server
CLI_NAME		= client
OBJS_DIR		= Objs
SRCS_DIR		= Sources

FILES_SERVER	= main_server Server utils
FILES_CLIENT	= main_client Chess Display Client utils

SRCS			= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES_SERVER)))
OBJS 			= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES_SERVER)))
CLI_SRCS		= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES_CLIENT)))
CLI_OBJS 		= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES_CLIENT)))

# ===---===---===---===---===---===---===---===---===---===---===---===---

CC 			= clang++
CPPFLAGS 	= -Wall -Wextra -Werror -O3 -std=c++17
SAN 		= -fsanitize=address -g3
INCLUDES	= -I Includes -I Libs/glfw/include
LDFLAGS		= Libs/glfw/src/libglfw3.a Libs/SOIL/build/lib/libSOIL.a

# ===---===---===---===---===---===---===---===---===---===---===---===---

ifeq ($(shell uname), Linux)
LDFLAGS		+= -L Libs/glew/build `pkg-config --static --libs glew` -lGL -lX11 -lpthread -lXrandr -lXi -ldl 
else
LDFLAGS		+= -framework OpenGl -framework AppKit -framework IOkit Libs/mac/libGLEW.a # todo check if glew compiles on mac
endif

# # ===---===---===---===---===---===---===---===---===---===---===---===---

all: $(OBJS_DIR) $(SER_NAME) $(CLI_OBJS_DIR) $(CLI_NAME)

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

setup:
	cd Libs/SOIL && ./configure && make
	cd Libs/glfw && cmake . && make
	cd Libs/glew/build && cmake ./cmake && make

cleanLibs:
	cd Libs/SOIL && make clean
	cd Libs/glew && make clean
	cd Libs/glfw && make clean

$(SER_NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(OBJS) -o $(SER_NAME)

$(CLI_NAME): $(CLI_OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(CLI_OBJS) -o $(CLI_NAME) $(LDFLAGS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR) $(CLI_OBJS_DIR)

fclean: clean
	rm -f $(SER_NAME) $(CLI_NAME)

re: fclean all

rer: re
	@./$(NAME)

.PHONY: all setup cleanLibs clean fclean re rer
