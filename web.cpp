#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <emscripten.h>

#include <iostream>
#include <vector>

struct context {
  std::string title;
  int width, height;
  SDL_Renderer *renderer;
  SDL_Point mouse;
  const Uint8 *keystates;
  Uint32 mousestate;
  SDL_Texture *screen;
  SDL_Rect screensize;
  SDL_Event event;
  SDL_Color bkg = {0, 255, 255, 255};

  int frameCount, timerFPS, lastFrame, fps, lastTime;
  int setFPS = 60;

  int logo;
  std::vector<SDL_Texture *> images;

  int font_size = 32;
  TTF_Font *font;
  SDL_Color font_color = {255, 255, 255, 255};
  Mix_Music* music;
};
int loadImage(std::string filename, context *ctx) {
  ctx->images.push_back(IMG_LoadTexture(ctx->renderer, filename.c_str()));
  return ctx->images.size() - 1;
}
void drawImage(int img, SDL_Rect dest, SDL_Rect src, context *ctx) {
  SDL_RenderCopy(ctx->renderer, ctx->images[img], &src, &dest);
}

void writeText(std::string t, int x, int y, context *ctx) {
  const char *text = t.c_str();
  SDL_Surface *text_surface =
      TTF_RenderText_Solid(ctx->font, text, ctx->font_color);
  SDL_Texture *text_texture =
      SDL_CreateTextureFromSurface(ctx->renderer, text_surface);
  SDL_Rect wrect = {x, y, text_surface->w, text_surface->h};
  SDL_FreeSurface(text_surface);
  SDL_RenderCopy(ctx->renderer, text_texture, NULL, &wrect);
  SDL_DestroyTexture(text_texture);
}

void updateKeys(context *ctx) {
  ctx->keystates = SDL_GetKeyboardState(NULL);
  ctx->mousestate = SDL_GetMouseState(&ctx->mouse.x, &ctx->mouse.y);
}

void setDrawColor(SDL_Color c, context *ctx) {
  SDL_SetRenderDrawColor(ctx->renderer, c.r, c.g, c.b, c.a);
}

void begin_render(context *ctx) {
  SDL_SetRenderTarget(ctx->renderer, ctx->screen);
  setDrawColor(ctx->bkg, ctx);
  SDL_RenderClear(ctx->renderer);
  ctx->frameCount++;
  ctx->timerFPS = SDL_GetTicks() - ctx->lastFrame;
  if (ctx->timerFPS < (1000 / ctx->setFPS)) {
    SDL_Delay((1000 / ctx->setFPS) - ctx->timerFPS);
  }
}

void end_render(context *ctx) {
  SDL_SetRenderTarget(ctx->renderer, NULL);
  SDL_RenderCopy(ctx->renderer, ctx->screen, &ctx->screensize,
                 &ctx->screensize);
  SDL_RenderPresent(ctx->renderer);
}

void load_and_play_music(float volume, std::string file, Mix_Music* music)
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0) {
        int flags = MIX_INIT_OGG;
        int initted = Mix_Init(flags);
        if ((initted & flags) != flags) {
            printf("Mix_Init: Failed to init required ogg support!\n");
            printf("Mix_Init: %s\n", Mix_GetError());
            // We can still continue without audio. :^(
        } else {
            Mix_VolumeMusic(volume);
            music = Mix_LoadMUS(file.c_str());
            if (!Mix_PlayingMusic()) {
                Mix_PlayMusic(music, -1);
            }
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error initializing SDL_mixer: %s\n", Mix_GetError());
        // No need to exit, we just play without sound.
    }
}

void init(context *ctx) {
  ctx->screensize.x = ctx->screensize.y = 0;
  ctx->screensize.w = ctx->width;
  ctx->screensize.h = ctx->height;
  ctx->screen =
      SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, ctx->width, ctx->height);

  ctx->logo = loadImage("res/logo.png", ctx);
  
  TTF_Init();
  ctx->font = TTF_OpenFont("res/peepo.ttf", ctx->font_size);

  load_and_play_music(-1, "res/music.ogg", ctx->music);

}

void quit(context *ctx) {
  SDL_DestroyRenderer(ctx->renderer);
  SDL_Quit();
}

void loop(context *ctx) {
  end_render(ctx);
  ctx->lastFrame = SDL_GetTicks();
  if (ctx->lastFrame >= (ctx->lastTime + 1000)) {
    ctx->lastTime = ctx->lastFrame;
    ctx->fps = ctx->frameCount;
    ctx->frameCount = 0;
  }
  updateKeys(ctx);
  begin_render(ctx);
}

void mainloop(void *arg) {
  context *ctx = static_cast<context *>(arg);
  loop(ctx);
  drawImage(ctx->logo, {ctx->mouse.x, ctx->mouse.y, 38 * 4, 28 * 4},
            {0, 0, 38, 28}, ctx);
  writeText(std::to_string(ctx->fps), 10, 0, ctx);
  // std::cout << ctx->mouse.x << ", " << ctx->mouse.y << std::endl;
}

int main() {
  context ctx;
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  ctx.width = 700, ctx.height = 500;
  SDL_Window *window;
  ctx.title = "web test";
  SDL_SetWindowTitle(window, ctx.title.c_str());
  SDL_Renderer *renderer;
  SDL_CreateWindowAndRenderer(ctx.width, ctx.height, 0, &window, &renderer);
  ctx.renderer = renderer;

  init(&ctx);

  emscripten_set_main_loop_arg(mainloop, &ctx, -1, 1);

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  quit(&ctx);
  return 0;
}