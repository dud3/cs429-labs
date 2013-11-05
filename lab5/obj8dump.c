
/* obj8dump -- print an OBJ8 object file to check it's correctness */

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

#include <stdio.h>
#include <stdlib.h>

typedef short Boolean;
#define TRUE 1
#define FALSE 0

Boolean debug = FALSE;

typedef char *STRING;

#define CAST(t,e) ((t)(e))
#define TYPED_MALLOC(t) CAST(t*, malloc(sizeof(t)))

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


/* print representation of a character for debugging */
char   *printrep (unsigned short  c)
{
    static char pr[8];

    if (c < 32)
        {
            /* control characters */
            pr[0] = '^';
            pr[1] = c + 64;
            pr[2] = '\0';
        }
    else if (c < 127)
        {
            /* printing characters */
            pr[0] = c;
            pr[1] = '\0';
        }
    else if (c == 127)
        return("<del>");
    else if (c <= 0377)
        {
            /* upper 128 codes from 128 to 255;  print as \ooo - octal  */
            pr[0] = '\\';
            pr[3] = '0' + (c & 7);
            c = c >> 3;
            pr[2] = '0' + (c & 7);
            c = c >> 3;
            pr[1] = '0' + (c & 3);
            pr[4] = '\0';
        }
    else
        {
            /* very large number -- print as 0xffff - 4 digit hex */
            (void)sprintf(pr, "0x%04x", c);
        }
    return(pr);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int get2(FILE *input)
{
    int c1 = getc(input);
    int c2 = getc(input);
    if (debug) fprintf(stderr, "read two bytes: 0x%X, 0x%X\n", c1, c2);

    if ((c1 == EOF) || (c2 == EOF))
        {
            fprintf(stderr, "Premature EOF\n");
            exit(1);
        }

    if (c1 & (~0x3F)) fprintf(stderr, "Extra high order bits for 0x%X\n", c1);
    if (c2 & (~0x3F)) fprintf(stderr, "Extra high order bits for 0x%X\n", c2);
    int n = ((c1 & 0x3F) << 6) | (c2 & 0x3F);
    return(n);
}


void Read_and_Dump_PDP8_Object_File(FILE *input)
{
    int c1 = getc(input);
    int c2 = getc(input);
    int c3 = getc(input);
    int c4 = getc(input);
    if (debug) fprintf(stderr, "read four bytes: 0x%X, 0x%X, 0x%X, 0x%X\n", c1, c2, c3, c4);

    if ((c1 != 'O') || (c2 != 'B') || (c3 != 'J') || (c4 != '8'))
        {
            fprintf(stderr, "First four bytes are not OBJ8: ");
            fprintf(stderr, "%s", printrep(c1));
            fprintf(stderr, "%s", printrep(c2));
            fprintf(stderr, "%s", printrep(c3));
            fprintf(stderr, "%s", printrep(c4));
            fprintf(stderr, " (%02X %02X %02X %02X)\n", c1, c2, c3, c4);
            exit(1);
        }

    int EP = get2(input);
    fprintf(stdout, "EP: %03X\n", EP);

    int n;
    while ((n = getc(input)) != EOF)
        {
            if (debug) fprintf(stderr, "Read next block of %d bytes\n", n);
            n = n - 1;
            int addr = get2(input); n -= 2;
            while (n > 0)
                {
                    int data = get2(input); n -= 2;            
                    fprintf(stdout, "%03X: %03X\n", addr, data);
                    addr += 1;
                }
        }
    
    if (debug)
        fprintf(stderr, "EOF\n");
}

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void scanargs(STRING s)
{
    /* check each character of the option list for
       its meaning. */

    while (*++s != '\0')
        switch (*s)
            {

            case 'D': /* debug option */
                debug = TRUE;
                break;

            default:
                fprintf (stderr,"obj8dump: Bad option %c\n", *s);
                fprintf (stderr,"usage: obj8dump [-D] file\n");
                exit(1);
            }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int main(int argc, STRING *argv)
{
    Boolean filenamenotgiven = TRUE;

    /* main driver program.  Define the input file
       from either standard input or a name on the
       command line.  Process all arguments. */

    while (argc > 1)
        {
            argc--, argv++;
            if (**argv == '-')
                scanargs(*argv);
            else
                {
                    FILE *input;

                    filenamenotgiven = FALSE;
                    input = fopen(*argv,"r");
                    if (input == NULL)
                        {
                            fprintf (stderr, "Can't open %s\n",*argv);
                        }
                    else
                        {
                            Read_and_Dump_PDP8_Object_File(input);
                            fclose(input);
                        }
                }
        }

    if (filenamenotgiven)
        {
            Read_and_Dump_PDP8_Object_File(stdin);
        }

    exit(0);
}


