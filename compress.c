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

static int bwtcmp(uint8_t *buf, size_t len, bwt_t a, bwt_t b)
{
	size_t i;

	for(i = 0; i < len; i++) {
		uint8_t aa, bb;
		aa = buf[(a + i) % len];
		bb = buf[(b + i) % len];
		if ( aa != bb )
			return aa - bb;
	}

	return 0;
}

static void sift_down(uint8_t *buf, size_t len,
			bwt_t *bwt, bwt_t start, bwt_t end)
{
	bwt_t last = (end + 1) / 2;
	while ( start < last ) {
		bwt_t l = (start * 2) + 1;
		bwt_t r = l + 1;
		bwt_t largest;

		if ( r <= end ) {
			if ( bwtcmp(buf, len, bwt[l], bwt[r]) < 0 ) {
				largest = r;
			}else{
				largest = l;
			}
		}else{
			largest = l;
		}

		if ( bwtcmp(buf, len, bwt[largest], bwt[start]) > 0 ) {
			bwt_t tmp;
			tmp = bwt[start];
			bwt[start] = bwt[largest];
			bwt[largest] = tmp;
			start = largest;
		}else
			return;
	}
}

static void heapify(uint8_t *buf, bwt_t *bwt, bwt_t num)
{
	int i;

	if ( num < 2 )
		return;
	for(i = (num - 1) / 2; i >= 0; i--)
		sift_down(buf, num, bwt, i, num - 1);
}

static int compress(int infd, int outfd)
{
	uint8_t buf[BMO_BLOCK_SIZE];
	uint8_t obuf[BMO_BLOCK_SIZE];
	bwt_t bwt[BMO_BLOCK_SIZE];
	size_t i, sz;
	int eof;

again:
	sz = sizeof(buf);
	if ( !fd_read(infd, buf, &sz, &eof) )
		fprintf(stderr, "%s: read: %s\n", cmd, os_err());

	printf("read %zu bytes\n", sz);

	if ( !eof )
		goto again;

	for(i = 0; i < sz; i++)
		bwt[i] = i;

	heapify(buf, bwt, sz);

	for(i = sz - 1; i > 0; i--) {
		bwt_t tmp = bwt[0];
		bwt[0] = bwt[i];
		bwt[i] = tmp;
		sift_down(buf, sz, bwt, 0, i - 1);
	}

	for(i = 0; i < sz; i++) {
#if 0
		printf("%zu: %u '%c' -> '%c'\n",
			i, bwt[i],
			buf[bwt[i]], buf[(bwt[i] + sz - 1)%sz]);
#endif
		obuf[i] = buf[(bwt[i] + sz - 1) % sz];
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
