/*
 * dump.h
 */

/*
 * dump: dump the bytes given in ch.
 */
void dump(char *ch, unsigned int cnt);

/*
 * dump_files: dump files given as arguments.
 */
void dump_files(int argc, char **argv);

/*
 * dump_stdin: read from stdin and dump.
 */
void dump_stdin(void);

/*
 * print_digs: Dump the bytes with the given base.
 */
void print_digs(unsigned char ch);

/* 
 * upper_hex: convert hexdigits to uppercase.
 */
void upper_hex(void);
