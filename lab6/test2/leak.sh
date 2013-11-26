#!/bin/bash

CASEDIR=cases
TMPDIR=tmpdir
PROG=cachesim
TESTCASE=0
TESTFAIL=0
shopt -s nullglob

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


if [ ! -f $PROG ]
then
	echo "Could not find $PROG. Compile source and place output in this directory.";
	exit
fi

__dir0="log4leak/lfu_leak"
__dir1="log4leak/lru_leak"
__dir2="log4leak/lruv_leak"
__dir3="log4leak/test1_leak"
__dir4="log4leak/test2_leak"
__dir5="log4leak/test_leak"
__dir6="log4leak/lruvt_leak"

if [ ! -x $PROG ]
then
	chmod +x $PROG
fi

if [ ! -d "log4leak" ]
then
	mkdir "log4leak"
fi

#0
if [ ! -d $__dir0 ]
then
	mkdir -p $__dir0
fi

#1
if [ ! -d $__dir1 ]
then
	mkdir $__dir1
fi

#2
if [ ! -d $__dir2 ]
then
	mkdir $__dir2
fi

#3
if [ ! -d $__dir3 ]
then
	mkdir $__dir3
fi

#4
if [ ! -d $__dir4 ]
then
	mkdir $__dir4
fi

#5
if [ ! -d $__dir5 ]
then
	mkdir $__dir5
fi

#6
if [ ! -d $__dir6 ]
then
	mkdir $__dir6
fi

rm -rf "$__dir0"/*
rm -rf "$__dir1"/*
rm -rf "$__dir2"/*
rm -rf "$__dir3"/*
rm -rf "$__dir4"/*
rm -rf "$__dir5"/*
rm -rf "$__dir6"/*


#Array of def
#echo ${#arr[@]} # will echo number of elements in array
arrayDef=(def/*)
arrayCases=(cases/test_trace0)


#Run LFU, LRU, LRUV, TEST1 Leaktest
#Pipe user output to log4leak/{subdir}/{0..5.out}
cmd0=./$PROG "def/lfu_definition" "$f" 1>/dev/null
cmd1=./$PROG "def/lru_definition" "$f" 1>&/dev/null
cmd2=./$PROG "def/lruv_definition" "$f" 1>&/dev/null
cmd3=./$PROG "def/test1_definition" "$f" 1>&/dev/null
cmd4=./$PROG "def/test2_definition" "$f" 1>&/dev/null
cmd5=./$PROG "def/test_definition" "$f" 1>&/dev/null
cmd6=./$PROG "def/lruvt_definition" "$f" 1>&/dev/null

i=0;
echo "Setting up... be patient"
for f in "${arrayCases[@]}"; do
	touch "$__dir0"/$i.out
	touch "$__dir1"/$i.out
	touch "$__dir2"/$i.out
	touch "$__dir3"/$i.out
	touch "$__dir4"/$i.out
	touch "$__dir5"/$i.out
	touch "$__dir6"/$i.out
	i=$((i+1))
done
echo -ne '#                         (08%)\r'
sleep 1

i=0;
for f in "${arrayCases[@]}"; do
	chmod +x "$__dir0"/$i.out
	chmod +x "$__dir1"/$i.out
	chmod +x "$__dir2"/$i.out
	echo -ne '##                        (14%)\r'

	chmod +x "$__dir3"/$i.out
	chmod +x "$__dir4"/$i.out
	chmod +x "$__dir5"/$i.out
	chmod +x "$__dir6"/$i.out
	echo -ne '####                      (22%)\r'
	i=$((i+1))
done

i=0;
for f in "${arrayCases[@]}"; do
	valgrind --log-file="$__dir0"/$i.out "./$PROG" "def/lfu_definition" "$f" 1>/dev/null
	echo -ne '#####                     (28%)\r'
	
	valgrind --log-file="$__dir1"/$i.out "./$PROG" "def/lru_definition" "$f" 1>&/dev/null
	echo -ne '######                    (35%)\r'
	
	valgrind --log-file="$__dir2"/$i.out "./$PROG" "def/lru_definition" "$f" 1>&/dev/null
	echo -ne '#########                 (41%)\r'
	
	valgrind --log-file="$__dir3"/$i.out "./$PROG" "def/test1_definition" "$f" 1>&/dev/null
	echo -ne '##############            (57%)\r'
	
	valgrind --log-file="$__dir4"/$i.out "./$PROG" "def/test2_definition" "$f" 1>&/dev/null
	echo -ne '#################         (65%)\r'

	valgrind --log-file="$__dir5"/$i.out "./$PROG" "def/test_definition" "$f" 1>&/dev/null
	echo -ne '#####################     (83%)\r'

	valgrind --log-file="$__dir6"/$i.out "./$PROG" "def/lruvt_definition" "$f" 1>&/dev/null
	echo -ne '#########################(100%)\r'

	i=$((i+1))
done
echo -ne '\n'


#User Output
arrayUser0=(log4leak/lfu_leak/*)
arrayUser1=(log4leak/lru_leak/*)
arrayUser2=(log4leak/lruv_leak/*)
arrayUser3=(log4leak/test1_leak/*)
arrayUser4=(log4leak/test2_leak/*)
arrayUser5=(log4leak/test_leak/*)
arrayUser6=(log4leak/lruvt_leak/*)

totalLeak=0;
#Check All is fread or not
#0
i=0;
echo -e '\E[;35m'"\033[1m0 LFU leak\033[0m"
for f in "${arrayUser0[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser0[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser0[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser0[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo

#1
i=0;
echo -e '\E[;35m'"\033[1m1 LRU leak\033[0m"
for f in "${arrayUser1[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser1[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser1[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser1[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo

#2
i=0;
echo -e '\E[;35m'"\033[1m2 LRUV leak\033[0m"
for f in "${arrayUser2[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser2[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser2[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser2[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo

#3
i=0;
echo -e '\E[;35m'"\033[1m3 TEST1 leak\033[0m"
for f in "${arrayUser3[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser3[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser3[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser3[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo

#4
i=0;
echo -e '\E[;35m'"\033[1m4 TEST2 leak\033[0m"
for f in "${arrayUser4[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser4[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser4[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser4[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo

#5
i=0;
echo -e '\E[;35m'"\033[1m5 TEST leak\033[0m"
for f in "${arrayUser5[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser5[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser5[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser5[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo


#6
i=0;
echo -e '\E[;35m'"\033[1m6 LRUVT leak\033[0m"
for f in "${arrayUser6[@]}";
do
	if grep -q "All heap blocks were freed" ${arrayUser6[$i]};
	then
    	# code if found
    	echo -e '\E[;32m'"\033[1m.$i ${arrayUser6[i]} : PASSED! No leak\033[0m"
	else
    	# code if not found
    	echo -e '\E[;33m'"\033[1m.$i ${arrayUser6[i]} : LEAKED!\033[0m"
    	totalLeak=$((totalLeak+1))
	fi
	i=$((i+1))
done
echo


echo Statistics: $totalLeak "out of 7 of leak(s)"
if [ $totalLeak = 0 ];
then
	echo -e '\E[;32m'"\033[1mALL PASSED! No leak\033[0m"
else
	echo -e '\E[;33m'"\033[1mSome Memory Leak!!! Please check log4text and debug with valgrind\033[0m"
fi
