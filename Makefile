CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

SRC := src/main.cpp src/simulator.cpp
BIN := neural_decoder

$(BIN): $(SRC) src/*.hpp
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: run clean
