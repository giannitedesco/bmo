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

static void mtf(uint8_t *tbl, uint8_t c)
{
	unsigned int i;
	uint8_t old;

	old = tbl[c];

	for(i = 0; i < 0x100; i++) {
		if ( tbl[i] < old )
			tbl[i]++;
	}
	tbl[c] = 0;
}

void mtf_encode(uint8_t *buf, size_t len)
{
	uint8_t tbl[0x100];
	size_t i;

	for(i = 0; i < 0x100; i++)
		tbl[i] = i;

	for(i = 0; i < len; i++) {
		uint8_t s = buf[i];

		buf[i] = tbl[s];
		mtf(tbl, s);
	}
}
