/*
* This file is part of bmo
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*/
#ifndef _BMO_H
#define _BMO_H

#include <compiler.h>

#define BMO_BLOCK_SIZE	(1 << 13)

typedef uint16_t bwt_t;

void hex_dumpf(FILE *f, const uint8_t *tmp, size_t len, size_t llen);
void hex_dump(const uint8_t *ptr, size_t len, size_t llen);

void bwt_encode(uint8_t *buf, size_t len, uint16_t *idx);
void bwt_decode(uint8_t *buf, size_t len, bwt_t idx);

void mtf_encode(uint8_t *buf, size_t len);
void mtf_decode(uint8_t *buf, size_t len);

#endif /* _BMO_H */
