/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function.
     The max operator count is checked by dlc. Note that '=' is not
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */


#endif
/*
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  // Right shift 8 * n bits and mask the least significant byte.
  return (x >> (n << 3)) & 0xFF;
}
/*
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int logicalShift(int x, int n) {
  // Do arithmetic right shift and then mask the original bits.
  return (x >> n) & ~(((0x80 << 24) >> n) << 1);
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int temp = (0x33 << 24) + (0x33 << 16) + (0x33 << 8) + 0x33; // Auxillary mask.
  // Count bits in pairs of two.
  x = x + ~((x >> 1) & ((0x55 << 24) + (0x55 << 16) + (0x55 << 8) + 0x55)) + 1;
  // Count bits in pairs of four.
  x = (x & temp) + ((x >> 2) & temp);
  // Count bits in pairs of eight.
  x = x + (x >> 4);
  // Mask off intermediate count.
  x = x & ((0x0F << 24) + (0x0F << 16) + (0x0F << 8) + 0x0F);
  // Sum up and mask to get the answer.
  x = (x >> 24) + (x >> 16) + (x >> 8) + x;
  x = x & 0xFF;
  return x;
}
/*
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int bang(int x) {
  // (x | (-x)) generates a leading zero only for one.
  return (~(x | (~x + 1)) >> 31) & 0x01;
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  // Returns 0x80000000.
  return 0x80 << 24;
}
/*
 * fitsBits - return 1 if x can be represented as an
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  // Number of left shifts to align the most significant bits.
  int move = 33 + ~n;
  // Shift back and forth, should be the same number.
  return !(((x << move) >> move) ^ x) ;
}
/*
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  // Arithmetically round up when it is a negtive number. In effect rounding toward zero.
  int round = ((0x01 << n) + ~0x00) & (x >> 31);
  return (x + round) >> n;
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  // Easy.
  return ~x + 1;
}
/*
 * isPositive - return 1 if x > 0, return 0 otherwise
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  // Be aware when x equals zero.
  return !((x >> 31) & 0x01) ^ (!x);
}
/*
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int leastBitPos(int x) {
  // Easy.
  return (x & (~x + 1));
}
/*
 * trueFiveEighths - multiplies by 5/8 rounding toward 0,
 *  avoiding errors due to overflow
 *  Examples: trueFiveEighths(11) = 6
 *            trueFiveEighths(-9) = -5
 *            trueFiveEighths(0x30000000) = 0x1E000000 (no overflow)
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 4
 */
int trueFiveEighths(int x) {
  // Rounding toward zero.
  int round = (x & (x >> 2) & 0x01) + (((x ^ (x >> 2)) | (x >> 1) | x) & (x >> 31) & 0x01);
  return (x >> 3) + (x >> 1) + round;
}
/*
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1,
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
  int posAndNeg = x ^ y; // MSB equls 1 only when one is positive and the other is negative.
  int bothPosNoOverflow = x | y | (x + y); // MSB equals 0 only when both are positive and no overflow happens.
  int bothNegNoOverflow = x & y & (x + y); // MSB equals 1 only when both are negtive and no overflow happens.
  return ((posAndNeg | ~bothPosNoOverflow | bothNegNoOverflow) >> 31) & 0x01;
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  int occur;
  int count;
  // Check in 16 bits scale.
  occur = !!(x >> 16);
  count = occur << 4;
  // Mask x to be the corresponding part.
  x = ((~occur + 1) & (x >> 16)) | (~(~occur + 1) & x);
  // Check in 8 bits scale.
  occur = !!(x >> 8);
  count = count + (occur << 3);
  // Mask x to be the corresponding part.
  x = ((~occur + 1) & (x >> 8)) | (~(~occur + 1) & x);
  // Check in 4 bits scale.
  occur = !!(x >> 4);
  count = count + (occur << 2);
  // Mask x to be the corresponding part.
  x = ((~occur + 1) & (x >> 4)) | (~(~occur + 1) & x);
  // Check in 2 bits scale.
  occur = !!(x >> 2);
  count = count + (occur << 1);
  // Mask x to be the corresponding part.
  x = ((~occur + 1) & (x >> 2)) | (~(~occur + 1) & x);
  // Check in 1 bit scale.
  occur = !!(x >> 1);
  count = count + occur;
  return count;
}
/*
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  unsigned sign = 0;
  unsigned exp = 0;
  unsigned round = 0;
  unsigned tmp = x;
  unsigned frac;
  // Special case.
  if (!x) {
    return 0;
  }
  // Set sign bit.
  if (x < 0) {
    x = -x;
    sign = 0x80000000;
  }
  // Shift to get the exponential.
  while (1) {
    tmp = x;
    x = x << 1;
    exp = exp + 1;
    if (tmp & 0x80000000) {
      break;
    }
  }
  // Typical rounding.
  if (0x0100 < (x & 0x01FF)) {
    round = 1;
  }
  // Rounding to even.
  if (0x0300 == (x & 0x03FF)) {
    round = 1;
  }
  frac = x;
  return sign + (frac >> 9) + ((159 - exp) << 23) + round;
}
/*
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  // Case infinity or NaN.
  if ((uf & 0x7F800000) == 0x7F800000 || !(uf & 0x7FFFFFFF)) {
    return uf;
  }
  // Overflow to infinity.
  if ((uf & 0x7F800000) == 0x7F000000) {
    return (uf & 0x80000000) | 0x7F800000;
  }
  // Normalized input, no overflow.
  if (uf & 0x7F800000) {
    return uf + 0x00800000;
  }
  // Denormalized input.
  return (uf & 0x80000000) | uf << 1;
}

