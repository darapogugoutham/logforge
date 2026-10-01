CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror
CPPFLAGS += -Iinclude
LDLIBS += -pthread

.PHONY: all test clean
all: build/logforge
build:
	mkdir -p build
build/logforge: src/main.cpp src/logforge.cpp include/logforge.hpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/main.cpp src/logforge.cpp $(LDLIBS) -o $@
build/tests: tests/test.cpp src/logforge.cpp include/logforge.hpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/test.cpp src/logforge.cpp $(LDLIBS) -o $@
test: build/tests build/logforge
	./build/tests
	python3 tests/integration.py
	python3 tests/scripts.py
clean:
	rm -rf build
