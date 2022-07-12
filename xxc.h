/*
 * hexdump.h
 */

#ifndef ISPOW_OF2
#define ISPOW_OF2(x)	\
	(x) && (!((x) & ((x)-1)))
#endif

#ifndef MIN
#define MIN(x, y)	\
	(x) < (y) ? (x) : (y)
#endif

/* Values for options */
// Endianness options
#define BIG	0
#define LITTLE	1
// Base options
#define	BIN	0x2
#define OCT	0x8
#define HEX	0x10
// Yes or no options
#define	YES	1
#define	NO	0

/* Options */
extern off_t g_lenb;		 	// Dump len bytes starting at seek 
extern off_t g_seek;		 	// Start at seek bytes
extern long g_colcnt;			// Column count
extern long g_group_cnt;		// Octets per group
extern char *g_separator;	 	// To separate groups
extern int g_plain_dump;	 	// Just dump it, no decorations.
extern int g_endian;			// Endianness
extern int g_base;			// Number base to use
extern int g_reverse;			// Convert dump to binary
extern int g_ucase;			// Use uppercase hexadecimal digits
extern int g_autoskip;			// Skip repeating lines
extern int g_ascii;			// Show corresponding ascii characters
extern int g_newline;			// Use newlines after columns
extern int g_c_style;			// Output in C include file style
extern int g_colors;			// Colors!!
