/*
   Assembler for PDP-8.  Memory and object file creation.
*/

#include "asm8.h"


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* we want to assemble instructions.  We could assemble and output them
   all at once.  But we have a problem of forward references.  So we
   keep the (partially) assembled instructions in an array of them,
   essentially simulated memory.  That allows us to come back and
   fix them up when the forward reference is resolved.

   We need to know which memory locations are from assembled
   instructions, and which are just empty; so each memory location
   has a bit (defined/not defined).
*/

INST     memory[4096];
Boolean defined[4096];
Address entry_point = 0;


void Clear_Object_Code(void)
{
    int i;
    for (i = 0; i < 4096; i++)
        {
            defined[i] = FALSE;
        }
}

void Define_Object_Code(Address addr, INST inst, Boolean redefine)
{
    if (debug)
        fprintf(stderr, "object code: 0x%03X = 0x%03X\n", addr, inst);
    if (defined[addr] && !redefine)
        {
            fprintf(stderr, "redefined memory location: 0x%03X: was 0x%03X; new value 0x%03X\n",
                    addr, memory[addr], inst);
            number_of_errors += 1;
        }

    defined[addr] = TRUE;
    memory[addr] = inst;
}

INST Fetch_Object_Code(Address addr)
{
    INST inst;

    if (defined[addr])
        inst = memory[addr];
    else
        inst = 0;

    if (debug)
        fprintf(stderr, "read object code: 0x%03X = 0x%03X\n", addr, inst);
    return(inst);
}

void splitIntoTwoBytes(short org, char* twoByte) {
    twoByte[0] = (org >> 6) & 0x3F;
    twoByte[1] = org & 0x3F;
}

void Output_Object_Code(void) {
    char twoByte[2];
    int i = 0;
    int j;
    fputs("OBJ8", output);
    splitIntoTwoBytes(entry_point, twoByte);
    fwrite(twoByte, 1, 2, output);
    while (i < 4096) {
        while (i < 4096 && !defined[i]) {
            ++i;
        }
        for (j = i; j < 4096 && 2 * (j - i) + 3 < 256; ++j) {
            if (!defined[j]) {
                break;
            }
        }
        if (i == j) {
            continue;
        }
        fputc(2 * (j - i) + 3, output);
        splitIntoTwoBytes(i, twoByte);
        fwrite(twoByte, 1, 2, output);
        while (i < j) {
            splitIntoTwoBytes(memory[i], twoByte);
            fwrite(twoByte, 1, 2, output);
            ++i;
        }
    }
}

