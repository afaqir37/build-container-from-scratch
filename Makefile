CXX=c++
SRC=container.cpp
OBJ=$(SRC:.cpp=.o)
RM=rm -rf
NAME=container

all: NAME

NAME: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all
