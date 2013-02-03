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
#include <bitwriter.h>

int omega_encode(uint8_t *buf, size_t *len)
{
	struct bitwriter wr;
	uint8_t out[(*len + 7) & ~0x7U];
	size_t i, olen;

	bitwriter_init(&wr, out, sizeof(out));

	for(i = 0; i < *len; i++) {
		unsigned int num = buf[i] + 1;
		unsigned int bcnt = 0;
		bitsw_t stk = 0;

		while(num > 1) {
			unsigned int l, j, tmp;
			for(tmp = num, l = 0; tmp > 0; tmp >>= 1)
				l++;
			for(j = 0; j < l; j++) {
				stk <<= 1;
				stk |= ((num >> j) & 1);
				bcnt++;
			}
			num = l - 1;
		}
		while(bcnt) {
			if ( !bitwriter_write(&wr, stk & 1, 1) )
				return 0;
			//fprintf(stderr, "%c", (stk & 1) ? '1' : '0');
			stk >>= 1;
			bcnt--;
		}
		if ( !bitwriter_write(&wr, 0, 1) )
			return 0;
		//fprintf(stderr, "0\n");
	}
	olen = bitwriter_fini(&wr);

//	if ( olen < *len ) {
		memcpy(buf, out, *len);
		*len = olen;
		return 1;
//	}

	return 0;
}
