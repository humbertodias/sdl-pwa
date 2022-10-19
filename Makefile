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

web:
	emcc web.cpp -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS='["ogg"]' -s USE_OGG=1 --preload-file res -o web.js

run:
	http-server .

format:
	find . -regextype posix-extended -regex '.*\.(c|cpp)'  -exec clang-format -style=Google -i "{}" +;

clean:
	rm -rf web.js *.wasm *.o *.data web
	$(MAKE) -C examples clean
	$(MAKE) -C games clean