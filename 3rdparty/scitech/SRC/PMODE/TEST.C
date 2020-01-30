#include <stdio.h>
#include <stdlib.h>
#include "scitech.h"
#include "pmode.h"

#ifndef	__32BIT__
#error Not a 32-bit environment!
#endif

#ifndef	__UNIX__
#error Not compiling for Unix!
#endif

#ifndef	__INTEL__
#error Not compiling for Intel!
#endif

#ifdef __MSDOS__
#error __MSDOS__ should not be defined!
#endif

#ifdef __WINDOWS__
#error __WINDOWS__ should not be defined!
#endif

void main(void)
{
    printf("Hello world!\n");

    printf("Program running in ");
	switch (PM_getModeType()) {
		case PM_realMode:
			printf("real mode.\n\n");
			break;
		case PM_286:
			printf("16 bit protected mode.\n\n");
			break;
		case PM_386:
			printf("32 bit protected mode.\n\n");
			break;
        }
}
