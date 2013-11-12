#!/bin/bash

CASEDIR=cases
TMPDIR=tmpdir
PROG=asm8
TESTCASE=0
TESTFAIL=0


function testcore {

	cmd=$1
	ref=$2
	out=$3
	cmptool=$4
	blank=$CASEDIR/blank
	log=$TMPDIR/$TESTCASE.log
	stdout=$TMPDIR/$TESTCASE.stdout

	bash -c "$cmd > $stdout 2> $log"

	CMP=`$cmptool $ref $out`
	if [ $? != 0 ]
	then
		FAILED=1
		echo "error: $CMP"
	fi

	LOG=`$cmptool --quiet $blank $log`
	if [ $? != 0 ]
	then
		ERROR=1
	fi

	let "TESTCASE=TESTCASE+1"
	if [ $FAILED -ne 0 ]
	then
		let "TESTFAIL=TESTFAIL+1"
	fi
}

function testfunc {
	FAILED=0
	ERROR=0

	echo -n "Testing $1..."
	"${@:2}"
	if [ $FAILED -eq 0 ]
	then
		echo -en '...\E[;32m'"\033[1mPASS\033[0m"
		if [ $ERROR -ne 0 ]
		then
			echo -e '...\E[1;33m'"\033 WARNING: ERRORS IN LOG FILE\033[0m"
		else
			echo;
		fi
	else
		echo -e '...\E[;31m'"\033[1mTest \"$1\" FAILED\033[0m"
	fi

}

if [ ! -f $PROG ]
then
	echo "Could not find $PROG. Compile source and place output in this directory.";
	exit
fi

if [ ! -x $PROG ]
then
	chmod +x $PROG
fi

if [ ! -d $TMPDIR ]
then
	mkdir $TMPDIR
fi

rm -f $TMPDIR/*
rm -f $CASEDIR/*.out

for i in `ls $CASEDIR/*.asm`
do
	base="${i%.*}"
	testfunc $base testcore "./$PROG $base.asm" "$base.obj"  "$base.out" "cmp"
done

let "TESTPASS=TESTCASE-TESTFAIL"
if [ $TESTPASS -eq $TESTCASE ]
then
	echo -e '\E[;32m'"\033[1mALL $TESTCASE TESTS PASSED \033[0m"
else
	echo -e '\E[;31m'"\033[1m$TESTFAIL TEST(S) FAILED OUT OF $TESTCASE TEST(S)\033[0m"
fi
exit	
