/*
 * This file is part of dotscara
 * Copyright (c) 2003 Gianni Tedesco
 * Released under the terms of the GNU GPL version 2
 */
#ifndef _FIBUF_HEADER_INCLUDED_
#define _FIBUF_HEADER_INCLUDED_

/* for memcpy */
#include <string.h>

typedef struct _fibuf *fibuf_t;

_private _check_result fibuf_t fibuf_new(int fd, size_t bufsz);
_private void fibuf_close(fibuf_t b);
_private void fibuf_newfd(fibuf_t b, int fd);
_check_result int _public fibuf_read(fibuf_t b, void *buf, size_t *len);
_private int fibuf_eof(fibuf_t b);
_private int fibuf_fd(fibuf_t b);

#endif /* _FIBUF_HEADER_INCLUDED_ */
