/*
* This file is part of bmo
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <bmo.h>
#include <os.h>

const char *cmd = "decompress";

static int usage(int code)
{
	FILE *f = (code) ? stderr : stdout;

	fprintf(f, "%s: Usage\n", cmd);
	fprintf(f, "\t$ %s < in > out\n", cmd);
	fprintf(f, "\n");

	return code;
}

static int decompress(int infd, int outfd)
{
	uint8_t buf[BMO_BLOCK_SIZE];
	size_t sz;
	bwt_t idx;
	int eof;

again:
	sz = sizeof(idx);
	if ( !fd_read(infd, &idx, &sz, &eof) ) {
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());
		return 0;
	}

	if ( eof ) {
		fprintf(stderr, "%s: desync on bwt read\n", cmd);
		return 1;
	}

	sz = sizeof(buf);
	if ( !fd_read(infd, buf, &sz, &eof) ) {
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());
		return 0;
	}

	printf("read %zu bytes\n", sz);

	if ( !eof )
		goto again;

	bwt_decode(buf, sz, idx);
	return 1;
}

int main(int argc, char **argv)
{
	if ( argc > 0 )
		cmd = argv[0];

	if ( !decompress(STDIN_FILENO, STDOUT_FILENO) )
		return usage(EXIT_FAILURE);

	return EXIT_SUCCESS;
}
