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
#include <assert.h>

#include <bmo.h>
#include <os.h>

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

void bwt_encode(uint8_t *buf, size_t len)
{
	uint8_t obuf[len];
	bwt_t bwt[len];
	size_t i;

	assert(len <= BMO_BLOCK_SIZE);

	for(i = 0; i < len; i++)
		bwt[i] = i;

	/* TODO: don't use heapsort */
	heapify(buf, bwt, len);
	for(i = len - 1; i > 0; i--) {
		bwt_t tmp = bwt[0];
		bwt[0] = bwt[i];
		bwt[i] = tmp;
		sift_down(buf, len, bwt, 0, i - 1);
	}

	for(i = 0; i < len; i++) {
#if 0
		printf("%zu: %u '%c' -> '%c'\n",
			i, bwt[i],
			buf[bwt[i]], buf[(bwt[i] + len - 1)%len]);
#endif
		obuf[i] = buf[(bwt[i] + len - 1) % len];
	}

	memcpy(buf, obuf, len);
}
