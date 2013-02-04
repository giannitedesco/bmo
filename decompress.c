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
#include <bmo-format.h>
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

static int decompress(fibuf_t in, int outfd)
{
	uint8_t buf[BMO_BLOCK_SIZE];
	struct bmo_hdr h;
	size_t sz;
	bwt_t idx;


	sz = sizeof(h);
	if ( !fibuf_read(in, &h, &sz) ) {
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());
		return 0;
	}

	if ( fibuf_eof(in) || sz < sizeof(h) ) {
		fprintf(stderr, "%s: desync on hdr read\n", cmd);
		return 1;
	}

	if ( h.h_magic != BMO_MAGIC ) {
		fprintf(stderr, "%s: bad magic\n", cmd);
		return 1;
	}
	if ( h.h_vers != BMO_CURRENT_VERS ) {
		fprintf(stderr, "%s: wrong version\n", cmd);
		return 1;
	}
again:
	sz = sizeof(idx);
	if ( !fibuf_read(in, &idx, &sz) ) {
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());
		return 0;
	}

	if ( fibuf_eof(in) || sz < sizeof(idx) ) {
		fprintf(stderr, "%s: desync on bwt read\n", cmd);
		return 1;
	}

	sz = (h.h_len < BMO_BLOCK_SIZE) ? h.h_len : BMO_BLOCK_SIZE;
	h.h_len -= sz;

	omega_decode(in, buf, sz);
//	fprintf(stderr, "omega decode:\n");
//	hex_dumpf(stderr, buf, sz, 0);

	mtf_decode(buf, sz);
//	fprintf(stderr, "MTF decode:\n");
//	hex_dumpf(stderr, buf, sz, 0);

	bwt_decode(buf, sz, idx);
//	fprintf(stderr, "BWT decode:\n");
//	hex_dumpf(stderr, buf, sz, 0);

	if ( !fd_write(outfd, buf, sz) )
		return 0;

	if ( !fibuf_eof(in) )
		goto again;

	return 1;
}

int main(int argc, char **argv)
{
	fibuf_t in;

	if ( argc > 0 )
		cmd = argv[0];

	in = fibuf_new(STDIN_FILENO, 0);
	if ( NULL == in )
		return EXIT_FAILURE;

	if ( !decompress(in, STDOUT_FILENO) )
		return usage(EXIT_FAILURE);

	return EXIT_SUCCESS;
}
