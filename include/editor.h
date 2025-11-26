#ifndef EDITOR_H
#define EDITOR_H

#include "gap_buffer.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  QUIT = 0,
  RUNNING,
} editor_state;

typedef struct {
  editor_state state;

  gap_buffer_t *buffer;

  int cursor_line;
  int cursor_col;

  int scroll_y; // vertical scroll offset (line index)
  int scroll_x; // horizontal scroll offset (column index)

  bool cursor_visible;
  uint32_t last_blink; // blinking caret
} editor_t;

editor_t *editor_create(size_t inital_capacity);
void editor_destory(editor_t *editor);

void editor_insert_char(editor_t *editor, const char c);
void editor_cursor_recompute_ticks(editor_t *editor);
void editor_backspace(editor_t *editor);
void editor_move_left(editor_t *editor);
void editor_move_right(editor_t *editor);
void editor_move_up(editor_t *editor);
void editor_move_down(editor_t *editor);
int editor_get_line_length(editor_t *editor, int line_number);

#endif // !EDITOR_H
