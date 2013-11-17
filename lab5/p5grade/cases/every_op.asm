one,	1
two,	2
x,	88
a,	65
addr_of_exit, 255
trash, 0
a_letter, 'c'

program,	0
		CLA
		TAD one
		AND two
		SZA
		TAD x
		IOT 4 0		/ if x is printed, something's wrong
		ISZ a
		RTL
		CMA
		RAR
		RTR
		CML
		SNL
		NOP
		DCA trash
		SZA RSS
		CML
		TAD x
		SNL
		IOT 4 0		/ if x is printed, something's wrong
		JMS do_it
		CLA
		IAC
		RAL
		JMP I addr_of_exit

do_it, 0
		OSR
		CLA
		TAD x
		SMA RSS
		IOT 4,0		/ if x is printed, something's wrong
		IOT 3,0
		IOT 3 0
		JMS I do_it

ORIG 255
exit, 0
	HLT
END program
