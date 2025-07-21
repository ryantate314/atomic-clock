CXX = g++
CXXFLAGS = -std=c++11 -Wall
SRC = test.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = test

# Add other source files if needed, e.g., NativeUDP.cpp
# SRC = test.cpp NativeUDP.cpp

all: $(TARGET)

$(TARGET): $(SRC)
    $(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)
:Wall

run: $(TARGET)
    ./$(TARGET)

clean:
    rm -f $(TARGET) $(OBJ)