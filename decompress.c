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

#if 0
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#define dhex_dump(...) dhex_dump(__VA_ARGS)
#else
#define dprintf(x...) do { } while(0);
#define dhex_dump(x...) do { } while(0);
#endif

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
	int compressed;
	size_t sz;
	bwt_t idx;


	sz = sizeof(h);
	if ( !fibuf_read(in, &h, &sz) ) {
		dprintf("%s: read: %s\n", cmd, os_err());
		return 0;
	}

	if ( fibuf_eof(in) || sz < sizeof(h) ) {
		dprintf("%s: desync on hdr read\n", cmd);
		return 1;
	}

	if ( h.h_magic != BMO_MAGIC ) {
		dprintf("%s: bad magic\n", cmd);
		return 1;
	}
	if ( h.h_vers != BMO_CURRENT_VERS ) {
		dprintf("%s: wrong version\n", cmd);
		return 1;
	}
again:
	sz = sizeof(idx);
	if ( !fibuf_read(in, &idx, &sz) ) {
		dprintf("%s: read: %s\n", cmd, os_err());
		return 0;
	}

	if ( fibuf_eof(in) || sz < sizeof(idx) ) {
		dprintf("%s: desync on bwt read\n", cmd);
		return 1;
	}

	compressed = !!(idx & BMO_BLOCK_COMPRESSED);
	idx &= ~BMO_BLOCK_COMPRESSED;

	sz = (h.h_len < BMO_BLOCK_SIZE) ? h.h_len : BMO_BLOCK_SIZE;
	h.h_len -= sz;

	if ( compressed ) {
		omega_decode(in, buf, sz);
		dprintf("omega decode:\n");
		dhex_dump(buf, sz, 0);

		mtf_decode(buf, sz);
		dprintf("MTF decode:\n");
		dhex_dump(buf, sz, 0);

		bwt_decode(buf, sz, idx);
		dprintf("BWT decode:\n");
		dhex_dump(buf, sz, 0);
	}else{
		if ( !fibuf_read(in, buf, &sz) )
			return 0;
		dprintf("uncompressed block\n");
	}

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
