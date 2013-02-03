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
#include <bitreader.h>

void omega_decode(uint8_t *buf, size_t *len)
{
	uint8_t out[BMO_BLOCK_SIZE];
	struct bitreader r;
	size_t o;

	bitreader_init(&r, buf, *len);
	for(o = 0; !bitreader_finished(&r); o++) {
		unsigned num = 1;
		bitsr_t val;
		while ( bitreader_read(&r, &val, 1) ) {
			unsigned i, l = num;
//			fprintf(stderr, "%lx", val);
			if ( !val )
				break;
			num = 1;
			for(i = 0; i < l; i++) {
				num <<= 1;
				if ( !bitreader_read(&r, &val, 1) )
					break;
//				fprintf(stderr, "%lx", val);
				if ( val )
					num |= 1;
			}
		}
//		fprintf(stderr, " = 0x%.2x / 0x%.2x\n", num, num - 1);
		num -= 1;
		assert(num < 0x100);
		out[o] = num;

		if ( (o + 1) > 1536 )
			break;
	}

	memcpy(buf, out, o);
	*len = o;
}
