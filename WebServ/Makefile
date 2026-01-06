NAME		= webserv
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -pedantic
INCLUDES	= -I./http_integration/inc -I./network_layer/inc

NETWORK_DIR	= network_layer
NETWORK_INC	= $(NETWORK_DIR)/inc
NETWORK_SRC	= $(NETWORK_DIR)/src
HTTP_DIR	= http_integration
HTTP_SRC	= $(HTTP_DIR)/src
OBJ_DIR		= obj

SRCS		= $(NETWORK_SRC)/main.cpp \
			  $(NETWORK_SRC)/Logger.cpp \
			  $(NETWORK_SRC)/SocketOps.cpp \
			  $(NETWORK_SRC)/FdSetManager.cpp \
			  $(NETWORK_SRC)/Client.cpp \
			  $(NETWORK_SRC)/ServerManager.cpp \
			  $(NETWORK_SRC)/ServerManager_handlers.cpp \
			  $(NETWORK_SRC)/ServerManager_io.cpp \
			  $(HTTP_SRC)/HttpRequest.cpp \
			  $(HTTP_SRC)/Response.cpp \
			  $(HTTP_SRC)/ServerConfig.cpp \
			  $(HTTP_SRC)/ConfigParser.cpp \
			  $(HTTP_SRC)/ConfigFile.cpp \
			  $(HTTP_SRC)/Location.cpp \
			  $(HTTP_SRC)/Mime.cpp \
			  $(HTTP_SRC)/CgiHandler.cpp \
			  $(HTTP_SRC)/Utils.cpp

NETWORK_OBJS = $(patsubst $(NETWORK_SRC)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(NETWORK_SRC)/%,$(SRCS)))
HTTP_OBJS    = $(patsubst $(HTTP_SRC)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(HTTP_SRC)/%,$(SRCS)))
OBJS         = $(NETWORK_OBJS) $(HTTP_OBJS)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "✓ Built $(NAME)"

$(OBJ_DIR)/%.o: $(NETWORK_SRC)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(HTTP_SRC)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "✓ Cleaned objects"

fclean: clean
	rm -f $(NAME)
	@echo "✓ Cleaned binary"

re: fclean all

run: $(NAME)
	@echo "✓ Starting webserv with config/default.conf"
	./$(NAME) config/default.conf

.PHONY: all clean fclean re run

