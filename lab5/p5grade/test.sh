#!/bin/bash

source "tf.sh"

for i in `ls $CASEDIR/*.asm`
do
	base="${i%.*}"
	corename="${base#$CASEDIR/}"
	if [ -f $base.in ]
	then
		INPUT="<$base.in"
	else
		INPUT=''
	fi
	cp $base.asm $TMPDIR/$corename.asm
	testfunc $corename  "./$PROG $TMPDIR/$corename.asm" "$TMPDIR/$corename" "$base.out" "validator/validator"
done

echo -e '\E[;32m'"\033[1m $TESTPASS OUT OF $TESTCASE TEST(S) PASSED\033[0m" 

let "SCORE=10-WA"

exit $SCORE

# vim: tw=0
