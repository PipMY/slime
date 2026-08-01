# Variables
CXX = clang++
CXXFLAGS = -std=c++26 -I include

SRCS = src/main.cpp src/tgaimage.cpp src/obj_decoder.cpp src/drawing.cpp src/lighting.cpp
OBJS = $(SRCS:src/%.cpp=build/%.o)

program: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o program

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build program

run: program
	./program
