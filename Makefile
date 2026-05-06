# Variables
CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = tests
SRC = tests.cpp
BENCHMARK_TARGET = grover/benchmark
BENCHMARK_SRC = grover/benchmark.cpp
LIMIT_TARGET = qubit_limit_test
LIMIT_SRC = qubit_limit_test.cpp
HEADERS = state.h gates.h circuit.h complex.h

# Default target (runs when you just type 'make')
all: $(TARGET)

# Link the program
$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Build the benchmark
$(BENCHMARK_TARGET): $(BENCHMARK_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -O2 -o $(BENCHMARK_TARGET) $(BENCHMARK_SRC)

# A convenience target to build and run immediately
run: $(TARGET)
	./$(TARGET)

# Build and run the benchmark
benchmark: $(BENCHMARK_TARGET)
	./$(BENCHMARK_TARGET)

# Build and run the qubit limit stress test
limit: $(LIMIT_TARGET)
	./$(LIMIT_TARGET)

$(LIMIT_TARGET): $(LIMIT_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(LIMIT_TARGET) $(LIMIT_SRC)

# Clean up the binary
clean:
	rm -f $(TARGET) $(BENCHMARK_TARGET) $(LIMIT_TARGET)
