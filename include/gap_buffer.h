#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#include <stddef.h>

typedef struct {
  char *buffer;
  size_t gap_start;
  size_t gap_end;
  size_t capacity;
} gap_buffer_t;

gap_buffer_t *gap_create(size_t initial_capacity);
void gap_destroy(gap_buffer_t *g);

void gap_expand(gap_buffer_t *g);
char gap_peek_before(gap_buffer_t *g);
char gap_peek_after(gap_buffer_t *g);
void gap_insert_char(gap_buffer_t *g, const char c);
void gap_delete_char(gap_buffer_t *g);
void gap_move_left(gap_buffer_t *g);
void gap_move_right(gap_buffer_t *g);
void gap_print(const gap_buffer_t *g);
void gap_to_string(const gap_buffer_t *g, char *out);
int gap_buffer_length(const gap_buffer_t *g);

#endif // !GAP_BUFFER_H
