/*
 * snake.c - The classic snake game written in ANSI C
 * Compile with: `gcc snake.c -ansi -pedantic -Wall -Wextra -Werror -o snake`
 * Link with: `-lSDL2 -pthread` (taken from `pkg-config --libs --cflags sdl2`)
 */
#include <stdlib.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
/* NOTE: Remember to compile with `-s USE_SDL=2` if using emscripten. */
#include <SDL2/SDL.h>
#include <emscripten.h>
#define SDL_REN_FLAGS (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
#else
#include <SDL.h>
#define SDL_REN_FLAGS (SDL_RENDERER_ACCELERATED)
#endif /* __EMSCRIPTEN__ */

#define SNAKE_GAME_WIDTH 18
#define SNAKE_GAME_HEIGHT 18
#define SNAKE_STEP_RATE_IN_MILLISECONDS 125
#define SNAKE_BLOCK_SIZE_IN_PIXELS 24
#define SNAKE_MATRIX_SIZE (SNAKE_GAME_WIDTH * SNAKE_GAME_HEIGHT)
#define SDL_TOTAL_WINDOW_WIDTH (SNAKE_BLOCK_SIZE_IN_PIXELS * SNAKE_GAME_WIDTH)
#define SDL_TOTAL_WINDOW_HEIGHT (SNAKE_BLOCK_SIZE_IN_PIXELS * SNAKE_GAME_HEIGHT)
/* Used to ease access to snake's body. */
#define X_ 0
#define Y_ 1

struct SnakeContext {
  struct Body {
    int length;    /* Length of the snake. */
    enum Direction /* All snake's possible directions. */
    {
      SNAKE_DIR_RIGHT,
      SNAKE_DIR_UP,
      SNAKE_DIR_LEFT,
      SNAKE_DIR_DOWN
    } prev_dir,
        dir;
    /* Position of each block of the snake. */
    int pos[SNAKE_MATRIX_SIZE][2U];
  } body;

  struct Food {
    int x;
    int y;
  } food;
};

static void snake_context_new_food_pos(struct SnakeContext* ctx);
static void snake_context_initialize(struct SnakeContext* ctx);
static void snake_context_step(struct SnakeContext* ctx);
static void snake_contex_redir(struct SnakeContext* ctx, enum Direction dir);

struct MainLoopPayload {
  SDL_Renderer* renderer;
  struct SnakeContext snake_ctx;
};

static Uint32 sdl_timer_callback(Uint32 interval, void* payload);
static int sdl_main_loop(struct MainLoopPayload* payload);
#ifdef __EMSCRIPTEN__
static void emscripten_main_loop(void* payload);
#endif /* __EMSCRIPTEN__ */

int main(int argc, char* argv[]) {
  int exit_value;
  SDL_Window* window;
  struct MainLoopPayload loop_payload;
  SDL_TimerID step_timer;
  (void)argc;
  (void)argv;
  exit_value = 0;
  window = NULL;
  loop_payload.renderer = NULL;
  step_timer = 0;
  srand(time(NULL));
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    exit_value = 1;
    goto quit;
  }
  window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, SDL_TOTAL_WINDOW_WIDTH,
                            SDL_TOTAL_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    exit_value = 2;
    goto quit;
  }
  loop_payload.renderer = SDL_CreateRenderer(window, -1, SDL_REN_FLAGS);
  if (loop_payload.renderer == NULL) {
    exit_value = 3;
    goto quit;
  }
  snake_context_initialize(&loop_payload.snake_ctx);
  step_timer =
      SDL_AddTimer(SNAKE_STEP_RATE_IN_MILLISECONDS, sdl_timer_callback, NULL);
  if (step_timer == 0) {
    exit_value = 4;
    goto quit;
  }
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(emscripten_main_loop, &loop_payload, -1, 1);
#else
  while (sdl_main_loop(&loop_payload)) {
  }
#endif /* __EMSCRIPTEN__ */
quit:
  if (exit_value > 0) fprintf(stderr, "%s", SDL_GetError());
  SDL_RemoveTimer(step_timer);
  SDL_DestroyRenderer(loop_payload.renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return exit_value;
}

static int snake_context_is_food_inside_snake(struct SnakeContext* ctx) {
  int i;
  for (i = 0; i < ctx->body.length; i++) {
    if (ctx->food.x == ctx->body.pos[i][X_] &&
        ctx->food.y == ctx->body.pos[i][Y_])
      return 1;
  }
  return 0;
}

void snake_context_new_food_pos(struct SnakeContext* ctx) {
  for (;;) {
    ctx->food.x = rand() % SNAKE_GAME_WIDTH;
    ctx->food.y = rand() % SNAKE_GAME_HEIGHT;
    if (snake_context_is_food_inside_snake(ctx) == 0) return;
  }
}

void snake_context_initialize(struct SnakeContext* ctx) {
  int i;
  ctx->body.dir = ctx->body.prev_dir = SNAKE_DIR_RIGHT;
  ctx->body.length = 3;
  for (i = 0; i < SNAKE_MATRIX_SIZE; i++) {
    ctx->body.pos[i][X_] = 8;
    ctx->body.pos[i][Y_] = 8;
  }
  snake_context_new_food_pos(ctx);
}

void snake_context_step(struct SnakeContext* ctx) {
  int i;
  /* Update the snake's body. */
  for (i = ctx->body.length; i > 0; i--) {
    ctx->body.pos[i][X_] = ctx->body.pos[i - 1][X_];
    ctx->body.pos[i][Y_] = ctx->body.pos[i - 1][Y_];
  }
  /* Move snake's head based on direction. */
  switch (ctx->body.dir) {
    case SNAKE_DIR_RIGHT:
      ctx->body.pos[0][X_]++;
      break;
    case SNAKE_DIR_UP:
      ctx->body.pos[0][Y_]--;
      break;
    case SNAKE_DIR_LEFT:
      ctx->body.pos[0][X_]--;
      break;
    case SNAKE_DIR_DOWN:
      ctx->body.pos[0][Y_]++;
      break;
  }
  ctx->body.prev_dir = ctx->body.dir;
  /* Warp to the other side of the screen if necessary. */
  if (ctx->body.pos[0][X_] == SNAKE_GAME_WIDTH)
    ctx->body.pos[0][X_] = 0;
  else if (ctx->body.pos[0][X_] == -1)
    ctx->body.pos[0][X_] = SNAKE_GAME_WIDTH - 1;
  else if (ctx->body.pos[0][Y_] == SNAKE_GAME_HEIGHT)
    ctx->body.pos[0][Y_] = 0;
  else if (ctx->body.pos[0][Y_] == -1)
    ctx->body.pos[0][Y_] = SNAKE_GAME_HEIGHT - 1;
  /* Check snake's head collision with food... */
  if (ctx->food.x == ctx->body.pos[0][X_] &&
      ctx->food.y == ctx->body.pos[0][Y_]) {
    snake_context_new_food_pos(ctx);
    ctx->body.length++;
  }
  /* Check snake's collision with itself... */
  for (i = 1; i < ctx->body.length; i++) {
    if (ctx->body.pos[0][X_] == ctx->body.pos[i][X_] &&
        ctx->body.pos[0][Y_] == ctx->body.pos[i][Y_]) {
      snake_context_initialize(ctx);
      return;
    }
  }
}

void snake_contex_redir(struct SnakeContext* ctx, enum Direction dir) {
  if (dir == SNAKE_DIR_RIGHT && ctx->body.prev_dir != SNAKE_DIR_LEFT)
    ctx->body.dir = SNAKE_DIR_RIGHT;
  else if (dir == SNAKE_DIR_UP && ctx->body.prev_dir != SNAKE_DIR_DOWN)
    ctx->body.dir = SNAKE_DIR_UP;
  else if (dir == SNAKE_DIR_LEFT && ctx->body.prev_dir != SNAKE_DIR_RIGHT)
    ctx->body.dir = SNAKE_DIR_LEFT;
  else if (dir == SNAKE_DIR_DOWN && ctx->body.prev_dir != SNAKE_DIR_UP)
    ctx->body.dir = SNAKE_DIR_DOWN;
}

Uint32 sdl_timer_callback(Uint32 interval, void* payload) {
  SDL_Event e;
  SDL_UserEvent ue;
  /* NOTE: snake_context_step is not directly called here for
   * multithreaded concerns.
   */
  (void)payload;
  ue.type = SDL_USEREVENT;
  ue.code = 0;
  ue.data1 = NULL;
  ue.data2 = NULL;
  e.type = SDL_USEREVENT;
  e.user = ue;
  SDL_PushEvent(&e);
  return interval;
}

int sdl_main_loop(struct MainLoopPayload* payload) {
  SDL_Event e;
  SDL_Rect r;
  int i;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
      case SDL_QUIT:
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif /* __EMSCRIPTEN__ */
        return 0;
      case SDL_USEREVENT:
        /* Its our timer, time to update! */
        snake_context_step(&payload->snake_ctx);
        break;
      case SDL_KEYDOWN:
        switch (e.key.keysym.scancode) {
          case SDL_SCANCODE_ESCAPE:
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif /* __EMSCRIPTEN__ */
            return 0;
          /* Restart the game as if the program was launched. */
          case SDL_SCANCODE_R:
            snake_context_initialize(&payload->snake_ctx);
            break;
          /* Decide new direction of the snake. */
          case SDL_SCANCODE_RIGHT:
            snake_contex_redir(&payload->snake_ctx, SNAKE_DIR_RIGHT);
            break;
          case SDL_SCANCODE_UP:
            snake_contex_redir(&payload->snake_ctx, SNAKE_DIR_UP);
            break;
          case SDL_SCANCODE_LEFT:
            snake_contex_redir(&payload->snake_ctx, SNAKE_DIR_LEFT);
            break;
          case SDL_SCANCODE_DOWN:
            snake_contex_redir(&payload->snake_ctx, SNAKE_DIR_DOWN);
            break;
          default:
            break;
        }
        break;
    }
  }
  /* Start drawing scene. */
  r.w = r.h = SNAKE_BLOCK_SIZE_IN_PIXELS;
  SDL_SetRenderDrawColor(payload->renderer, 0, 0, 0, 255);
  SDL_RenderClear(payload->renderer);
  /* Draw food. */
  r.x = payload->snake_ctx.food.x * SNAKE_BLOCK_SIZE_IN_PIXELS;
  r.y = payload->snake_ctx.food.y * SNAKE_BLOCK_SIZE_IN_PIXELS;
  SDL_SetRenderDrawColor(payload->renderer, 0, 0, 128, 255);
  SDL_RenderFillRect(payload->renderer, &r);
  /* Draw snake's head. */
  r.x = payload->snake_ctx.body.pos[0][X_] * SNAKE_BLOCK_SIZE_IN_PIXELS;
  r.y = payload->snake_ctx.body.pos[0][Y_] * SNAKE_BLOCK_SIZE_IN_PIXELS;
  SDL_SetRenderDrawColor(payload->renderer, 255, 255, 0, 255);
  SDL_RenderFillRect(payload->renderer, &r);
  /* Draw snake's body. */
  SDL_SetRenderDrawColor(payload->renderer, 0, 128, 0, 255);
  for (i = 1; i < payload->snake_ctx.body.length; i++) {
    r.x = payload->snake_ctx.body.pos[i][X_] * SNAKE_BLOCK_SIZE_IN_PIXELS;
    r.y = payload->snake_ctx.body.pos[i][Y_] * SNAKE_BLOCK_SIZE_IN_PIXELS;
    SDL_RenderFillRect(payload->renderer, &r);
  }
  /* Present everything. */
  SDL_RenderPresent(payload->renderer);
  return 1;
}

#ifdef __EMSCRIPTEN__
void emscripten_main_loop(void* payload) {
  sdl_main_loop((struct MainLoopPayload*)payload);
}
#endif /* __EMSCRIPTEN__ */
