/*
 * dump.c
 * ======
 *
 * Handle the dumping.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "xxc.h"
#include "dump.h"
#include "colors.h"

char digstr[] = "0123456789abcdef"; // Might need to change to uppercase

/*
 * upper_hex: Convert hexdigits to uppercase. Called in dux.c
 */
inline void upper_hex(void)
{
	for (int i = 10; i < 16; ++i)
		digstr[i] = toupper(digstr[i]);
}

/*
 * dump_stdin: read from stdin and dump.
 */
void dump_stdin(void)
{
	off_t	cnt = 0;
	char	*bytes, *p;
	int	ch;

	if (g_lenb == 0)
		return;

	bytes = malloc(sizeof(*bytes) * g_colcnt);
	p = bytes;

	if (g_seek < 0) {
		fprintf(stderr, "Cannot seek from end on stdin\n");
		goto fail;
	}

	if (g_c_style == YES)
		printf("unsigned char bytes[] = {\n");
	while ((ch = getchar()) != EOF) {
		++cnt;
		/* if lenb < 0, no limit */
		if (g_lenb > 0) {
			if (cnt > g_seek)
				--g_lenb;
		}
		/* start reading? */
		if (cnt > g_seek)
			*p++ = (char)(ch);
		if (((cnt-g_seek) % g_colcnt == 0) && (cnt > g_seek)) {
			/* don't print offset for plain dump and C-style include */
			if (g_plain_dump == YES && g_c_style == NO) {
				printf("%s", g_separator);
			} else if (g_c_style == YES) {
				printf(" 0x");
			} else {
				if (g_colors == YES)
					printf(COL_OFFSET);
				printf("%08lx", cnt-(p-bytes));
				if (g_colors == YES)
					printf(RESET);
				printf(":%s", g_separator);
			}

			dump(bytes, p - bytes);

			// Add ',' after last byte in row
			if (g_c_style == YES)
				printf(",\n");
			else if (g_newline == YES && ((p-bytes) == g_colcnt))
				putchar('\n');
			p = bytes;
		}
		if (g_lenb == 0)
			break;
	}
	/* Print any remaining */
	if (p != bytes && cnt > g_seek) {
		if (g_plain_dump == NO && g_c_style == NO) {
			if (g_colors == YES)
				printf(COL_OFFSET);
			printf("%08lx", ((cnt-g_seek)%g_colcnt) ?
				 	((cnt-g_seek)%g_colcnt) : g_colcnt);
			if (g_colors == YES)
				printf(RESET);
			printf(":%s", g_separator);
		}
		else if (g_c_style == YES)
			printf(" 0x");
		else	// plain dump
			printf("%s", g_separator);
		dump(bytes, p - bytes);
			
		if (g_newline == YES)
			putchar('\n');
	}
	if (g_c_style == YES)
		printf("};\n"
			"unsigned int bytes_len = %ld;\n", cnt-g_seek);
	free(bytes);
	exit(EXIT_SUCCESS);
fail:
	free(bytes);
	exit(EXIT_FAILURE);
}

/*
 * print_digs:	Print the given byte according to the given base.
 */

inline void print_digs(unsigned char ch)
{
	unsigned char b = 0;

	if (g_colors == YES) {
		if (ch == 0xff)
			printf(COL_FF);
		else if (ch == 0x00)
			printf(COL_ZERO);
		else if (isprint(ch))
			printf(COL_ASCII);
	}

	if (g_base == HEX) {
		b = ch & 0xf0;
		b >>= 4;
		putchar(digstr[b]);
		b = ch & 0x0f;
		putchar(digstr[b]);
	} else if (g_base == BIN) {
		b = 0;	// Use b as loop variable.
		while (b < 8) {
			putchar(digstr[(ch & 0x80) >> 7]);
			ch <<= 1;
			++b;
		}
	} else if (g_base == OCT) {
		b = ch & 0300;
		b >>= 6;
		putchar(digstr[b]);
		b = ch & 070;
		b >>= 3;
		putchar(digstr[b]);
		b = ch & 07;
		putchar(digstr[b]);
	}
	
	if (g_colors == YES)
		printf(RESET);
}

/*
 * dump: dump cnt bytes given in char *ch.
 */
void dump(char *ch, unsigned int cnt)
{
	char *p = ch;			/* To print the ascii */
	unsigned int cnt1 = cnt;

	while (cnt) {
		print_digs(*ch);
		++ch;
		--cnt;
		if (cnt == 0)
			break;
		/* Separate if we have written group_cnt characters */
		if (( ((cnt1-cnt) % g_group_cnt) == 0 ) && (cnt != 0))
			printf("%s", g_separator);
	}

	if (g_plain_dump == NO && g_c_style == NO && g_ascii == YES ) {
		cnt = cnt1;
		// Justify the last ascii column, if needed.
		while (cnt < g_colcnt) {
			if ((cnt) % (g_group_cnt) == 0) {
				int l = strlen(g_separator);
				while (l) {
					--l;
					putchar(' ');
				}
			}
			if (g_base == HEX)
				printf("  ");
			else if (g_base == OCT)
				printf("   ");
			else	/* base == BIN */
				printf("        ");
			++cnt;
		}
		if (g_colors == YES)
			printf(COL_ASCII_BAR);
		printf("  |");
		cnt = cnt1;
		while (cnt) {
			if (isprint(*p)) {
				if (g_colors == YES)
					printf(COL_ASCII);
				putchar(*p);
			} else {
				if (g_colors == YES)
					printf(RESET);
				putchar('.');
			}
			++p;
			--cnt;
		}
		if (g_colors == YES)
			printf(COL_ASCII_BAR);
		printf("|");
		if (g_colors == YES)
			printf(RESET);
	}
}

/*
 * dump_files: read files from arguments and dump.
 */
void dump_files(int argc, char **argv)
{
	off_t	cnt = 0;
	char	*bytes;
	char	*errmsg = argv[0]; 
	int	r = 0;
	FILE	*fp;

	bytes = malloc(sizeof(*bytes) * g_colcnt);

	argv += argc;	/* Point argv to first filename argument */
	while (*argv)  {
		if((fp = fopen(*argv, "r")) == NULL) {
			perror(errmsg);
			goto fail;
		}
		if (g_seek > 0) {
			fseeko(fp, 0, SEEK_END);
			if (g_seek > ftell(fp)) {	// seek value goes past EOF
				goto done;
			} else {
				if (fseeko(fp, g_seek, SEEK_SET) != 0) {
					perror(errmsg);
					goto fail;
				}
			}
			cnt += g_seek;
		} else if (g_seek < 0) {		// seek backwards from EOF?
			fseeko(fp, 0, SEEK_END);
			cnt = ftell(fp);	// get size of file
			if ((-g_seek) > cnt)	// seek past beginning of file?
				goto done;
			if (fseeko(fp, g_seek, SEEK_END) != 0) {
				perror(errmsg);
				goto fail;
			}
			cnt -= g_seek;
		}

		if (g_c_style == YES)
			printf("unsigned char %s[] = {\n", argv[0]);
		while ((r = fread(bytes, sizeof(*bytes), g_colcnt, fp)) == g_colcnt) {
			cnt += r;
			/* if g_lenb < 0, no limit */
			if (g_lenb > 0) {
				if ((g_lenb -= r) < 0) {
					r = g_colcnt + g_lenb;
					g_lenb = 0;
				}
			} else if (g_lenb == 0) {
				break;
			}
			/* Start dumping */
			if (g_plain_dump == NO && g_c_style == NO) {
				if (g_colors == YES)
					printf(COL_OFFSET);
				printf("%08lx", cnt-r);
				if (g_colors == YES)
					printf(RESET);
				printf(":%s", g_separator);
			} else if (g_c_style == YES) {
				printf(" 0x");
			} else { // plain dump
				printf("%s", g_separator);
			}
			dump(bytes, r);
			/* check if we reached EOF */
			if ((r = fgetc(fp)) != EOF) {
				if (g_c_style == YES)
					putchar(',');
				ungetc(r, fp);
			}
			if (g_newline == YES)
				putchar('\n');
		}
		cnt += r;
		// r < g_colcnt?
		if ((r != 0) && (g_lenb != 0)) {
			if (g_plain_dump == NO && g_c_style == NO) {
				if (g_colors == YES)
					printf(COL_OFFSET);
				printf("%08lx", cnt-r);
				if (g_colors == YES)
					printf(RESET);
				printf(":%s", g_separator);
			} else if (g_c_style == YES) {
				printf(" 0x");
			} else {	// plain dump
				printf("%s", g_separator);
			}
			dump(bytes, r);
			if (g_newline == YES)
				putchar('\n');
		} else if (r == 0) {
			if (ferror(fp)) {
				perror(errmsg);
				goto fail;
			}
		}
		if (g_c_style == YES)
			printf("};\n"
				"unsigned int %s_cnt = %ld;\n", argv[0], cnt-g_seek);
		fclose(fp);
		++argv;
	}
done:
	free(bytes);
	exit(EXIT_SUCCESS);
fail:
	free(bytes);
	exit(EXIT_FAILURE);
}
