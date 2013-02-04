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

#if 0
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#define dhex_dump(...) dhex_dump(__VA_ARGS)
#else
#define dprintf(x...) do { } while(0);
#define dhex_dump(x...) do { } while(0);
#endif

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
	uint8_t orig[BMO_BLOCK_SIZE];
	uint8_t *ptr;
	struct bmo_hdr h;
	int compressed;
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
	if ( !fd_read(infd, orig, &sz, &eof) ) {
		dprintf("%s: read: %s\n", cmd, os_err());
		return 0;
	}

	if ( !sz )
		goto done;

	h.h_len += sz;

	dprintf("read %zu bytes\n", sz);

	dhex_dump(orig, sz, 0);
	memcpy(buf, orig, sizeof(buf));

	bwt_encode(buf, sz, &idx);
	dprintf("BWT: rotation %u\n", idx);
	dhex_dump(buf, sz, 0);

	mtf_encode(buf, sz);
	dprintf("MTF encoded:\n");
	dhex_dump(buf, sz, 0);

	omega_encode(buf, &sz, &compressed);
	if ( compressed ) {
		dprintf("Omega encoded %zu bytes:\n", sz);
		dhex_dump(buf, sz, 0);
		idx |= BMO_BLOCK_COMPRESSED;
		ptr = buf;
	}else{
		dprintf("compress failed:\n");
		dhex_dump(orig, sz, 0);
		ptr = orig;
	}

	if ( !fobuf_write(out, &idx, sizeof(idx))) {
		dprintf("%s: write: %s\n", cmd, os_err());
		return 0;
	}

	if ( !fobuf_write(out, ptr, sz)) {
		dprintf("%s: write: %s\n", cmd, os_err());
		return 0;
	}

	if ( !eof )
		goto again;

done:
	if ( !fobuf_flush(out) )
		return 0;

	if ( !fd_pwrite(fobuf_fd(out), 0, &h, sizeof(h)) )
		return 0;

	dprintf("uncompressed len = %"PRId64"\n", h.h_len);
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
