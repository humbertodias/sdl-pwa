#include <SDL.h>
#include <emscripten.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Surface* screen = SDL_SetVideoMode(512, 512, 32, SDL_SWSURFACE);

  while (1) {
    if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

    Uint8* pixels = screen->pixels;

    for (int i = 0; i < 1048576; i++) {
      char randomByte = rand() % 255;
      pixels[i] = randomByte;
    }

    if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

    SDL_Flip(screen);

    emscripten_sleep(16);
  }
}
