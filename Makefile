# Variables
CXX = clang++
CXXFLAGS = -std=c++26

program: main.o tgaimage.o obj_decoder.o drawing.o
	$(CXX) $(CXXFLAGS) main.o tgaimage.o obj_decoder.o drawing.o -o program

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

tgaimage.o: tgaimage.cpp
	$(CXX) $(CXXFLAGS) -c tgaimage.cpp

obj_decoder.o: obj_decoder.cpp
	$(CXX) $(CXXFLAGS) -c obj_decoder.cpp

drawing.o: drawing.cpp
	$(CXX) $(CXXFLAGS) -c drawing.cpp

clean:
	rm -rf *.o program

run: program
	./program
