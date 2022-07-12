/*
 * Author: Abhiram V (ramv)
 *
 * Dump hex
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "xxc.h"
#include "dump.h"
#include "revert.h"

static int ishex(char *p);
static int isoct(char *p);
static int isbin(char *p);
static void usage(void);
static void parse_args(int *argc, char **argv);

/* Options */
off_t g_lenb = -1;		 	// Dump len bytes starting at seek 
off_t g_seek = 0;		 	// Start at seek bytes
long g_colcnt = 16;		 	// Column count
long g_group_cnt = 2;		 	// Octets per group
char *g_separator = " ";	 	// To separate groups
int g_plain_dump = NO;		 	// Just dump it, no decorations.
int g_endian = BIG;			// Endianness
int g_base = HEX;			// Number base to use
int g_reverse = NO;			// Convert dump to binary
int g_ucase = NO;			// Use uppercase hexadecimal digits
int g_autoskip = NO;			// Skip repeating lines
int g_ascii = YES;			// Show corresponding ascii characters
int g_newline = YES;			// Use newlines after columns
int g_c_style = NO;			// Output in C include file style
int g_colors = NO;			// Colors!!


int main(int argc, char **argv)
{
	/* change argc to the index of first file arg or zero if none */
	parse_args(&argc, argv);

	if (g_ucase == YES && g_base == HEX)
		upper_hex();		// Convert hexdigits to uppercase

	if (g_reverse == NO) {
		if (argc == 0)
			dump_stdin();
		else
			dump_files(argc, argv);
	} else {
		if (argc == 0) {
			fprintf(stderr, "Expected filename.\n");
			exit(EXIT_FAILURE);
		} else {
			// if argv[argc+1] is NULL, use stdout
			patch(argv[argc], argv[argc+1]);
		}
	}
	return 0;
}

/*
 * parse_args:	parse the command line arguments
 */
void parse_args(int *argc, char **argv)
{
	int c;
	int i;	/* For usage in isbin. */

	while (1) {
		static struct option long_options[] = {
			{"seek", required_argument, NULL, 's'},
			{"column", required_argument, NULL, 'c'},
			{"group-count", required_argument, NULL, 'g'},
			{"length", required_argument, NULL, 'l'},
			{"separator", required_argument, NULL, 'S'},
			{"help", no_argument, NULL, 'h'},
			{"hex", no_argument, &g_base, HEX},
			{"oct", no_argument, &g_base, OCT},
			{"bin", no_argument, &g_base, BIN},
			{"little-endian", no_argument, &g_endian, LITTLE},
			{"upper-case", no_argument, &g_ucase, YES},
			{"auto-skip", no_argument, &g_autoskip, YES},
			{"no-ascii", no_argument, &g_ascii, NO},
			{"reverse", no_argument, &g_reverse, YES},
			{"include-style", no_argument, &g_c_style, YES},
			{"plain-dump", no_argument, &g_plain_dump, YES},
			{"no-newline", no_argument, &g_newline, NO},
			{"color", no_argument, &g_colors, YES},
			{0,	0,	0,	0}
		};

		if ((c = getopt_long(*argc, argv, "s:c:g:l:S:obeuaAriChpn",
						long_options, NULL)) == -1)
			break;
		switch (c) {
			case 's':
				if (ishex(optarg))
					g_seek = strtol(optarg, NULL, 16);
				else if ((i = isbin(optarg)))
					g_seek = strtol(optarg + i, NULL, 2);
				else if (isoct(optarg))
					g_seek = strtol(optarg, NULL, 8);
				else
					g_seek = strtol(optarg, NULL, 10);
				break;
			case 'c':
				if (ishex(optarg))
					g_colcnt = strtol(optarg, NULL, 16);
				else if ((i = isbin(optarg)))
					g_colcnt = strtol(optarg + i, NULL, 2);
				else if (isoct(optarg))
					g_colcnt = strtol(optarg, NULL, 8);
				else
					g_colcnt = strtol(optarg, NULL, 10);
				if (g_colcnt <= 0)
					g_colcnt = 16;
				break;
			case 'g':
				if (ishex(optarg))
					g_group_cnt = strtol(optarg, NULL, 16);
				else if ((i = isbin(optarg)))
					g_group_cnt = strtol(optarg + i, NULL, 2);
				else if (isoct(optarg))
					g_group_cnt = strtol(optarg, NULL, 8);
				else 
					g_group_cnt = strtol(optarg, NULL, 10);
				if (g_group_cnt <= 0)
					g_group_cnt = 2;
				break;
			case 'l':
				if (ishex(optarg))
					g_lenb = strtol(optarg, NULL, 16);
				/* isbin returns the beginning of the bitstring
				 * or 0 if it isn't a bitstring (0b prefix). */
				else if ((i = isbin(optarg)))
					g_lenb = strtol(optarg + i, NULL, 2);
				else if (isoct(optarg)) 
					g_lenb = strtol(optarg, NULL, 8);
				else
					g_lenb = strtol(optarg, NULL, 10);
				break;
			case 'S':
				if (optarg != NULL)
					g_separator = optarg;
				break;
			case 'o':
				g_base = OCT;
				break;
			case 'b':
				g_base = BIN;
				break;
			case 'e':
				g_endian = LITTLE;
				break;
			case 'u':
				g_ucase = YES;
				break;
			case 'p':
				g_plain_dump = YES;
				break;
			case 'A':
				g_ascii = NO;
				break;
			case 'a':
				g_autoskip = YES;
				break;
			case 'r':
				g_reverse = YES;
				break;
			case 'i':
				g_c_style = YES;
				g_group_cnt = 1;
				g_separator = malloc(strlen(", 0x") + 1);
				strcpy(g_separator, ", 0x");
				break;
			case 'C':
				g_colors = YES;
				break;
			case 'n':
				g_newline = NO;
				break;
			case 'h':
				printf("Help yourself\n");
				exit(EXIT_SUCCESS);
				break;
			case '?':
				exit(EXIT_FAILURE);
		}
	}
	if ((g_endian == LITTLE) && !(ISPOW_OF2(g_group_cnt))) {
		fprintf(stderr, "number of octets group must be a power of 2 "
				"with -e\n");
		exit(EXIT_FAILURE);
	}
	/* Get filename if any */
	if (optind < *argc)
		*argc = optind;	/* set argc to index of first non-option arg */
	else	/* No filename arguments */
		*argc = 0;

}
/*
 * usage: print usage information (options, behaviour, etc)
 */
void usage(void)
{
	/* Print usage information. */
}

/*
 * ishex: return 1 if string has [+-]0x prefix
 */
int ishex(char *p)
{
	if (p[0] == '-' || p[0] == '+') {
		if (p[1] == '0' && (p[2] == 'x' || p[2] == 'X'))
			return 1;
		else
			return 0;
	} else if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * isoct: return 1 if string has [+-]0[oO] prefix
 */
int isoct(char *p)
{
	if (p[0] == '-' || p[0] == '+') {
		if (p[1] == '0' && (p[2] == 'o' || p[2] == 'O' || p[2] != '\0'))
			return 1;
		else
			return 0;
	} else if (p[0] == '0' && (p[1] == 'o' || p[1] == 'O' || p[1] != '\0')) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * isbin: if string has [+-]0b prefix return index of beginning of bitstring
 * (excluding the prefix).
 */
int isbin(char *p)
{
	if (p[0] == '-' || p[0] == '+') {
		if (p[1] == '0' && (p[2] == 'b' || p[2] == 'B'))
			return 3;	/* bitstring begins at p[3] */
		else
			return 0;
	} else if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B')) {
		return 2;	/* bistrting begins at p[2] */
	} else {
		return 0;
	}
}
