#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

typedef uint16_t pdp8word;

#define PDP8_MEMSIZE 4096
#define OBJ8_SEGWORD_MAX			126
#define OBJ8_HDR_SIZE				4
#define OBJ8_EP_SIZE				(sizeof(pdp8word))
#define VALIDATOR_LIMIT_SEGNUMBER		(4096)
#define VALIDATOR_LIMIT_DIFF			16

#define VE_FOPEN				0x1 // Could not open file
#define VE_MEM					0x2 // Images do not match
#define VE_ENTRY				0x4 // Entry points do not match

#define VERBOSE					0

const char* OBJ8_STD_HDR="OBJ8";

pdp8word omem[PDP8_MEMSIZE] = {0}, rmem[PDP8_MEMSIZE] = {0};
pdp8word oep, rep;

pdp8word deflat(FILE* f)
{
	int high;
	int low;
	pdp8word w;

	high = getc(f);
	low = getc(f);
	w = ((high & 0x3F) << 6) | (low & 0x3F);
	return w & 0xFFF;
}

int loadbin(pdp8word mem[], pdp8word *pep, const char *fn)
{
	int bcount = 0;
	char hdr[OBJ8_HDR_SIZE];
	FILE *f = fopen(fn, "r");
	if (!f) {
		int en = errno;
		fprintf(stderr, "Could not open file %s\n", fn);
		return en;
	}
	fread(hdr, OBJ8_HDR_SIZE, 1, f);
	if (memcmp(hdr, OBJ8_STD_HDR, OBJ8_HDR_SIZE)) {
		printf("File %s doesn't have a valid OBJ8 header\n", fn);
		printf("Header dump:\n");
		for(int i = 0; i < OBJ8_HDR_SIZE; i++) {
			printf("\t%X ", hdr[i]);
		}
		return ENOEXEC;
	}
	*pep = deflat(f);
	while (!feof(f)) {
		/* Read blocks */
		uint8_t bsize;
		pdp8word baddr;

		long bstart = ftell(f);
		size_t attempt = fread(&bsize, sizeof(bsize), 1, f);
		if (attempt == 0) // EOF
			break; 
		long bend = bstart + bsize;
		baddr = deflat(f);
#if VERBOSE
		fprintf(stderr, "Block %d [%X, %X), file section [%X, %x)\n", bcount, baddr, baddr+bsize, bstart, bend);
#endif
		int pos = 0;
		while (ftell(f) < bend) {
			mem[baddr+pos] = deflat(f);
			//fprintf(stderr, "Load %03hX to %03X\n", mem[baddr+pos] , baddr+pos);
#if VERBOSE
			fprintf(stderr, "File position %ld\n", ftell(f));
#endif
#if 0
			if (ftell(f) == 15)
				break;
#endif
			pos++;
		}

		bcount++;
	}
	if (bcount > VALIDATOR_LIMIT_SEGNUMBER) {
		printf("File %s uses too many blocks, try to coalesce small blocks into larger ones\n");
		return EINVAL;
	}
	fclose(f);
	return 0;
}

int loadhex(pdp8word mem[], pdp8word *pep, const char *fn)
{
	FILE *f = fopen(fn, "r");
	if (!f) {
		int en = errno;
		fprintf(stderr, "Could not open file %s\n", fn);
		return en;
	}
	fscanf(f, "EP: %03hX\n", pep);
	while (!feof(f)) {
		int pos;
		int value;
		fscanf(f, "%03X: %03X\n", &pos, &value);
		if (pos < 0 || pos >= PDP8_MEMSIZE) {
			fprintf(stderr, "Invalid object code\n");
		}
		mem[pos] = value & 0xFFF;
	}
	fclose(f);
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <obj8file> <reference obj8file (in HEX)>\n", argv[0]);
		fprintf(stderr, "\t Example: %s test.out test.obj\n", argv[0]);
		return 0;
	}
	if (loadbin(omem, &oep, argv[1]))
		return VE_FOPEN;
	if (loadhex(rmem, &rep, argv[2]))
		return VE_FOPEN;
	int diffn = 0;
	for (int i = 0; i < PDP8_MEMSIZE; i++) {
		if (omem[i] != rmem[i]) {
			ret |= VE_MEM;
			diffn++;
			fprintf(stderr, "DIFF at %03X: 0x%03hX, should be: 0x%03hX\n",
					i, omem[i], rmem[i]);
			if (diffn > VALIDATOR_LIMIT_DIFF) {
				fprintf(stderr, "Too many differences, exiting\n");
				break;
			}
		}
	}
	if (oep != rep) {
		ret |= VE_ENTRY;
		fprintf(stderr, "DIFF at EPs: 0x%03hX, should be: 0x%03hX\n",
				oep, rep);
	}
	return ret;
}
