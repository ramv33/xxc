#
# xxc Makefile
#

CFLAGS=-g -Wall -Wextra 

xxc: dump.o revert.o
xxc.o: xxc.h dump.h revert.h
revert.o: revert.h xxc.h
dump.o: dump.h xxc.h

.PHONY: clean

clean:
	rm -fv *.o test.out test xxc
