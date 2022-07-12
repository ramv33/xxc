#!/bin/bash
# Check the revert functionality of dux (-r) with various column.

EXE="dux"

if [ ! -e bin ] ; then
	echo "No file named 'bin'"
	echo "Create a file named 'bin' which will be used to do the bin"
	exit 1
fi

NORMAL_FAILED=0
for BASE in {'hex','oct','bin'} ; do		# Test for each base
	for i in $(seq 0 32) ; do		# Column counts
		for j in $(seq 0 $i) ; do	# Group counts
			./$EXE --$BASE -c $i -g $j 'bin' > dumpfile
			./$EXE --$BASE -c $i -g $j -r dumpfile > 'bin.out'
			diff 'bin' 'bin.out' &> /dev/null &&	# Test if reverted output is different from original file
				echo -e "N($BASE): \e[33m($i, $j)\n\e[1;32m[PASSED]\e[m" || {
					let '++NORMAL_FAILED';
					echo -ne "N: \e[33m($i, $j)\n\e[1;31m[FAILED]\e[m " ;
					wc -c 'bin' 'bin.out' | tr -s '\n' ' ';
					echo
				}
		done
	done
done

NORMAL_SEP_FAILED=0
for BASE in {'hex','oct','bin'} ; do
	for N_SEP in {'-','--','---','----'} ; do
		for i in $(seq 0 32) ; do
			for j in $(seq 0 $i) ; do
				./$EXE -S "$N_SEP" --$BASE -c $i -g $j 'bin' > dumpfile
				./$EXE -S "$N_SEP" --$BASE -c $i -g $j -r dumpfile > 'bin.out'
				diff 'bin' 'bin.out' &> /dev/null &&
					echo -e "N($BASE, '$N_SEP'): \e[33m($i, $j)\n\e[1;32m[PASSED]\e[m" || {
						let '++NORMAL_SEP_FAILED';
						echo -ne "N: \e[33m($i, $j)\n\e[1;31m[FAILED]\e[m " ;
						wc -c 'bin' 'bin.out' | tr -s '\n' ' ';
						echo
					}
			done
		done
	done
done


PLAIN_FAILED=0
for BASE in {'hex','oct','bin'} ; do
	for i in $(seq 0 32); do
		for j in $(seq 0 $i); do
			./$EXE --$BASE -c $i -g $j -p 'bin' > dumpfile
			./$EXE --$BASE -c $i -g $j -p -r dumpfile > 'bin.out'
			diff 'bin' 'bin.out' &> /dev/null &&
				echo -e "P($BASE): \e[33m($i, $j)\n\e[1;32m[PASSED]\e[m" || {
					let '++PLAIN_FAILED' ;
					echo -ne "P: \e[33m($i, $j)\n\e[1;31m[FAILED]\e[m " ;
					wc -c 'bin' 'bin.out' | tr -s '\n' ' ';
					echo
				}
		done
	done
done

PLAIN_SEP_FAILED=0
for BASE in {'hex','oct','bin'} ; do
	for P_SEP in {'-','--','---','----'} ; do
		for i in $(seq 0 32); do
			for j in $(seq 0 $i); do
				./$EXE -S "$P_SEP" --$BASE -c $i -g $j -p 'bin' > dumpfile
				./$EXE -S "$P_SEP" --$BASE -c $i -g $j -p -r dumpfile > 'bin.out'
				diff 'bin' 'bin.out' &> /dev/null &&
					echo -e "P($BASE, '$P_SEP'): \e[33m($i, $j)\n\e[1;32m[PASSED]\e[m" || {
						let '++PLAIN_SEP_FAILED' ;
						echo -ne "P: \e[33m($i, $j)\n\e[1;31m[FAILED]\e[m " ;
						wc -c 'bin' 'bin.out' | tr -s '\n' ' ';
						echo
					}
			done
		done
	done
done


PLAIN_NONEWLINE_FAILED=0
for BASE in {'hex','oct','bin'} ; do
	for P_SEP in {'-','--','---','----'} ; do
		for i in $(seq 0 32); do
			for j in $(seq 0 $i); do
				./$EXE --no-newline -S "$P_SEP" --$BASE -c $i -g $j -p 'bin' > dumpfile
				./$EXE --no-newline -S "$P_SEP" --$BASE -c $i -g $j -p -r dumpfile > 'bin.out'
				diff 'bin' 'bin.out' &> /dev/null &&
					echo -e "P($BASE, '$P_SEP', no-newline): \e[33m($i, $j)\n\e[1;32m[PASSED]\e[m" || {
						let '++PLAIN_NONEWLINE_FAILED' ;
						echo -ne "P: \e[33m($i, $j)\n\e[1;31m[FAILED]\e[m " ;
						wc -c 'bin' 'bin.out' | tr -s '\n' ' ';
						echo
					}
			done
		done
	done
done

echo "NORMAL"
if [ $NORMAL_FAILED -eq 0 ] ; then
	echo -e "N: \e[1;32mPASSED ALL TESTS SUCCESSFULLY :)\e[m"
else
	echo -e "N: \e[1;31mFAILED $NORMAL_FAILED TEST(S) :(\e[m"
fi

echo "NORMAL WITH SEPARATORS"
if [ $NORMAL_SEP_FAILED -eq 0 ] ; then
	echo -e "N: \e[1;32mPASSED ALL TESTS SUCCESSFULLY :)\e[m"
else
	echo -e "N: \e[1;31mFAILED $NORMAL_SEP_FAILED TEST(S) :(\e[m"
fi

echo "PLAIN"
if [ $PLAIN_FAILED -eq 0 ] ; then
	echo -e "P: \e[1;32mPASSED ALL TESTS SUCCESSFULLY :)\e[m"
else
	echo -e "P: \e[1;31mFAILED $PLAIN_FAILED TEST(S) :(\e[m"
fi

echo "PLAIN WITH SEPARATORS"
if [ $PLAIN_SEP_FAILED -eq 0 ] ; then
	echo -e "P: \e[1;32mPASSED ALL TESTS SUCCESSFULLY :)\e[m"
else
	echo -e "P: \e[1;31mFAILED $PLAIN_SEP_FAILED TEST(s) :(\e[m"
fi

echo "PLAIN WITH SEPARATORS AND NO NEWLINE"
if [ $PLAIN_NONEWLINE_FAILED -eq 0 ] ; then
	echo -e "P: \e[1;32mPASSED ALL TESTS SUCCESSFULLY :)\e[m"
else
	echo -e "P: \e[1;31mFAILED $PLAIN_NONEWLINE_FAILED TEST(s) :(\e[m"
fi

