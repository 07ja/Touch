CXX = clang++
CC = clang

CXXFLAGS = -Wall -std=c++17 -Iinclude/imgui -Iinclude/imgui/backends -Iinclude \
           -I/opt/homebrew/opt/sdl3/include -I/opt/homebrew/opt/sdl3_image/include \
           -I/opt/homebrew/Cellar/onnxruntime/1.26.0_1/include/onnxruntime

LDFLAGS = -L/opt/homebrew/opt/sdl3/lib -L/opt/homebrew/opt/sdl3_image/lib \
          -L/opt/homebrew/Cellar/onnxruntime/1.26.0_1/lib

LIBS = -lSDL3 -lSDL3_image -lonnxruntime

CPPSRC = src/*.cpp src/imgui/*.cpp src/imgui/backends/*.cpp

OUT = Touch

build:
	$(CXX) $(CXXFLAGS) $(CPPSRC) -o $(OUT) $(LDFLAGS) $(LIBS)

run: build
	DYLD_LIBRARY_PATH=/opt/homebrew/opt/sdl3/lib:/opt/homebrew/opt/sdl3_image/lib:/opt/homebrew/Cellar/onnxruntime/1.26.0_1/lib ./$(OUT)

clean:
	rm -f $(OUT)