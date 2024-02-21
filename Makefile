NAME			= server
CLI_NAME		= client
OBJS_DIR		= Objs
SRCS_DIR		= Sources

FILES_SERVER	= main_server Server utils
FILES_CLIENT	= main_client Client utils

SRCS			= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES_SERVER)))
OBJS 			= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES_SERVER)))
CLI_SRCS		= $(addprefix $(SRCS_DIR)/, $(addsuffix .cpp, $(FILES_CLIENT)))
CLI_OBJS 		= $(addprefix $(OBJS_DIR)/, $(addsuffix .o, $(FILES_CLIENT)))

# ===---===---===---===---===---===---===---===---===---===---===---===---

CC 			= clang++
CPPFLAGS 	= -Wall -Wextra -Werror -O3 -std=c++17
SAN 		= -fsanitize=address -g3
INCLUDES	= -I Includes

# # ===---===---===---===---===---===---===---===---===---===---===---===---

all: $(OBJS_DIR) $(NAME) $(CLI_OBJS_DIR) $(CLI_NAME)

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(OBJS) -o $(NAME)

$(CLI_NAME): $(CLI_OBJS)
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) $(CLI_OBJS) -o $(CLI_NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(SAN) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR) $(CLI_OBJS_DIR)

fclean: clean
	rm -f $(NAME) $(CLI_NAME)

re: fclean all

run: all
	@./$(NAME)

log: all
	@mkdir -p Logs
	@./$(NAME) > Logs/.log 2> Logs/err.log

rer: re
	@./$(NAME)

.PHONY: all clean fclean re run log rer
