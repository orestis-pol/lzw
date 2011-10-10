#include <stdio.h>
#include <stdlib.h>


typedef struct bit_buf {
    FILE *fp;
    unsigned int data, bits;
    int pos, num;
    unsigned char buf[4096];
} bit_buf;

bit_buf *bit_init(FILE *fp)
{
    bit_buf *bib = malloc(sizeof(bit_buf));
    if (bib == NULL) return NULL;
    bib->fp = fp;
    bib->data = bib->bits = 0;
    bib->pos = bib->num = 0;
    return bib;
}

int bit_write(unsigned int data, unsigned int bits, bit_buf *bib)
{
    bib->bits += bits;
    if (bib->bits > 32) {
        bib->bits -= 32;
        bib->data |= data >> bib->bits;
        bib->buf[bib->pos]   = bib->data >> 24;
        bib->buf[bib->pos+1] = bib->data >> 16;
        bib->buf[bib->pos+2] = bib->data >> 8;
        bib->buf[bib->pos+3] = bib->data;
        bib->pos += 4;
        if (bib->pos == 4096) {
            if (fwrite(bib->buf, 1, 4096, bib->fp) != 4096)
                return -1;
            bib->pos = 0;
        }
        bib->data = 0;
    }
    bib->data |= data << (32 - bib->bits);
    return 0;
}

int bit_flush(bit_buf *bib)
{
    if (bib->bits)
        bib->buf[bib->pos++] = bib->data >> 24;
    if (bib->bits > 8)
        bib->buf[bib->pos++] = bib->data >> 16;
    if (bib->bits > 16)
        bib->buf[bib->pos++] = bib->data >> 8;
    if (bib->bits > 24)
        bib->buf[bib->pos++] = bib->data;
    if (fwrite(bib->buf, 1, bib->pos, bib->fp) != (size_t)bib->pos)
        return -1;
    bib->bits = 0;
    bib->pos = 0;
    return 0;
}

int bit_read(unsigned int bits, bit_buf *bib)
{
    unsigned int ret = 0;
    while (bits > bib->bits) {
        bits -= bib->bits;
        ret |= (int)((bib->data & ((1 << bib->bits) - 1)) << bits);
        if (bib->pos == bib->num) {
            if (!(bib->num = fread(bib->buf, 1, 4096, bib->fp)))
                return (feof(bib->fp) ? -2 : -1);
            bib->pos = 0;
        }
        if (bib->pos+4 <= bib->num) {
            bib->data = (((unsigned int)bib->buf[bib->pos])   << 24) |
                        (((unsigned int)bib->buf[bib->pos+1]) << 16) |
                        (((unsigned int)bib->buf[bib->pos+2]) << 8)  |
                         ((unsigned int)bib->buf[bib->pos+3]);
            bib->pos += 4;
            bib->bits = 32;
        } else {
            bib->data = (unsigned int)bib->buf[bib->pos++];
            bib->bits = 8;
        }
    }
    bib->bits -= bits;
    ret |= ((bib->data >> bib->bits) & ((1 << bits) - 1));
    return (int)ret;
}
