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
#include <bitwriter.h>

int main(int argc, char **argv)
{
#if 0
	struct bitwriter w;
	struct bitreader r;
	unsigned int i;
	uint8_t buf[8];

	bitwriter_init(&w, buf, sizeof(buf));
	for(i = 0; i < sizeof(buf) * 8; i++) {
		bitwriter_write(&w, i % 2, 1);
	}
	bitwriter_fini(&w);

	bitreader_init(&r, buf, sizeof(buf));
	for(i = 0; !bitreader_finished(&r); i++) {
		bitsr_t val;
		if ( !bitreader_read(&r, &val, 1) )
			break;
		printf("%lu = %u\n", val, i % 2);
	//	assert(val == (i % 2));
	}
#endif
	return EXIT_SUCCESS;
}
