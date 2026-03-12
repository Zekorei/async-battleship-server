#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"buffer.h"
#include"constants.h"

buffer_t *create_buffer() {
  buffer_t *new_buf = calloc(1, sizeof(*new_buf));
  if (!new_buf) return NULL;
  return new_buf;
}

void free_buffer(buffer_t *buf) {
  free(buf);
}

void init_buffer(buffer_t *buf) {
  memset(buf, 0, sizeof(*buf));
}

int append_b(buffer_t *buf, char *str, size_t len) {
  size_t remaining_space = MAX_MSG_LEN - buf->size;
  size_t to_copy = (remaining_space >= len) ? len : remaining_space;
  fprintf(stderr, "append_b: size=%zu, len=%zu, to_copy=%zu\n", buf->size, len, to_copy);

  memcpy(buf->data + buf->size, str, to_copy);
  buf->size += to_copy;

  return (int)(len - to_copy);
}

char *pop_b(buffer_t *buf) {
  int newline_ind = is_ready(buf);
  if (newline_ind == -1) return NULL;
  int line_len = newline_ind + 1;

  char *line = malloc(line_len + 1);
  if (!line) return NULL;

  memcpy(line, buf->data, line_len);
  line[line_len] = '\0';

  size_t remaining = buf->size - line_len;
  memmove(buf->data, buf->data + line_len, remaining);

  memset(buf->data + remaining, 0, MAX_MSG_LEN - remaining);
  buf->size = remaining;

  return line;
}

void debug_buffer(buffer_t *buf) {
  printf("buffer->size: %zu\n", buf->size);
  for (size_t i = 0; i < buf->size; i++) {
    printf("%02zu: char='%c' hex=0x%02x\n", i,
    	(buf->data[i] >= 32 && buf->data[i] <= 126) ? buf->data[i] : '.',
    	(unsigned char)buf->data[i]);
  }
}

int is_ready(buffer_t *buf) {
  for (size_t i = 0; i < buf->size; i++) {
    if (buf->data[i] == '\n') {
      return (int)i;
    }
  }
  return -1;
}

