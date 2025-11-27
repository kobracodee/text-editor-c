#include "../include/editor.h"
#include "../include/gap_buffer.h"
#include "SDL_timer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

editor_t *editor_create(size_t inital_capacity) {
  editor_t *e = malloc(sizeof(editor_t));
  if (!e)
    return NULL;

  e->state = RUNNING;

  e->buffer = gap_create(inital_capacity);

  e->cursor_line = 0;
  e->cursor_col = 0;

  e->scroll_x = 0;
  e->scroll_y = 0;

  e->cursor_visible = true;
  e->last_blink = SDL_GetTicks();

  return e;
}

void editor_destory(editor_t *editor) {
  if (!editor)
    return;
  gap_destroy(editor->buffer);
  free(editor);
}

void editor_insert_char(editor_t *editor, const char c) {
  editor_cursor_recompute_ticks(editor);

  gap_insert_char(editor->buffer, c);

  if (c == '\n') {
    editor->cursor_line++;
    editor->cursor_col = 0;
  } else {
    editor->cursor_col++;
  }
}

void editor_cursor_recompute_ticks(editor_t *editor) {
  if (!editor->cursor_visible) {
    editor->cursor_visible = !editor->cursor_visible;
    editor->last_blink = SDL_GetTicks();
  }
}

void editor_backspace(editor_t *editor) {
  editor_cursor_recompute_ticks(editor);
  char deleted = gap_peek_before(editor->buffer);

  if (deleted == 0)
    return;

  gap_delete_char(editor->buffer);

  if (deleted == '\n') {
    editor->cursor_line--;
    // Move cursor to end of previous line
    editor->cursor_col = editor_get_line_length(editor, editor->cursor_line);
  } else {
    if (editor->cursor_col > 0) {
      editor->cursor_col--;
    }
  }
}

int editor_get_line_length(editor_t *editor, int line_number) {
  char *s = malloc(editor->buffer->capacity);
  gap_to_string(editor->buffer, s);

  int current = 0;
  int len = 0;

  for (char *p = s; *p; p++) {
    if (current == line_number) {
      if (*p == '\n') {
        break;
      }
      len++;
    }
    if (*p == '\n')
      current++;
  }

  free(s);
  return len;
}

void editor_move_left(editor_t *editor) {
  editor_cursor_recompute_ticks(editor);
  char c = gap_peek_before(editor->buffer);
  if (c == 0)
    return;

  gap_move_left(editor->buffer);

  if (c == '\n') {
    editor->cursor_line--;
    editor->cursor_col = editor_get_line_length(editor, editor->cursor_line);
  } else {
    editor->cursor_col--;
  }
}
void editor_move_right(editor_t *editor) {
  editor_cursor_recompute_ticks(editor);
  char c = gap_peek_after(editor->buffer);
  if (c == 0)
    return;

  gap_move_right(editor->buffer);

  if (c == '\n') {
    editor->cursor_line++;
    editor->cursor_col = 0;
  } else {
    editor->cursor_col++;
  }
}

int editor_count_lines(editor_t *e) {
  char *s = malloc(e->buffer->capacity);
  gap_to_string(e->buffer, s);

  int lines = 1;
  for (char *p = s; *p; p++) {
    if (*p == '\n')
      lines++;
  }
  free(s);
  return lines;
}

void editor_move_up(editor_t *editor) {
  if (editor->cursor_line == 0)
    return;
  editor_cursor_recompute_ticks(editor);

  int target_col = editor->cursor_col;

  // Move left until hit \n
  while (gap_peek_before(editor->buffer) != '\n') {
    editor_move_left(editor);
    if (editor->buffer->gap_start == 0)
      break;
  }

  // skip the new line it self
  editor_move_left(editor);

  // move right until target col or newline
  int line_len = editor_get_line_length(editor, editor->cursor_line);
  int col = (target_col > line_len ? line_len : target_col);

  for (int i = 0; i < col; i++) {
    if (gap_peek_after(editor->buffer) == '\n')
      break;
    editor_move_right(editor);
  }
}
void editor_move_down(editor_t *editor) {
  int total_lines = editor_count_lines(editor);
  if (editor->cursor_line >= total_lines - 1)
    return;
  editor_cursor_recompute_ticks(editor);

  int target_col = editor->cursor_col;

  // move right until end of this line
  while (gap_peek_after(editor->buffer) != '\n') {
    editor_move_right(editor);
  }

  // skip newline
  editor_move_right(editor);

  int line_len = editor_get_line_length(editor, editor->cursor_line);
  int col = (target_col > line_len ? line_len : target_col);

  for (int i = 0; i < col; i++) {
    if (gap_peek_after(editor->buffer) == '\n')
      break;
    editor_move_right(editor);
  }
}
