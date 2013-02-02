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

void bwt_decode(uint8_t *buf, size_t len, bwt_t idx)
{
	uint8_t d[len];
	bwt_t t[len], k;
	size_t i, j, ti;

	assert(idx < len);

	/* build T, then decode */
	for(ti = i = 0; i < 0x100; i++) {
		for(j = 0; j < len; j++) {
			if ( buf[j] == i ) {
				t[ti] = j;
				ti++;
			}
		}
	}

	for(k = t[idx], j = 0; j < len; j++, k = t[k]) {
		d[j] = buf[k];
	}
	memcpy(buf, d, len);
}
