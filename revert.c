/*
 * revert.c
 * ========
 *
 * Handle the converting of dump to binary.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "revert.h"
#include "xxc.h"

/*
 * patch: convert dump to bin.
 */
void patch(char *infile, char *outfile)
{
	FILE	*in = NULL, *out = NULL, *tmpout = NULL;
	char	*offset_str = NULL;	// Offset read from dump file
	char	*ascii = NULL;		// ASCII column
	char	ndigs = 0;	 	// no: of digits according to base
	off_t	dump_offset = 0;	// Offset read from dump file
	off_t	out_off = 0;		// Number of bytes into binary file (output)
	int	size = 0;		// size of one row of dump file (excluding ASCII column)
	int	c = 0, r = 0;
	char	*p = NULL, *o = NULL;
	static char template[] = "bin.XXXXXX";

	if ((in = fopen(infile, "r")) == NULL) {
		fprintf(stderr, "Couldn't read input file: ");
		fflush(stdout);
		perror("");
		exit(EXIT_FAILURE);
	}
	if (outfile != NULL) {
		if (fopen(outfile, "r") == NULL) {	// Doesn't exist? 
			if ((out = fopen(outfile, "w")) == NULL) {
				fprintf(stderr, "Could not open output file '%s': ",
						outfile);
				fflush(stdout);
				perror("");
				exit(EXIT_FAILURE);
			} else {	// Opened outfile successfully.
				tmpout = out;
			}
		} else if (tmpout == NULL) {	// outfile exists?
			mkstemp(template);
			tmpout = fopen(template, "w");	
		}
	} else {	/* outfile == NULL? Use stdout */
		out = stdout;
		tmpout = stdout;
	}

	if (tmpout == NULL)
		tmpout = out;
	
	if (g_base == HEX)
		ndigs = 2;	/* hex has 2 digits */
	else if (g_base == OCT)
		ndigs = 3;
	else
		ndigs = 8;
	size = g_colcnt * ndigs;
	size += ((g_colcnt / g_group_cnt) + (g_colcnt%g_group_cnt ? 1 : 0)) * strlen(g_separator);
	
	/* TODO: implement patching for include style dump */
	/* skip the initial 'unsigned char xxx[] = {' */
	if (g_c_style == YES) {
		printf("Reverting include style dumps are not yet supported\n");
		return;
	//	while (fgetc(in) != '\n')
	//		;
	}

	offset_str = malloc(sizeof(*offset_str) * 16 + 1);
	p = malloc(sizeof(*p) * size + 1);
	ascii = malloc(g_colcnt+2+2+1);		// Read the ascii + the '|' and 2 spaces and newline
	while (1) {
		/* Skip the beginning offset part */
		if (g_plain_dump == NO && g_c_style == NO) {	
			o = offset_str;
			int cnt = 0;
			while ((cnt <= 16) && ((c = fgetc(in)) != ':')) {
				++cnt;
				*o = (char)c;
				if (c == EOF) {
					if (ferror(in)) {
						perror("patch: in");
						goto fail;
					} else {
						goto done;
					}
				}
				++o;
			}
			if (cnt > 16) {
				fprintf(stderr, "Invalid dump file\n");
				goto fail;
			}
			*o = '\0';
			/* Get offset from dump file that we are patching */
			dump_offset = strtol(offset_str, NULL, 16);
			/* Seek if necessary and copy the bytes until the offset (out_off) to tmpout */
			if (g_lenb  == -1)
				if (dump_offset < out_off)
					if (tmpout != stdout)
						out_off += copy_file(out, tmpout, out_off, dump_offset - out_off);
			if (dump_offset > out_off) {
				if (tmpout == stdout)
					for (int j = 0; j < dump_offset; ++j)
						putchar('\0');
				else
					fseek(tmpout, dump_offset, SEEK_SET);
				out_off = dump_offset;
			}
		}

		memset(p, 0, sizeof(*p)*size + 1);
		if ((r = fread(p, sizeof(*p), size, in)) == 0) {
			if (ferror(in)) {
				perror("patch: infile");
				goto fail;
			} else {
				if (g_lenb != -1)
					goto done;
				else	// -l flag has been passed, don't copy the remaining file
					goto finish;
			}
		}
		dump2bin(tmpout, p, ndigs, &r, &out_off);
		/* Have reverted the required number of bytes as passed in */
		if ((g_lenb != -1) && (out_off - g_seek) >= g_lenb)
			goto finish;
		/* Read the ascii + the '|'  2 spaces and newline */
		/* If it is a plain dump with no newlines, skip this */
		if (g_plain_dump == NO || g_newline == YES)
				if ((fgets(ascii, g_colcnt+2+2+1, in) == NULL))
					break;
	}
done:
	if (tmpout != stdout) {
		copy_file(out, tmpout, out_off, -1); /* Write any remaining */
		rename(template, outfile);
	}
finish:
	free(p);
	free(offset_str);
	free(ascii);
	exit(EXIT_SUCCESS);
fail:
	free(p);
	free(offset_str);
	free(ascii);
	exit(EXIT_FAILURE);
}

/*
 * dump2bin: Convert dump into binary.
 * 	*out:		File to write binary to.
 * 	*str:		String containing the dump.
 * 	ndigs:		number of digits for each byte according to base. (eg: 2 for hex)
 * 	*cnt:		size of str
 * 	*out_off:	number of bytes written to output file
 */
void dump2bin(FILE *out, char *str, char ndigs, int *cnt, off_t *out_off)
{
	char	*saveptr = NULL;
	char	*token = strtok_r(str, g_separator, &saveptr);
	char	tmp, c;
	int	lim;

	/* Set limit for 'for' loop */
	if (g_plain_dump == YES)
		lim = g_colcnt;
	else
		lim = MIN(g_colcnt, g_group_cnt);

	while (token != NULL) {
		for (int i = 1; i <= lim; ++i) {
			if ((g_lenb != -1) && ((*out_off-g_seek) >= g_lenb))
				return;

			tmp = token[i*ndigs];
			token[i*ndigs] = '\0';			// Add '\0' so we can call strtol on it
			c = strtol(token + (ndigs*(i-1)), NULL, g_base);
			*cnt -= ndigs;
			if (*cnt < 1) {	// no more to read
				token[i*ndigs] = tmp;
				break;
			}
			if (*out_off >= g_seek)
				fprintf(out, "%c", c);
			++*out_off;
			/* If next character is not a valid digit */
			if (is_dig(tmp, g_base) == 0)
				break;
			token[i*ndigs] = tmp;
		}
		if ((token = strtok_r(NULL, g_separator, &saveptr)))
			if (is_dig(*token, g_base) == 0)	// Not a valid digit
				break;
		*cnt -= strlen(g_separator);
	}
}

/*
 * copy_file:	Write size bytes from in into out starting from start.
 * 		If n < 0: write till EOF is reached.
 */
int copy_file(FILE *in, FILE *out, off_t start, off_t size)
{
	char	p[256];
	int	r;
	off_t	size1 = size;

	if (in == NULL || out == NULL)
		return 1;
	if (start > 0) {
		fseeko(in, start, SEEK_SET);
		fseeko(out, start, SEEK_SET);
	}

	while (size) {
		if ((r = (fread(p, sizeof(*p), sizeof(p), in))) == 0) {
			if (ferror(in)) {
				perror("copy_file:fread");
				exit(EXIT_FAILURE);
			} else {
				return 0;
			}
		}
		if (size > 0) {	/* Write limited number of bytes */
			r = fwrite(p, sizeof(*p), r > size ? size : r, out);
			size -= r;
		} else {	/* Write till EOF is reached. */
			if (fwrite(p, sizeof(*p), r, out) == 0)
				break;
		}
	}
	return size1 - size;
}

/*
 * is_dig: return true if given char is a valid digit of given base.
 */
int is_dig(char c, int base)
{
	if (base == HEX)
		return isxdigit(c);
	else if (base == OCT)
		return c >= '0' && c <= '7';
	else
		return c == '0' || c == '1';
}
