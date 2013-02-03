/*
* This file is part of bmo
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*/
#ifndef _BITWRITER_H
#define _BITWRITER_H

#define BITWRITER_WIDTH	64
#define BW_TYPE(width) uint ## width ## _t
#define BW_TYPE_EXPAND(x) BW_TYPE(x)
typedef BW_TYPE_EXPAND(BITWRITER_WIDTH) bitsw_t;

struct bitwriter {
	const uint8_t *out, *cur, *end;
	unsigned int nbits;
	bitsw_t word;
};

static inline void bitwriter_spill(struct bitwriter *w)
{
	if ( w->nbits < BITWRITER_WIDTH ) {
		*(bitsw_t *)w->cur = w->word;
		w->word = 0;
		w->nbits = BITWRITER_WIDTH;
		w->cur += sizeof(bitsw_t);
	}
}

static inline void bitwriter_init(struct bitwriter *w,
					uint8_t *ptr,
					size_t len)
{
	w->cur = w->out = ptr;
	w->end = ptr + len;
	w->nbits = BITWRITER_WIDTH;
	w->word = 0;
}

static inline int bitwriter_write(struct bitwriter *w,
				bitsw_t in,
				unsigned int bits)
{
	unsigned int b;

	while(bits) {
		if ( w->cur >= w->end )
			return 0;

		if ( bits > w->nbits )
			b = w->nbits;
		else
			b = bits;

		w->word |= (in & ((1 << b) - 1)) <<
				(w->nbits - 1);
		w->nbits -= b;
		bits -= b;
		in >>= b;

		if ( !w->nbits )
			bitwriter_spill(w);
	}
	return 1;
}

static inline size_t bitwriter_fini(struct bitwriter *w)
{
	if ( w->nbits < BITWRITER_WIDTH )
		bitwriter_spill(w);
	return (w->cur - w->out);
}

#endif /* _BITWRITER_H */
