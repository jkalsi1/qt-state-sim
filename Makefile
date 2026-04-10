# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = tests
SRC = tests.cpp

# Default target (runs when you just type 'make')
all: $(TARGET)

# Link the program
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# A convenience target to build and run immediately
run: $(TARGET)
	./$(TARGET)

# Clean up the binary
clean:
	rm -f $(TARGET)
