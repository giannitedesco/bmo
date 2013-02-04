/*
* This file is part of Firestorm NIDS
* Copyright (c) 2003 Gianni Tedesco
* Released under the terms of the GNU GPL version 2
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/poll.h>

#include <compiler.h>
#include <os.h>
#include <fibuf.h>

/** fibuf - File input buffer object */
struct _fibuf {
	/** File descriptor */
	int fd;
	/** EOF flag. Set to 1 if a read exposed an EOF */
	int eof;
	/** Number of valid bytes left in the buffer */
	size_t valid;

	/** @internal Pointer to base of buffer */
	uint8_t *buf;
	/** @internal Current read position in buffer */
	uint8_t *read_pos;
	/** @internal Total size of buffer */
	size_t buf_sz;
};

/**
 * @brief Attach a file descriptor to an input buffer
 * @param b: a fibuf structure to use
 * @param fd: file descriptor to read from
 * @param bufsz: size of input buffer to use, 0 is default
 *
 * Attach a file descriptor to an input buffer and ready it for use.
 *
 * @return zero on error, non-zero on success
 */
fibuf_t fibuf_new(int fd, size_t bufsz)
{
	struct _fibuf *b;

	b = calloc(1, sizeof(*b));
	if ( b ) {
		if ( !bufsz )
			bufsz = 8192;

		b->buf = malloc(bufsz);
		if ( b->buf == NULL )
			return 0;

		b->fd = fd;
		b->read_pos = b->buf;
		b->valid = 0;
		b->buf_sz = bufsz;
		b->eof = 0;
	}

	return b;
}

/** fibuf_close
 * @param b: a fibuf strcture to finish up with.
 *
 * Same as fibuf_detach, but we close the file too.
 */
void fibuf_close(fibuf_t b)
{
	if ( b ) {
		if ( b->fd != -1 )
			close(b->fd);

		/* XXX: We don't care about BADF, and EIO is unlikely for a
		 * read-only file descriptor, only problem if the user used
		 * it for writing too, but if they close it via this interface
		 * then they have a bug anyway.
		 */
		if ( b->buf )
			free(b->buf);
		free(b);
	}
}

/** fibuf_newfd
 * @param b: the fibuf structure to modify
 * @param fd: the new file descriptor
 *
 * An optimisation of fbuf_close followed by fbuf_attach,
 */
void fibuf_newfd(fibuf_t b, int fd)
{
	if ( b->fd == -1 )
		goto noclose;

	close(b->fd);

	/* XXX: We don't care about BADF, and EIO is unlikely for a
	 * read-only file descriptor, only problem if the user used
	 * it for writing too, but if they close it via this interface
	 * then they have a bug anyway.
	 */

noclose:
	b->fd = fd;
	b->valid = 0;
	b->eof = 0;
}

/**
 * @brief slow path for fibuf_read
 * @param b: A fibuf structure to use
 * @param buf: pointer to where you want to read to
 * @param sz: pointer to the length of your buffer
 *
 * Slow path to reading form a buffer, do not call directly, instead use
 * fibuf_read().
 *
 * @return zero on error, non-zero on success. In addition the
 * len parameter is set to the number of bytes actually read.
 */
static int fibuf_read_slow(fibuf_t b, uint8_t *buf, size_t *sz)
{
	size_t len = *sz;
	size_t final;
	size_t siz;
	int ret;

	*sz = 0;

	/* empty the buffer, b->valid may be 0 here */
	memcpy(buf, b->read_pos, b->valid);
	buf += b->valid;
	len -= b->valid;
	*sz += b->valid;
	b->read_pos = b->buf;
	b->valid = 0;

	/* If the remaining data is the same size or bigger than the
	 * buffer, there is no point going through the buffer, lets
	 * save the extra copy and read directly in to the users
	 * supplied buffer.
	 */
	if ( len >= b->buf_sz ) {
		siz = len;
		ret = fd_read(b->fd, buf, &siz, &b->eof);
		*sz += siz;
		return ret;
	}

	/* fill the buffer up again. */
	siz = b->buf_sz;
	ret = fd_read(b->fd, b->buf, &siz, &b->eof);
	b->valid = siz;

	/* copy the last bit, left could be 0 again */
	final = ( b->valid < len ) ? b->valid : len;
	memcpy(buf, b->read_pos, final);
	b->read_pos += final;
	b->valid -= final;
	*sz += final;

	return ret;
}

int inline fibuf_read(fibuf_t b, void *buf, size_t *len)
{
	if ( likely(*len <= b->valid) ) {
		memcpy(buf, b->read_pos, *len);
		b->read_pos += *len;
		b->valid -= *len;
		return 1;
	}

	return fibuf_read_slow(b, buf, len);
}

int fibuf_eof(fibuf_t b)
{
	return ( (b->valid == 0) && (b->eof != 0) );
}

int fibuf_fd(fibuf_t b)
{
	return b->fd;
}
