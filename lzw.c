#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bit_io.h"
#include "byte_io.h"

/* Not ANSI C code - only for Windows compatibility */
#if defined WIN32 || defined WIN64
#include <fcntl.h>
#include <io.h>
#endif
/* End */


int number_arg(char *arg)
{
    int i;
    for (i = 0 ; arg[i] ; ++i)
        if (arg[i] < '0' || arg[i] > '9')
            return 0;
    return 1;
}

void reverse(unsigned char *s, unsigned int l)
{
    unsigned char t, *r = &s[l - 1], *e = &s[l >> 1];
    while (s != e) {
        t = *s;
        *s++ = *r;
        *r-- = t;
    }
}

int main(int argc, char **argv)
{
    enum { OK = 0, ENC_ERROR, MEM_ERROR, IN_ERROR, OUT_ERROR, ARG_ERROR };
    bit_buf *bib = NULL;
    byte_buf *byb = NULL;
    FILE *ifp = stdin, *ofp = stdout;
    int byte, pos, ppos, npos, arg;
    int b = 0, d = 0, i = 0, o = 0, p = 0, v = 0, n = 0, e = OK;
    unsigned long long oc = 3ull, ic = 0ull, osc = 0ull, isc = 1ull;
    unsigned int *temp, *pool = NULL, pool_kb = 256, pool_ukb = 1;
    unsigned int bits = 9, mbits = 16, dir_s = 257, seq_s = 1;
    unsigned char *seq = NULL;
    for (arg = 1 ; arg != argc ; ++arg) {
        if (!strcmp(argv[arg], "-h")) {
            fprintf(stderr, "\
Usage: %s [OPTION] ...\n\n\
Compress or decompress files using LZW algorithm\n\n\
  -h, type this help message\n\
  -d, decompress (else execute compress)\n\
  -b, set max bits for dictionary codes (for compress)\n\
  -p, type compression ratio at standard error (for compress)\n\
  -n, disable clear code (for compress)\n\
  -v, type code bit resizes at standard error (for compress)\n\
  -i file, give input file (else reads from standard input)\n\
  -o file, give output file (else writes at standard output)\n\n", argv[0]);
            goto no_error;
        }
        if (!strcmp(argv[arg], "-i")) {
            if (i++)
                fprintf(stderr, "%s: Already specified -i\n", argv[0]);
            else if (++arg == argc)
                fprintf(stderr, "%s: Missing file after -i\n", argv[0]);
            else if ((ifp = fopen(argv[arg], "rb")) == NULL)
                fprintf(stderr, "%s: %s: Cannot open input file\n",
                                argv[0], argv[arg]);
            else continue;
            goto arg_error;
        }
        if (!strcmp(argv[arg], "-o")) {
            if (o++)
                fprintf(stderr, "%s: Already specified -o\n", argv[0]);
            else if (++arg == argc)
                fprintf(stderr, "%s: Missing file after -o\n", argv[0]);
            else if ((ofp = fopen(argv[arg], "wb")) == NULL)
                fprintf(stderr, "%s: %s: Cannot open output file\n",
                                argv[0], argv[arg]);
            else continue;
            goto arg_error;
        }
        if (!strcmp(argv[arg], "-b")) {
            if (b++)
                fprintf(stderr, "%s: Already specified -b\n", argv[0]);
            else if (d)
                fprintf(stderr, "%s: Cannot use -b with -d\n", argv[0]);
            else if (++arg == argc)
                fprintf(stderr, "%s: Missing number after -b\n", argv[0]);
            else if (!number_arg(argv[arg]))
                fprintf(stderr, "%s: Not a number after -b\n", argv[0]);
            else if ((mbits = (unsigned int)atoi(argv[arg])) < 9 || mbits > 16)
                fprintf(stderr, "%s: %d: Invalid max bits\n", argv[0], mbits);
            else continue;
            goto arg_error;
        }
        if (!strcmp(argv[arg], "-d")) {
            if (d++)
                fprintf(stderr, "%s: Already specified -d\n", argv[0]);
            else if (b)
                fprintf(stderr, "%s: Cannot use -d with -b\n", argv[0]);
            else if (p)
                fprintf(stderr, "%s: Cannot use -d with -p\n", argv[0]);
            else if (n)
                fprintf(stderr, "%s: Cannot use -d with -n\n", argv[0]);
            else if (v)
                fprintf(stderr, "%s: Cannot use -d with -v\n", argv[0]);
            else continue;
            goto arg_error;
        }
        if (!strcmp(argv[arg], "-p")) {
            if (p++)
                fprintf(stderr, "%s: Already specified -p\n", argv[0]);
            else if (d)
                fprintf(stderr, "%s: Cannot use -p with -d\n", argv[0]);
            else continue;
            goto arg_error;
        }
        if (!strcmp(argv[arg], "-n")) {
            if (n++)
                fprintf(stderr, "%s: Already specified -n\n", argv[0]);
            else if (d)
                fprintf(stderr, "%s: Cannot use -n with -d\n", argv[0]);
            else continue;
            goto arg_error;
        }
        if (!strcmp(argv[arg], "-v")) {
            if (v++)
                fprintf(stderr, "%s: Already specified -v\n", argv[0]);
            else if (d)
                fprintf(stderr, "%s: Cannot use -v with -d\n", argv[0]);
            else continue;
            goto arg_error;
        }
        fprintf(stderr, "%s: Invalid arguments\n", argv[0]);
        fprintf(stderr, "For help, type %s -h\n", argv[0]);
        goto arg_error;
    }
/* Not ANSI C code - only for Windows compatibility */
#if defined WIN32 || defined WIN64
    if (!i && _setmode(_fileno(stdin), _O_BINARY) == -1)
        goto in_error;
    if (!o && _setmode(_fileno(stdout), _O_BINARY) == -1)
        goto out_error;
#endif
/* End */
    if (!d) {
        if ((byb = byte_init(ifp)) == NULL)
            goto mem_error;
        if ((byte = byte_read(byb)) < 0) {
            if (byte == -1)
                goto in_error;
            goto no_error;
        }
        bib = bit_init(ofp);
        pool = malloc(pool_kb << 10);
        if (bib == NULL || pool == NULL)
            goto mem_error;
        if (bit_write(mbits - 9, 3, bib) == -1)
            goto out_error;
        for (pos = 0 ; pos != 256 ; ++pos)
            pool[pos] = pos;
        pos = byte;
        while ((byte = byte_read(byb)) >= 0) {
            isc++;
            npos = (pool[pos] >> 16) << 8;
            if (npos && pool[npos + byte]) {
                pos = npos + byte;
                continue;
            }
            if (bit_write((pool[pos] & 0xFFFF), bits, bib) == -1)
                goto out_error;
            osc += bits;
            if (dir_s == (1u << bits)) {
                if (bits != mbits) {
                    bits++;
                    if (v)
                        fprintf(stderr, "UP (in: %llu bytes, out: %llu bits)\n",
                                         ic + isc, oc + osc);
                }
                if (!n && osc > (isc << 3)) {
                   for (pos = 0 ; pos != 256 ; ++pos)
                        pool[pos] = pos;
                    if (bit_write(256, bits, bib) == -1)
                        goto out_error;
                    ic += isc;
                    if (v)
                        fprintf(stderr, "CL (in: %llu bytes, out: %llu bits)\n",
                                         ic, oc + osc);
                    oc += osc + bits;
                    isc = osc = 0ull;
                    pool_ukb = 1;
                    dir_s = 257;
                    bits = 9;
                }
            }
            if (dir_s != (1u << mbits) && isc) {
                if (npos)
                    pool[npos+byte] = dir_s++;
                else {
                    if (pool_ukb == pool_kb) {
                        pool_kb <<= 1;
                        temp = realloc(pool, pool_kb << 10);
                        if (temp == NULL)
                            goto mem_error;
                        pool = temp;
                    }
                    memset(&pool[pool_ukb << 8], 0, 1024);
                    pool[pos] |= (pool_ukb << 16);
                    pool[((pool_ukb++) << 8) + byte] = dir_s++;
                }
            }
            pos = byte;
        }
        if (byte == -1)
            goto in_error;
        if (bit_write(pool[pos] & 0xFFFF, bits, bib) == -1)
            goto out_error;
        if (bit_flush(bib) == -1)
            goto out_error;
        oc = (oc + osc + bits + 7ull) >> 3;
        ic += isc;
    } else {
        if ((bib = bit_init(ifp)) == NULL)
            goto mem_error;
        if ((mbits = (unsigned int)(bit_read(3, bib) + 9)) < 9) {
            if (mbits == 8)
                goto in_error;
            goto no_error;
        }
        byb = byte_init(ofp);
        seq = malloc(1 << mbits);
        pool = malloc(1 << (mbits + 2));
        if (byb == NULL || seq == NULL || pool == NULL)
            goto mem_error;
        for (pos = 0 ; pos != 256 ; ++pos)
            pool[pos] = pos;
        do {
            ppos = bit_read(bits, bib);
        } while (ppos == 256);
        if (ppos == -1)
            goto in_error;
        if (ppos == -2)
            goto enc_error;
        seq[0] = ppos;
        if (byte_write((char*)seq, 1, byb) == -1)
            goto out_error;
        while ((npos = bit_read(bits, bib)) >= 0) {
            if ((unsigned int)npos > dir_s)
                goto enc_error;
            if (npos == 256) {
                bits = 9;
                do {
                    ppos = bit_read(bits, bib);
                } while (ppos == 256);
                if (ppos == -1)
                    goto in_error;
                if (ppos == -2) break;
                seq[0] = ppos;
                if (byte_write((char*)seq, 1, byb) == -1)
                    goto out_error;
                dir_s = 257;
                seq_s = 1;
                continue;
            }
            if ((unsigned int)npos == dir_s)
                seq[seq_s++] = seq[0];
            else {
                pos = npos;
                seq[0] = pool[npos];
                seq_s = 1;
                while (pos > 256) {
                    pos = pool[pos] >> 8;
                    seq[seq_s++] = pool[pos];
                }
                reverse(seq, seq_s);
            }
            if (byte_write((char*)seq, seq_s, byb) == -1)
                goto out_error;
            if (dir_s != (unsigned int)(1 << mbits)) {
                pool[dir_s++] = (ppos << 8) | seq[0];
                if (bits != mbits && dir_s == (1u << bits))
                    bits++;
            }
            ppos = npos;
        }
        if (npos == -1)
            goto in_error;
        if (byte_flush(byb) == -1)
            goto out_error;
    }
no_error:
    if (fclose(ifp) == EOF && e == OK)
        e = IN_ERROR;
    if (fflush(ofp) == EOF && e == OK)
        e = OUT_ERROR;
    if (fclose(ofp) == EOF && e == OK)
        e = OUT_ERROR;
    free(seq);
    free(pool);
    free(byb);
    free(bib);
    if (e == ENC_ERROR)
        fprintf(stderr, "%s: Invalid input encoding\n", argv[0]);
    else if (e == MEM_ERROR)
        fprintf(stderr, "%s: Not enough memory\n", argv[0]);
    else if (e == IN_ERROR)
        fprintf(stderr, "%s: Error while reading input file\n", argv[0]);
    else if (e == OUT_ERROR)
        fprintf(stderr, "%s: Error while writing output file\n", argv[0]);
    else if (e == OK && p) {
        if (oc <= ic)
            fprintf(stderr, "Achieved compression of %.2f%%\n",
                             100.0*(1.0-(double)oc/(double)ic));
        else
            fprintf(stderr, "Used additional %.2f%%\n",
                             100.0*((double)oc/(double)ic-1.0));
    }
    return e;
enc_error:
    e = ENC_ERROR;
    goto no_error;
mem_error:
    e = MEM_ERROR;
    goto no_error;
in_error:
    e = IN_ERROR;
    goto no_error;
out_error:
    e = OUT_ERROR;
    goto no_error;
arg_error:
    e = ARG_ERROR;
    goto no_error;
}
