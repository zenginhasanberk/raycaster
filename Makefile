CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

INCLUDE_PATH = -I/usr/local/include
LIBRARY_PATH = -L/usr/local/lib

LIBS = -framework OpenGL -framework GLUT

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)

EXEC = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LIBRARY_PATH) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATH) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
