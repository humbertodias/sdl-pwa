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

compile:	hello	sdl1	sdl2	sdl2-cross	loop

run:
	http-server .

format:
	find . -regextype posix-extended -regex '.*\.(c|cpp)'  -exec clang-format -style=Google -i "{}" +;

clean:
	rm -rf *.wasm sdl_.js loop.js hello.js *.o *.data
	rm -f sdl_*.html loop.html hello.html