/*
* This file is part of bmo
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*/
#ifndef _BITREADER_H
#define _BITREADER_H

#define BITREADER_WIDTH	64
#define BR_TYPE(width) uint ## width ## _t
#define BR_TYPE_EXPAND(x) BR_TYPE(x)
typedef BR_TYPE_EXPAND(BITREADER_WIDTH) bitsr_t;

struct bitreader {
	fibuf_t in;
	unsigned int nbits;
	bitsr_t word;
};

static inline int bitreader_slurp(struct bitreader *r)
{
	size_t sz = sizeof(r->word);
	if ( !fibuf_read(r->in, &r->word, &sz) || sz != sizeof(r->word) )
		return 0;
	r->nbits = BITREADER_WIDTH;
	return 1;
}

static inline void bitreader_init(struct bitreader *r, fibuf_t in)
{
	r->in = in;
	r->nbits = 0;
	r->word = 0;
}

static inline int bitreader_finished(struct bitreader *r)
{
	if ( !fibuf_eof(r->in) || r->nbits )
		return 0;
	return 1;
}

static inline int bitreader_read(struct bitreader *r,
				bitsr_t *out,
				unsigned int bits)
{
	assert(bits <= BITREADER_WIDTH);

	for(*out = 0; bits; ) {
		unsigned int b;
		unsigned int shift;

		if ( !r->nbits && !bitreader_slurp(r) )
			return 0;

		if ( r->nbits < bits )
			b = r->nbits;
		else
			b = bits;

		shift = (r->nbits) - (b);
		*out <<= b;
		*out |= (r->word >> shift) & ((1 << b) - 1);
		r->nbits -= b;
		bits -= b;
	}

	return 1;
}

#endif /* _BITREADER_H */
