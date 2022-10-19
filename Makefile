emcc-install:
	# Get the emsdk repo
	git clone https://github.com/emscripten-core/emsdk.git

	# Enter that directory
	cd emsdk

	# Fetch the latest version of the emsdk (not needed the first time you clone)
	git pull

	# Download and install the latest SDK tools.
	./emsdk install latest

	# Make the "latest" SDK "active" for the current user. (writes .emscripten file)
	./emsdk activate latest

	# Activate PATH and other environment variables in the current terminal
	source ./emsdk_env.sh

hello:
	emcc -c hello.c -o hello.o
	emcc hello.o -o hello.html


sdl1:
	emcc -c sdl_1_2_sample.c -o sdl_1_2_sample.o
	emcc sdl_1_2_sample.o -o sdl_1_2_sample.html

sdl2:
	emcc -c sdl_2_0_sample.c -o sdl_2_0_sample.o -s USE_SDL=2
	emcc sdl_2_0_sample.o -o sdl_2_0_sample.html -s USE_SDL=2

sdl2-cross:
	emcc -c sdl_2_0_cross.c -o sdl_2_0_cross.o -s USE_SDL=2
	emcc sdl_2_0_cross.o -o sdl_2_0_cross.html -s USE_SDL=2

loop:
	emcc -c loop.c -o loop.o -s ASYNCIFY=1
	emcc loop.o -o loop.html -s ASYNCIFY=1

snake:
	emcc -c snake.c -o snake.o -s USE_SDL=2
	emcc snake.o -o snake.html -s USE_SDL=2	

pong:
	emcc -c pong.c -o pong.o -s USE_SDL=2
	emcc pong.o -o pong.html -s USE_SDL=2	

web:
	emcc web.cpp -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_OGG=1 --use-preload-plugins  --preload-file res -o web.js

compile:	hello	sdl1	sdl2	sdl2-cross	loop	web	snake	pong

run:
	http-server .

format:
	find . -regextype posix-extended -regex '.*\.(c|cpp)'  -exec clang-format -style=Google -i "{}" +;

clean:
	rm -rf *.wasm sdl_.js loop.js hello.js snake.js pong.js *.o *.data web
	rm -f sdl_*.html loop.html hello.html snake.html pong.html
