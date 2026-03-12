#ifndef _BUFFER_H
#define _BUFFER_H

#include<stddef.h>

#include"constants.h"

typedef struct buffer {
  size_t size;
  char data[MAX_MSG_LEN];
} buffer_t;

buffer_t	*create_buffer();
void		free_buffer(buffer_t *buf);
void		init_buffer(buffer_t *buf);

/* returns the number of dumped bytes */
int		append_b(buffer_t *buf, char *str, size_t len);
char		*pop_b(buffer_t *buf); // reads a full line from the buffer and returns it
void		debug_buffer(buffer_t *buf);

/* returns the index of the first '\n', -1 otherwise */
int		is_ready(buffer_t *buf);

#endif
