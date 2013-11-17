#!/bin/bash

CURRENT=`pwd`
CASEDIR=cases
SOURCE="$1"
TITLE=Program5
PROG=asm8
TMPDIR=tmpdir
TESTCASE=0
TESTPASS=0
TESTFAIL=0
TESTLEAK=0
WA=0
WV=0

function testcore {
	cmdline=$1
	coreout=$2
	referenceout=$3
	difftool=$4

	bash -c "timeout -s 9 15s $cmdline >/dev/null 2>/dev/null"
	RET=$?

	if [ $RET == 139 ]
	then
		SEGF=1
		FAILED=1
	fi
	
	if [ $RET == 137 ]
	then
		SEGF=1
		FAILED=1
		TLE=1
	fi

	if [ -f $coreout.out ]
	then
		out=$coreout.out
	else
		out=$coreout.obj
	fi
	$difftool $out $referenceout >$out.log
	RET=$?

	if [ $RET != 0 ]
	then 
		ANSFAILED=1
		VERBFAILED=1
		echo "See $out.log for diagnose"
	fi
}

function testfunc {
	ANSFAILED=0
	VERBFAILED=0
	SEGF=0
	TLE=0
	echo -n "Testing $1""..."
	testcore "${@:2}"
	let "TESTCASE=TESTCASE+1"
	let "WA=WA+ANSFAILED"
	if [ $VERBFAILED -eq 0 ]
	then
		echo -e '...\E[;32m'"\033[1mPASS\033[0m"
		let "TESTPASS=TESTPASS+1"
	elif [ $ANSFAILED -eq 0 ]
	then
		echo -e '...\E[1;33m'"\033[1mVERBOSE\033[0m"
	elif [ $TLE -ne 0 ]
	then
		echo -e '...\E[;31m'"\033[1m\"$1\"TIME LIMIT EXCEEDED\033[0m"
	elif [ $SEGF -ne 0 ]
	then
		echo -e '...\E[;31m'"\033[1m\"$1\"SEGFAULT\033[0m"
	else
		echo -e '...\E[;31m'"\033[1m\"$1\"WRONG OUTPUT\033[0m"
	fi
}

if [ ! -f $PROG ]
then
	cd $SOURCE
	rm *.o $PROG
	make $PROG
	cp $PROG $CURRENT
	cd $CURRENT
fi

if [ ! -f $PROG ]
then
	echo "[$TITLE] Could not build $PROG at $SOURCE. Please check your source code or Makefile";
	exit
fi

if [ ! -d $TMPDIR ]
then
	mkdir $TMPDIR 
fi

rm -f $TMPDIR/*

# vim: tw=0
