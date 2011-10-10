#ifndef BIT_IO
#define BIT_IO

#include <stdio.h>


/*
 * Use the bit_buf pointer (bit_buf*)
 * exactly as FILE pointer (FILE*) of
 * the standard C I/O library (stdio).
 * You must not use the same bit buffer
 * for both input and output, even if
 * the file stream is opened for both.
 * Also that the buffer is allocated
 * as dynamic memory so you should
 * deallocate in case you have no
 * further need of it. Also note that
 * once a buffer returns an error code,
 * you must NOT use the same buffer
 * again or the behaviour will be
 * undefined.
 */
typedef struct bit_buf bit_buf;

/*
 * Allocate and return a bit buffer to
 * write or read from file stream "fp".
 * Returns NULL if it's out of memory.
 * You should free() the returned
 * pointer at end of its usage but note
 * that the file stream will be neither
 * flushed nor closed.
 */
bit_buf *bit_init(FILE *fp);

/*
 * Write the number "c" using "b" bits to
 * the output file stream using bit buffer
 * "u". If no error occurs 0 is returned,
 * else -1 is returned and you must not
 * use the same buffer again afterwards.
 */
int bit_write(unsigned int c, unsigned int b, bit_buf *u);

/*
 * Flush the bit buffer "u" to write to the
 * output file whatever data we already
 * gave the bit buffer through "bit_write".
 * Note that if number of bits submitted do
 * not complete the last byte, it is padded
 * with zeros and is written to output file.
 * Note also, that, this is not the same
 * flushing as with fflush() function from
 * C standard I/O library (stdio). You must
 * always call this function at end of bit
 * writing process or else you most probably
 * will leave unwritten bits. Also the
 * fflush() function will not be called.
 * If no error occurs returns 0, else -1 is
 * returned and as mentioned for bit_write(),
 * you must not use the buffer again.
 */
int bit_flush(bit_buf *u);

/*
 * Read the next "b" bits from the input file
 * stream using bit buffer "u" and return them
 * as an integer. If there are no more "b" bits,
 * -2 is returned. In case of error, returns -1
 * and the same buffer must not be used again.
 * If "b" is more than 32 then behaviour is
 * undefined and also note that "b" should not
 * be equal to 32, or else you will not be able
 * to distinguish returned values between
 * -1 or -2 codes and valid 32-bit data.
 */
int bit_read(unsigned int b, bit_buf *u);


#endif
