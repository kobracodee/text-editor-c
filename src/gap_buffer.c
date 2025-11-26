#include "../include/gap_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

gap_buffer_t *gap_create(size_t initial_capacity) {
  gap_buffer_t *g = malloc(sizeof(gap_buffer_t));
  if (NULL == g) {
    fprintf(stderr, "Could not initalize gap buffer.\n");
    return NULL;
  }
  g->buffer = malloc(initial_capacity);
  if (NULL == g->buffer) {
    fprintf(stderr, "Could not initalize gap-buffer buffer.\n");
    return NULL;
  }
  g->gap_start = 0;
  g->gap_end = initial_capacity;
  g->capacity = initial_capacity;

  return g;
}

void gap_destroy(gap_buffer_t *g) {
  if (!g)
    return;
  free(g->buffer);
  free(g);
}

char gap_peek_before(gap_buffer_t *g) {
  if (g->gap_start == 0)
    return 0;
  return g->buffer[g->gap_start - 1];
}

char gap_peek_after(gap_buffer_t *g) {
  if (g->gap_end >= g->capacity)
    return 0;
  return g->buffer[g->gap_end];
}

void gap_expand(gap_buffer_t *g) {
  size_t old_cap = g->capacity;
  size_t new_cap = old_cap * 2;

  char *new_buff = malloc(new_cap);

  size_t before = g->gap_start;
  size_t after = old_cap - g->gap_end;
  size_t new_gap_end = new_cap - after;

  // Copy text before gap
  memcpy(new_buff, g->buffer, before);

  // Copy text after gap
  memcpy(new_buff + new_gap_end, g->buffer + g->gap_end, after);

  free(g->buffer);
  g->buffer = new_buff;
  g->capacity = new_cap;
  g->gap_end = new_gap_end;
}

void gap_insert_char(gap_buffer_t *gap, const char c) {
  if (gap->gap_start == gap->gap_end) {
    gap_expand(gap);
  }
  gap->buffer[gap->gap_start++] = c;
}

void gap_delete_char(gap_buffer_t *g) {
  if (g->gap_start > 0)
    g->gap_start--;
}

void gap_move_left(gap_buffer_t *g) {
  if (g->gap_start > 0) {
    g->buffer[--g->gap_end] = g->buffer[--g->gap_start];
  }
}

void gap_move_right(gap_buffer_t *g) {
  if (g->gap_end < g->capacity) {
    g->buffer[g->gap_start++] = g->buffer[g->gap_end++];
  }
}

void gap_print(const gap_buffer_t *g) {
  printf("Buffer (capacity=%zu):\n", g->capacity);
  printf("[");

  for (size_t i = 0; i < g->capacity; i++) {
    if (i == g->gap_start) {
      printf("<"); // mark start of gap
    }
    if (i == g->gap_end) {
      printf(">"); // mark end of gap
    }

    if (i >= g->gap_start && i < g->gap_end) {
      printf("_"); // gap contents
    } else {
      char c = g->buffer[i];
      if (c >= 32 && c <= 126)
        printf("%c", c); // printable char
      else
        printf("."); // placeholder for non-printables
    }
  }

  // close gap marker at end if needed
  if (g->gap_end == g->capacity) {
    printf(">");
  }

  printf("]\n");
}

int gap_buffer_length(const gap_buffer_t *g) {
  return g->gap_start + (g->capacity - g->gap_end);
}

void gap_to_string(const gap_buffer_t *g, char *out) {
  int left_len = g->gap_start;
  int right_len = g->capacity - g->gap_end;

  memcpy(out, g->buffer, left_len);
  memcpy(out + left_len, g->buffer + g->gap_end, right_len);

  out[left_len + right_len] = '\0';
}
