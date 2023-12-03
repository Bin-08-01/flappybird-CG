CPPFLAGS = -Wall -Wextra -Wpedantic
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

all: flappy-bird

flappy-bird: source.cpp
	g++ $(CPPFLAGS) -o $@ $^ $(LIBS)

run:
	./flappy-bird

clean:
	rm -rf flappy-bird