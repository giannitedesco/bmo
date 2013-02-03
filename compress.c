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

const char *cmd = "compress";

static int usage(int code)
{
	FILE *f = (code) ? stderr : stdout;

	fprintf(f, "%s: Usage\n", cmd);
	fprintf(f, "\t$ %s < in > out\n", cmd);
	fprintf(f, "\n");

	return code;
}

static int compress(int infd, int outfd)
{
	uint8_t buf[BMO_BLOCK_SIZE];
	bwt_t idx;
	size_t sz;
	int eof;

again:
	sz = sizeof(buf);
	if ( !fd_read(infd, buf, &sz, &eof) ) {
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());
		return 0;
	}

	fprintf(stderr, "read %zu bytes\n", sz);

	if ( !eof )
		goto again;

	hex_dumpf(stderr, buf, sz, 0);

	bwt_encode(buf, sz, &idx);
	fprintf(stderr, "BWT: rotation %u\n", idx);
	hex_dumpf(stderr, buf, sz, 0);

	mtf_encode(buf, sz);
	fprintf(stderr, "MTF encoded:\n");
	hex_dumpf(stderr, buf, sz, 0);

	omega_encode(buf, &sz);
	fprintf(stderr, "Omega encoded %zu bytes:\n", sz);
	hex_dumpf(stderr, buf, sz, 0);

	if ( !fd_write(outfd, &idx, sizeof(idx))) {
		fprintf(stderr, "%s: write: %s\n", cmd, os_err());
		return 0;
	}

	if ( !fd_write(outfd, buf, sz)) {
		fprintf(stderr, "%s: write: %s\n", cmd, os_err());
		return 0;
	}
	return 1;
}

int main(int argc, char **argv)
{
	if ( argc > 0 )
		cmd = argv[0];

	if ( !compress(STDIN_FILENO, STDOUT_FILENO) )
		return usage(EXIT_FAILURE);

	return EXIT_SUCCESS;
}
