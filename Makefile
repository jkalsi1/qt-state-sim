# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = tests
SRC = tests.cpp
BENCHMARK_TARGET = grover/benchmark
BENCHMARK_SRC = grover/benchmark.cpp

# Default target (runs when you just type 'make')
all: $(TARGET)

# Link the program
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Build the benchmark
$(BENCHMARK_TARGET): $(BENCHMARK_SRC)
	$(CXX) $(CXXFLAGS) -O2 -o $(BENCHMARK_TARGET) $(BENCHMARK_SRC)

# A convenience target to build and run immediately
run: $(TARGET)
	./$(TARGET)

# Build and run the benchmark
benchmark: $(BENCHMARK_TARGET)
	./$(BENCHMARK_TARGET)

# Clean up the binary
clean:
	rm -f $(TARGET) $(BENCHMARK_TARGET)
