/*
* This file is part of bmo
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*
* Skew algorithm sample code used from:
* "Simple Linear Work Suffix Array Construction"
* By {juha,sanders} at mpi-sb.mpg.de.
* http://www.mpi-sb.mpg.de/ ̃sanders/programs/suffix/
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#include <bmo.h>

static inline int leq2(int a1, int a2, int b1, int b2)
{				// lexic. order for pairs
	return ((a1 < b1) || ((a1 == b1) && (a2 <= b2)));
}				// and triples

static inline int leq3(int a1, int a2, int a3, int b1, int b2, int b3)
{
	return ((a1 < b1) || ((a1 == b1) && leq2(a2, a3, b2, b3)));
}

// stably sort a[0..n-1] to b[0..n-1] with keys in 0..K from r
static void radixPass(int *a, int *b, const int *r, int n, int K)
{				// count occurrences
	int c[K + 1];	// counter array
	int i, sum;

	memset(c, 0, sizeof(c));

	for (i = 0; i <= K; i++)
		c[i] = 0;	// reset counters
	for (i = 0; i < n; i++) {
		assert(r[a[i]] < K + 1);
		c[r[a[i]]]++;	// count occurences
	}
	for (i = 0, sum = 0; i <= K; i++) {	// exclusive prefix sums
		int t = c[i];
		c[i] = sum;
		sum += t;
	}
	for (i = 0; i < n; i++)
		b[c[r[a[i]]]++] = a[i];	// sort
}

// find the suffix array SA of s[0..n-1] in {1..K}^n
// require s[n]=s[n+1]=s[n+2]=0, n>=2
static void suffixArray(const int *s, int *SA, int n, int K)
{
	int n0 = (n + 2) / 3, n1 = (n + 1) / 3, n2 = n / 3, n02 = n0 + n2;
	int s12[n02 + 3];
	int SA12[n02 + 3];
	int s0[n0];
	int SA0[n0];
	int name = 0, c0 = -1, c1 = -1, c2 = -1;
	int i, j, p, t, k;

	memset(s12, 0, sizeof(s12));
	memset(SA12, 0, sizeof(SA12));
	memset(s0, 0, sizeof(s0));
	memset(SA0, 0, sizeof(SA0));
	s12[n02] = s12[n02 + 1] = s12[n02 + 2] = 0;
	SA12[n02] = SA12[n02 + 1] = SA12[n02 + 2] = 0;

	// generate positions of mod 1 and mod  2 suffixes
	// the "+(n0-n1)" adds a dummy mod 1 suffix if n%3 == 1
	for (i = 0, j = 0; i < n + (n0 - n1); i++)
		if (i % 3 != 0)
			s12[j++] = i;

	// lsb radix sort the mod 1 and mod 2 triples
	radixPass(s12, SA12, s + 2, n02, K);
	radixPass(SA12, s12, s + 1, n02, K);
	radixPass(s12, SA12, s, n02, K);

	// find lexicographic names of triples
	for (i = 0; i < n02; i++) {
		if (s[SA12[i]] != c0 || s[SA12[i] + 1] != c1
		    || s[SA12[i] + 2] != c2) {
			name++;
			c0 = s[SA12[i]];
			c1 = s[SA12[i] + 1];
			c2 = s[SA12[i] + 2];
		}
		if (SA12[i] % 3 == 1) {
			s12[SA12[i] / 3] = name;
		}		// left half
		else {
			s12[SA12[i] / 3 + n0] = name;
		}		// right half
	}

	// recurse if names are not yet unique
	if (name < n02) {
		suffixArray(s12, SA12, n02, name);
		// store unique names in s12 using the suffix array 
		for (i = 0; i < n02; i++)
			s12[SA12[i]] = i + 1;
	} else			// generate the suffix array of s12 directly
		for (i = 0; i < n02; i++)
			SA12[s12[i] - 1] = i;

	// stably sort the mod 0 suffixes from SA12 by their first character
	for (i = 0, j = 0; i < n02; i++)
		if (SA12[i] < n0)
			s0[j++] = 3 * SA12[i];
	radixPass(s0, SA0, s, n0, K);

	// merge sorted SA0 suffixes and sorted SA12 suffixes
	for (p = 0, t = n0 - n1, k = 0; k < n; k++) {
#define GetI() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)
		int i = GetI();	// pos of current offset 12 suffix
		int j = SA0[p];	// pos of current offset 0  suffix
		if (SA12[t] < n0 ? leq2(s[i], s12[SA12[t] + n0], s[j], s12[j / 3]) : leq3(s[i], s[i + 1], s12[SA12[t] - n0 + 1], s[j], s[j + 1], s12[j / 3 + n0])) {	// suffix from SA12 is smaller
			SA[k] = i;
			t++;
			if (t == n02) {	// done --- only SA0 suffixes left
				for (k++; p < n0; p++, k++)
					SA[k] = SA0[p];
			}
		} else {
			SA[k] = j;
			p++;
			if (p == n0) {	// done --- only SA12 suffixes left
				for (k++; t < n02; t++, k++)
					SA[k] = GetI();
			}
		}
	}
}

void bwt_encode(uint8_t *buf, size_t len, bwt_t *idx)
{
	size_t biglen = 3 * ((len * 2 + 2) / 3);
	int *tmp, *SA;
	uint8_t out[len];
	unsigned int i;
	size_t o;

	assert((biglen % 3) == 0);
	tmp = calloc(biglen + 3, sizeof(*tmp));
	SA = calloc(biglen + 3, sizeof(*SA));
	for(i = 0; i < biglen; i++) {
		tmp[i] = buf[i % len];
	}

	memset(SA, 0, sizeof(SA));
	suffixArray(tmp, SA, biglen, 256);
	for(i = o = 0; i < biglen; i++) {
		if ( SA[i] < (int)len ) {
			out[o] = buf[(SA[i] + len - 1) % len];
			if ( !SA[i] )
				*idx = o;
			o++;
		}
	}
	memcpy(buf, out, len);
	free(tmp);
	free(SA);
}
