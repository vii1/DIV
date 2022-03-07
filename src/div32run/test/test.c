#define DEFINIR_AQUI
#include "test.h"
#include "testdata.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void test_math();

int errors = 0;
int tests = 0;
int calls = 0;

int epsilon = 0;

int testmem[1024];
int proc2;

int exec_func1( void ( *pfunc )( void ), int arg1 )
{
	assert( sp == 0 );
	pila[sp] = arg1;
	pfunc();
	if( sp != 0 ) {
		errors++;
		printf( "ERROR! Wrong SP (expected 0, actual %d)\n", sp );
		sp = 0;
	}
	return pila[sp];
}

int exec_func2( void ( *pfunc )( void ), int arg1, int arg2 )
{
	assert( sp == 0 );
	pila[sp++] = arg1;
	pila[sp] = arg2;
	pfunc();
	if( sp != 0 ) {
		errors++;
		printf( "ERROR! Wrong SP (expected 0, actual %d)\n", sp );
		sp = 0;
	}
	return pila[sp];
}

int test_func1( const char* name, void ( *pfunc )( void ), int arg1, int expected )
{
	int result = exec_func1( pfunc, arg1 );
	++calls;
	if( abs( expected - result ) > epsilon ) {
		if( epsilon ) {
			printf( "ERROR! [%d] %s( %d ) -> Expected %d ñ%d, got %d\n", calls, name, arg1, expected, epsilon, result );
		} else {
			printf( "ERROR! [%d] %s( %d ) -> Expected %d, got %d\n", calls, name, arg1, expected, result );
		}
		return 1;
	} else {
		return 0;
	}
}

int test_func2( const char* name, void ( *pfunc )( void ), int arg1, int arg2, int expected )
{
	int result = exec_func2( pfunc, arg1, arg2 );
	++calls;
	if( abs( expected - result ) > epsilon ) {
		printf( "ERROR! [%d] %s( %d, %d ) -> Expected %d, got %d\n", calls, name, arg1, arg2, expected, result );
		return 1;
	} else {
		return 0;
	}
}

int assert_equal( const char* name, int expected, int result )
{
	if( abs( expected - result ) > epsilon ) {
		if( epsilon ) {
			printf( "ERROR! [%d] %s -> Expected %d ñ%d, got %d\n", calls, name, expected, epsilon, result );
		} else {
			printf( "ERROR! [%d] %s -> Expected %d, got %d\n", calls, name, expected, result );
		}
		return 1;
	} else {
		return 0;
	}
}

int main( int argc, char* argv[] )
{
	mem = testmem;
	imem_max = COUNT( testmem );
	id = ( imem_max / 2 ) | 1;
	proc2 = ( imem_max / 4 * 3 ) | 1;
	sp = 0;

	test_math();

	printf( "Tests run: %d, Failed: %d\n", tests, errors );
	return errors;
}
