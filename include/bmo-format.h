/*
* This file is part of bmo
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*/
#ifndef _BMO_FORMAT_H
#define _BMO_FORMAT_H

#define BMO_MAGIC		(('O' << 24)|('M'<<16)|('B'<<8))
#define BMO_CURRENT_VERS	1

struct bmo_hdr {
	uint32_t h_vers;
	uint32_t h_magic;
	uint64_t h_len;
} _packed;

#endif /* _BMO_FORMAT_H */
