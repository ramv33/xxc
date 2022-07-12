/*
 * revert.h
 */

/*
 * patch:	Read dump from infile and patch it to binary as outfile.
 */
void patch(char *infile, char *outfile);

/*
 * dump2bin: Convert dump into binary.
 * 	*out:	File to write binary to.
 * 	*str:	String containing the dump.
 * 	ndigs:	number of digits for each byte according to base.
 * 	*cnt:
 * 	*out_off:
 */
void dump2bin(FILE *out, char *str, char ndigs, int *cnt, off_t *out_off);

/*
 * copy_file:	Write size bytes from f1 to f2 starting from start.
 *		If size < 0: write till EOF is reached.
 */
int copy_file(FILE *f1, FILE *f2, off_t start, off_t size);

/*
 * is_dig: return true if c is valid digit of given base.
 */
int is_dig(char c, int base);
