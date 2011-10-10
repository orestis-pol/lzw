#ifndef BYTE_IO
#define BYTE_IO

#include <stdio.h>


typedef struct byte_buf byte_buf;

byte_buf *byte_init(FILE* fp);

int byte_read(byte_buf *byb);

int byte_write(char *seq, int size, byte_buf *byb);

int byte_flush(byte_buf *byb);


#endif
