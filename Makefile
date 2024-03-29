SER_NAME		= server
CLI_NAME		= client
OBJS_DIR		= Objs
SRCS_DIR		= Sources

FILES_SERVER	= main_server Chess Server
FILES_CLIENT	= main_client callbacks Chess Client Display Text utils

SER_SRCS			= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES_SERVER)))
SER_OBJS 			= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES_SERVER)))
CLI_SRCS		= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES_CLIENT)))
CLI_OBJS 		= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES_CLIENT)))

# ===---===---===---===---===---===---===---===---===---===---===---===---

CC 			= clang++
CPPFLAGS 	= -Wall -Wextra -Werror -O3 -std=c++17
SAN 		= -fsanitize=address -g3
INCLUDES	= -I Includes -I Libs/glfw/include -I Libs/SOIL/build/include -I Libs/glew-2.2.0/include
LDFLAGS		= Libs/glfw/src/libglfw3.a Libs/glew-2.2.0/build/lib/libGLEW.a Libs/SOIL/build/lib/libSOIL.a

# ===---===---===---===---===---===---===---===---===---===---===---===---

ifeq ($(shell uname), Linux)
LDFLAGS		+= -lGL -lX11 -lpthread -lXrandr -lXi -ldl #-L Libs/glew/build `pkg-config --static --libs glew` 
else
LDFLAGS		+= -framework OpenGl -framework AppKit -framework IOkit
endif

# # ===---===---===---===---===---===---===---===---===---===---===---===---

all: $(OBJS_DIR) $(SER_NAME) $(CLI_NAME)

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

setup:
	cd Libs/SOIL && ./configure && make
	cd Libs/glfw && cmake . && make
	cd Libs/glew-2.2.0/build && cmake ./cmake && make

cleanLibs:
	cd Libs/SOIL && make clean
	cd Libs/glew-2.2.0 && make clean
	cd Libs/glfw && make clean

$(SER_NAME): $(SER_OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(SER_OBJS) -o $(SER_NAME)

$(CLI_NAME): $(CLI_OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(CLI_OBJS) -o $(CLI_NAME) $(LDFLAGS)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(SER_NAME) $(CLI_NAME)

re: fclean all

rer: re
	@./$(NAME)

.PHONY: all setup cleanLibs clean fclean re rer
