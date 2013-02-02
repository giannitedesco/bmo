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


static uint8_t mtf(uint8_t *tbl, uint8_t c)
{
	uint8_t old = tbl[c];
	if ( c ) {
		memmove(tbl + 1, tbl, c);
		tbl[0] = old;
	}
	return old;
}

void mtf_decode(uint8_t *buf, size_t len)
{
	uint8_t tbl[0x100];
	size_t i;

	for(i = 0; i < 0x100; i++)
		tbl[i] = i;

	for(i = 0; i < len; i++) {
		buf[i] = mtf(tbl, buf[i]);
	}
}
