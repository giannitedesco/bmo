/*
* This file is part of cola
* Copyright (c) 2013 Gianni Tedesco
* This program is released under the terms of the GNU GPL version 2
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *cmd = "compress";

static int usage(int code)
{
	FILE *f = (code) ? stderr : stdout;

	fprintf(f, "%s: Usage\n", cmd);
	fprintf(f, "\t$ %s < in > out\n", cmd);
	fprintf(f, "\n");

	return code;
}

int main(int argc, char **argv)
{
	if ( argc > 0 )
		cmd = argv[0];

	return usage(EXIT_FAILURE);
}
