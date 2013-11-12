ascii_nl, 10 /This is space so your terminal ends with a newline
test, 400
  ORIG 4092

PARTEVILHERE, 54
OTHERPARTEVILLURKS, 54 /Random comment for no reason! /Oh noes?! /Another?
FINALPART, 54
		ORIG 104 /Comments, they are everywhere. / //
LABELTOEVIL, 4092
OTHERLABEL, 4093
ANDANOTHER, 4094
	ORIG 100
whee,,,: /Make sure you handle that colon////////
	ORIG 13

START, /Why is a comment here?
	TAD I LABELTOEVIL
	DCA 42
	TAD I OTHERLABEL
	DCA 43
	TAD I ANDANOTHER
	DCA 44
	JMP I test

	ORIG 400
OVERHERE,
	
	TAD 42
	iot 4,,, ,,, 0
	cla
	TAD 43
	iot 4 ,,, 0
	cla
		TAD 44
	 iot 4, ,, , 0
	  cla
	 TAD ascii_nl
			 iot 4 0
	 cla
	sza rss  /Better hope you have this right in your PDP8.  Remove rss if you dare.
	hlt
	jmp start


parray, 0
		0
		0
		0
		0        / array of 5 digits.
arradd, parray   / address of print digit array
arrptr, parray   / pointer into array for next digit
tempn,  0        / space to save working value of n
ascii_zero,  '0'
ten,    10

printn,  0
		dca tempn		/ save n
		tad ten
		dca X           / to divide by 10
		tad tempn		/ reload n
plo,	jms div 		/ divide A by 10; quotient in A; remainder in R
		dca tempn       / n = n / 10
        tad r			/ load remainder
		dca i arrptr	/ save digit
		isz arrptr
		tad tempn
		sza				/ are we done? is tempn = 0
		jmp plo
		/ we have broken n into its digits; now print them
		/ load pointer and decrement it
ploo,	cla cma			/ -1 -> A
		tad arrptr
		dca arrptr
		tad i arrptr    / get the digit
		tad ascii_zero  / change to ascii
		iot 4 0			/ output to stdout
		cla
		tad arrptr
		cma iac			/ -arrptr
		tad arradd		/ compare with base address
		sza
		jmp ploo
		jmp i printn     / we done.

/ divide
/ divide A register by the value in X

q,      0    / quotient
r,      0    / remainder

X,      0
div,    0
		dca  r
		dca  q    / set q = 0
		tad  X
		sza rss cla / make sure X is not zero
		jmp I div   / divide by zero, return zero

dloop,  tad  X    /  divide by X
		cma iac   / -X
		tad  r    /  r - X
		spa       / if r > X skip
		jmp  dfini
		isz  q    / q = q + 1 (for the X we just subtracted)
		dca  r    / r = r - X
        jmp  dloop
dfini,  cla
		tad q			/ put quotient in a
		jmp i div     / return 
		END start
	
	


