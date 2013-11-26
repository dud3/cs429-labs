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

__dir0="$TMPDIR/lfu_result"
__dir1="$TMPDIR/lru_result"
__dir2="$TMPDIR/lruv_result"
__dir3="$TMPDIR/test1_result"
__dir4="$TMPDIR/test2_result"
__dir5="$TMPDIR/test_result"
__dir6="$TMPDIR/lruvt_result"
__dir7="$TMPDIR/lfu_fifo_all_result"

if [ ! -x $PROG ]
then
	chmod +x $PROG
fi

if [ ! -d $TMPDIR ]
then
	mkdir $TMPDIR
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

#7
if [ ! -d $__dir7 ]
then
	mkdir $__dir7
fi

rm -rf "$__dir0"/*
rm -rf "$__dir1"/*
rm -rf "$__dir2"/*
rm -rf "$__dir3"/*
rm -rf "$__dir4"/*
rm -rf "$__dir5"/*
rm -rf "$__dir6"/*
rm -rf "$__dir7"/*


#Array of def
#echo ${#arr[@]} # will echo number of elements in array
arrayDef=(def/*)
arrayCases=(cases/*)


#Run LFU, LRU, LRUV, TEST1 TEST
#Pipe user output to tempdir/{subdir}/{0..5.out}
i=0;	
for f in "${arrayCases[@]}"; do
	./$PROG "def/lfu_definition" "$f" 1>&"$__dir0"/$i.out
	./$PROG "def/lru_definition" "$f" 1>&"$__dir1"/$i.out
	./$PROG "def/lruv_definition" "$f" 1>&"$__dir2"/$i.out
	./$PROG "def/test1_definition" "$f" 1>&"$__dir3"/$i.out
	./$PROG "def/test2_definition" "$f" 1>&"$__dir4"/$i.out
	./$PROG "def/test_definition" "$f" 1>&"$__dir5"/$i.out
	./$PROG "def/lruvt_definition" "$f" 1>&"$__dir6"/$i.out
	./$PROG "def/evil_definition" "$f" 1>&"$__dir7"/$i.out
	i=$((i+1))
done


#Expected Result
arrayResult0=(expected_result/0_lfu/*)
arrayResult1=(expected_result/1_lru/*)
arrayResult2=(expected_result/2_lruv/*)
arrayResult3=(expected_result/3_test1/*)
arrayResult4=(expected_result/4_test2/*)
arrayResult5=(expected_result/5_test/*)
arrayResult6=(expected_result/6_lruvt/*)
arrayResult7=(expected_result/7_fifo_lfu_all/*)

#User Output
arrayUser0=(tmpdir/lfu_result/*)
arrayUser1=(tmpdir/lru_result/*)
arrayUser2=(tmpdir/lruv_result/*)
arrayUser3=(tmpdir/test1_result/*)
arrayUser4=(tmpdir/test2_result/*)
arrayUser5=(tmpdir/test_result/*)
arrayUser6=(tmpdir/lruvt_result/*)
arrayUser7=(tmpdir/lfu_fifo_all_result/*)

totalPassed=0;
index=0;
#Compare and check difference
#0
echo -e '\E[;35m'"\033[1m0 LFU definition\033[0m"
for ((i = 0; i < ${#arrayResult0[@]} && i < ${#arrayUser0[@]}; i++));
do
	if diff ${arrayResult0[$i]} ${arrayUser0[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult0[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult0[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo

#1
echo -e '\E[;35m'"\033[1m1 LRU definition\033[0m"
for ((i = 0; i < ${#arrayResult1[@]} && i < ${#arrayUser1[@]}; i++));
do
	if diff ${arrayResult1[$i]} ${arrayUser1[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult1[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult1[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo

#2
echo -e '\E[;35m'"\033[1m2 LRUV definition\033[0m"
for ((i = 0; i < ${#arrayResult2[@]} && i < ${#arrayUser2[@]}; i++));
do
	if diff ${arrayResult2[$i]} ${arrayUser2[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult2[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult2[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo

#3
echo -e '\E[;35m'"\033[1m3 TEST1 definition\033[0m"
for ((i = 0; i < ${#arrayResult3[@]} && i < ${#arrayUser3[@]}; i++));
do
	if diff ${arrayResult3[$i]} ${arrayUser3[$i]};
	then 
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult3[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayUResult3[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo

#4
echo -e '\E[;35m'"\033[1m4 TEST2 definition\033[0m"
for ((i = 0; i < ${#arrayResult4[@]} && i < ${#arrayUser4[@]}; i++));
do
	if diff ${arrayResult4[$i]} ${arrayUser4[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult4[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult4[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo

#5
echo -e '\E[;35m'"\033[1m5 TEST definition\033[0m"
for ((i = 0; i < ${#arrayResult5[@]} && i < ${#arrayUser5[@]}; i++));
do
	if diff ${arrayResult5[$i]} ${arrayUser5[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult5[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult5[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo


#6
echo -e '\E[;35m'"\033[1m6 LRUVT definition\033[0m"
for ((i = 0; i < ${#arrayResult6[@]} && i < ${#arrayUser6[@]}; i++));
do
    if diff ${arrayResult6[$i]} ${arrayUser6[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult6[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult6[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo


#7
echo -e '\E[;35m'"\033[1m6 LFU FIFO definition\033[0m"
echo -e '\E[;35m'"\033[1m  --victimCache implementation test based on Piazza @1147\033[0m"
for ((i = 0; i < ${#arrayResult7[@]} && i < ${#arrayUser7[@]}; i++));
do
    if diff ${arrayResult7[$i]} ${arrayUser7[$i]}; 
    then
    	echo -e '\E[;32m'"\033[1m.$i ${arrayResult7[i]} : PASSED! No diff\033[0m"
    	totalPassed=$((totalPassed+1))
	else
		echo -e '\E[;31m'"\033[1m.$i ${arrayResult7[i]} : FAILED! Nasty Output\033[0m"
    	
	fi
done
echo


echo Statistics: $totalPassed out of 48
