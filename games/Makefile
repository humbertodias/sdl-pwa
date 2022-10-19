snake:
	emcc -c snake.c -o snake.o -s USE_SDL=2
	emcc snake.o -o snake.html -s USE_SDL=2	

pong:
	emcc -c pong.c -o pong.o -s USE_SDL=2
	emcc pong.o -o pong.html -s USE_SDL=2	

web:
	emcc web.cpp -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_OGG=1 --preload-file res -o web.js

compile:	snake	pong

run:
	http-server .

format:
	find . -regextype posix-extended -regex '.*\.(c|cpp)'  -exec clang-format -style=Google -i "{}" +;

clean:
	rm -rf *.wasm snake.js pong.js *.o *.data web
	rm -f snake.html pong.html
