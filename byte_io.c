#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct byte_buf {
    FILE *fp;
    int pos, num;
    char buf[4096];
} byte_buf;

byte_buf *byte_init(FILE *fp)
{
    byte_buf *byb = malloc(sizeof(byte_buf));
    if (byb == NULL) return NULL;
    byb->fp = fp;
    byb->pos = byb->num = 0;
    return byb;
}

int byte_read(byte_buf *byb)
{
    if (byb->pos != byb->num)
        return (int)((unsigned char)byb->buf[byb->pos++]);
    if (!(byb->num = fread(byb->buf, 1, 4096, byb->fp)))
        return (feof(byb->fp) ? -2 : -1);
    byb->pos = 1;
    return (int)((unsigned char)byb->buf[0]);
}

int byte_write(char *seq, int size, byte_buf *byb)
{
    while (size + byb->pos > 4096) {
        memcpy(&(byb->buf[byb->pos]), seq, 4096 - byb->pos);
        if (fwrite(byb->buf, 1, 4096, byb->fp) != 4096)
            return -1;
        seq = &seq[4096 - byb->pos];
        size -= 4096 - byb->pos;
        byb->pos = 0;
    }
    memcpy(&byb->buf[byb->pos], seq, size);
    byb->pos += size;
    return 0;
}

int byte_flush(byte_buf *byb)
{
    if (byb->pos) {
        if (fwrite(byb->buf, 1, byb->pos, byb->fp) != (size_t)byb->pos)
            return -1;
        byb->pos = 0;
    }
    return 0;
}
