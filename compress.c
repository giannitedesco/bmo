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
#include <inttypes.h>

#include <bmo.h>
#include <bmo-format.h>
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

static int compress(int infd, fobuf_t out)
{
	uint8_t buf[BMO_BLOCK_SIZE];
	struct bmo_hdr h;
	size_t sz;
	bwt_t idx;
	int eof;

	h.h_vers = BMO_CURRENT_VERS;
	h.h_magic = BMO_MAGIC;
	h.h_len = 0;

	if ( !fobuf_write(out, &h, sizeof(h)) )
		return 0;

again:
	sz = sizeof(buf);
	if ( !fd_read(infd, buf, &sz, &eof) ) {
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());
		return 0;
	}

	h.h_len += sz;

//	fprintf(stderr, "read %zu bytes\n", sz);

//	hex_dumpf(stderr, buf, sz, 0);

	bwt_encode(buf, sz, &idx);
//	fprintf(stderr, "BWT: rotation %u\n", idx);
//	hex_dumpf(stderr, buf, sz, 0);

	mtf_encode(buf, sz);
//	fprintf(stderr, "MTF encoded:\n");
//	hex_dumpf(stderr, buf, sz, 0);

	omega_encode(buf, &sz);
//	fprintf(stderr, "Omega encoded %zu bytes:\n", sz);
//	hex_dumpf(stderr, buf, sz, 0);

	if ( !fobuf_write(out, &idx, sizeof(idx))) {
		fprintf(stderr, "%s: write: %s\n", cmd, os_err());
		return 0;
	}

	if ( !fobuf_write(out, buf, sz)) {
		fprintf(stderr, "%s: write: %s\n", cmd, os_err());
		return 0;
	}

	if ( !eof )
		goto again;

	if ( !fobuf_flush(out) )
		return 0;

	if ( !fd_pwrite(fobuf_fd(out), 0, &h, sizeof(h)) )
		return 0;

	fprintf(stderr, "uncompressed len = %"PRId64"\n", h.h_len);
	return 1;
}

int main(int argc, char **argv)
{
	fobuf_t out;

	if ( argc > 0 )
		cmd = argv[0];

	out = fobuf_new(STDOUT_FILENO, 0);
	if ( NULL == out )
		return 0;

	if ( !compress(STDIN_FILENO, out) )
		return usage(EXIT_FAILURE);

	if ( !fobuf_close(out) )
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}
