#ifndef __TEST_H__
#define __TEST_H__

#include "inter.h"
#include <stdio.h>

#define COUNT( x ) ( sizeof( x ) / sizeof( x[0] ) )

#define TEST_START() \
	int err = 0;     \
	calls = 0;       \
	epsilon = 0;     \
	{

#define TEST_END()                           \
	}                                        \
	if( err ) {                              \
		printf( "%s: FAILED!\n", __func__ ); \
		errors++;                            \
	} else {                                 \
		printf( "%s: OK\n", __func__ );      \
	}                                        \
	tests++;

extern int errors;
extern int tests;
extern int calls;
extern int epsilon;

extern int testmem[1024];
extern int proc2;

int test_func1( const char* name, void ( *pfunc )( void ), int arg1, int expected );
int test_func2( const char* name, void ( *pfunc )( void ), int arg1, int arg2, int expected );

#endif /* __TEST_H__ */
