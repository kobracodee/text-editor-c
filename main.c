#include "SDL_keyboard.h"
#include "SDL_rect.h"
#include "SDL_timer.h"
#include <stdint.h>
#define SDL_MAIN_HANDLED
#include "SDL_surface.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/editor.h"
#include "include/gap_buffer.h"

#define FONT "JetBrainsMono-Regular.ttf"
#define TAB_WIDTH 4
#define LINE_NUMBER_WIDTH 50

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  int window_width;
  int window_height;
  struct {
    TTF_Font *font;
    size_t font_size;
  } Font;
} sdl_t;

typedef struct {
  char **lines;
  int count;
} lines_t;

lines_t split_lines(const char *text) {
  lines_t L = {0};

  if (!text)
    return L;

  // Count line
  int count = 1;
  for (const char *p = text; *p; p++) {
    if (*p == '\n')
      count++;
  }

  L.count = count;
  L.lines = malloc(sizeof(char *) * count);

  int idx = 0;
  const char *start = text;

  for (const char *p = text;; p++) {
    if (*p == '\n' || *p == '\0') {
      int len = p - start;
      char *line = malloc(len + 1);
      memcpy(line, start, len);
      line[len] = '\0';
      L.lines[idx++] = line;

      if (*p == '\0')
        break;

      start = p + 1;
    }
  }

  return L;
}

bool init_sdl(sdl_t *sdl) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    SDL_Log("Could not initalize SDL! %s\n", SDL_GetError());
    return false;
  }

  sdl->window =
      SDL_CreateWindow("Text Editor", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1200, 600, SDL_WINDOW_RESIZABLE);

  sdl->window_width = 1200;
  sdl->window_height = 600;

  SDL_SetWindowResizable(sdl->window, SDL_TRUE);
  // sdl->window = SDL_CreateWindow("Text Editor",
  //                            SDL_WINDOWPOS_UNDEFINED,
  //                            SDL_WINDOWPOS_UNDEFINED, 0, 0,
  //                            SDL_WINDOW_FULLSCREEN_DESKTOP);

  if (!sdl->window) {
    SDL_Log("Could not create a Window! %s\n", SDL_GetError());
    return false;
  }

  sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);

  if (!sdl->renderer) {
    SDL_Log("Could not create a Renderer! %s\n", SDL_GetError());
    return false;
  }

  if (TTF_Init() < 0) {
    SDL_Log("Could not initalize TTF! %s\n", TTF_GetError());
    return false;
  }

  sdl->Font.font_size = 18;

  sdl->Font.font =
      TTF_OpenFont("JetBrainsMono-Regular.ttf", sdl->Font.font_size);
  if (!sdl->Font.font) {
    SDL_Log("TTF_OpenFont: %s\n", TTF_GetError());
    return false;
  }

  SDL_StartTextInput();

  return true;
}

void final_cleanup(sdl_t *sdl, editor_t *e) {
  editor_destory(e);

  SDL_DestroyRenderer(sdl->renderer);
  SDL_DestroyWindow(sdl->window);
  SDL_Quit();
}

void handle_input(editor_t *editor, sdl_t *sdl) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_TEXTINPUT:

      editor_insert_char(editor, event.text.text[0]);
      return;
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
          event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        int width = event.window.data1;
        int height = event.window.data2;

        printf("width: %d\n", width);
        printf("height: %d\n", height);

        sdl->window_width = width;
        sdl->window_height = height;

        SDL_SetWindowSize(sdl->window, width, height);
        SDL_RenderSetViewport(sdl->renderer, NULL);
      }
      return;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
        editor_insert_char(editor, '\n');
        break;
      case SDLK_LEFT:
        editor_move_left(editor);
        break;
      case SDLK_RIGHT:
        editor_move_right(editor);
        break;
      case SDLK_UP:
        editor_move_up(editor);
        break;
      case SDLK_DOWN:
        editor_move_down(editor);
        break;

      case SDLK_BACKSPACE:
        editor_backspace(editor);
        break;
      case SDLK_TAB:
        for (int i = 0; i < TAB_WIDTH; i++) {
          editor_insert_char(editor, ' ');
        }
        break;
      case SDLK_ESCAPE:
        editor->state = QUIT;
        return;
      }

      return;
    }
  }
}

#define BLACK_COLOR 0x000000FF
#define WHITE_COLOR 0xFFFFFFFF

// Clear screen / SDL Window to background color
void clear_screen(const sdl_t sdl) {
  const uint8_t r = (BLACK_COLOR >> 24) & 0xFF;
  const uint8_t g = (BLACK_COLOR >> 16) & 0xFF;
  const uint8_t b = (BLACK_COLOR >> 8) & 0xFF;
  const uint8_t a = (BLACK_COLOR >> 0) & 0xFF;

  SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
  SDL_RenderClear(sdl.renderer);
}

SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font,
                         const char *msg, SDL_Color color, SDL_Rect *out_rect) {
  SDL_Surface *surface = TTF_RenderText_Blended(font, msg, color);
  if (!surface) {
    SDL_Log("TTF_RenderText_Blended error: %s", TTF_GetError());
    return NULL;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    SDL_Log("SDL_CreateTextureFromSurface error: %s", SDL_GetError());
    SDL_FreeSurface(surface);
    return NULL;
  }

  out_rect->w = surface->w;
  out_rect->h = surface->h;
  SDL_FreeSurface(surface);

  return texture;
}

int digit_count(int number) {
  int count = 0;
  while (number > 0) {
    number /= 10;
    count++;
  }

  return count;
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  editor_t *editor = editor_create(1024);

  sdl_t sdl = {0};
  if (!init_sdl(&sdl))
    exit(EXIT_FAILURE);

  clear_screen(sdl);

  int char_w = 0, char_h = 0;
  TTF_SizeText(sdl.Font.font, "A", &char_w, &char_h);

  while (editor->state != QUIT) {
    handle_input(editor, &sdl);
    clear_screen(sdl);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color light_gray = {180, 180, 180, 255};

    // --- draw line-number gutter background ---
    SDL_SetRenderDrawColor(sdl.renderer, 40, 40, 40, 255); // dark grey
    SDL_Rect gutter = {0, 0, LINE_NUMBER_WIDTH, sdl.window_height};
    SDL_RenderFillRect(sdl.renderer, &gutter);

    char *msg = malloc(editor->buffer->capacity);
    gap_to_string(editor->buffer, msg);

    uint32_t now = SDL_GetTicks();
    if (now - editor->last_blink > 500) {
      editor->cursor_visible = !editor->cursor_visible;
      editor->last_blink = now;
    }

    lines_t L = split_lines(msg);

    if (L.count == 0) {
      L.count = 1;
      L.lines = malloc(sizeof(char *));
      L.lines[0] = "";
    }

    int y = 20;

    for (int i = 0; i < L.count; i++) {
      const char *line = L.lines[i];
      if (line[0] == '\0') {
        line = " ";
      }

      // render line line_number
      char line_number_str[16];
      snprintf(line_number_str, sizeof(line_number_str), "%d", i + 1);

      int digits = digit_count(i + 1);
      int number_w = digits * char_w;

      SDL_Rect ln_rect = {LINE_NUMBER_WIDTH - number_w - 5, y, number_w,
                          TTF_FontHeight(sdl.Font.font)};
      // SDL_Rect ln_rect = {LINE_NUMBER_WIDTH / digit_count(i + 1), y,
      //                     LINE_NUMBER_WIDTH / digit_count(i + 1),
      //                     TTF_FontHeight(sdl.Font.font)};

      SDL_Texture *ln_texture = render_text(
          sdl.renderer, sdl.Font.font, line_number_str, light_gray, &ln_rect);
      if (ln_texture) {
        SDL_RenderCopy(sdl.renderer, ln_texture, NULL, &ln_rect);
        SDL_DestroyTexture(ln_texture);
      }

      // render the actual text
      SDL_Rect text_rect = {LINE_NUMBER_WIDTH + 5, y, 0, 0};
      SDL_Texture *text =
          render_text(sdl.renderer, sdl.Font.font, line, white, &text_rect);

      if (text) {
        SDL_RenderCopy(sdl.renderer, text, NULL, &text_rect);
        SDL_DestroyTexture(text);
      }

      y += TTF_FontHeight(sdl.Font.font);
    }

    if (editor->cursor_visible) {
      int cursor_x = LINE_NUMBER_WIDTH + 5 + editor->cursor_col * char_w;
      int cursor_y = 20 + editor->cursor_line * char_h;

      SDL_SetRenderDrawColor(sdl.renderer, 255, 255, 255, 255);
      SDL_Rect caret = {cursor_x, cursor_y, 2, char_h};
      SDL_RenderFillRect(sdl.renderer, &caret);
    }

    SDL_RenderPresent(sdl.renderer);
  }

  gap_print(editor->buffer);

  final_cleanup(&sdl, editor);
  return 0;
}
