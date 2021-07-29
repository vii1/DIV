#define DEFINIR_AQUI
#include "inter.h"

#include <assert.h>
#include <stdio.h>
#include <dos.h>

void _abs( void );

int errors = 0;
int tests = 0;

int testmem[1024];

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

int test_func1( const char* name, void ( *pfunc )( void ), int arg1, int expected )
{
	int result = exec_func1( pfunc, arg1 );
	if( result != expected ) {
		printf( "ERROR! %s( %d ) -> Expected %d, got %d\n", name, arg1, expected, result );
		return 1;
	} else {
		return 0;
	}
}

void test_abs()
{
	int err = 0;
	int testvalues[] = { 1, 5, -5, 0, -781093233, -768238602, -395951812, -121276127, -423148680, 681604608, -726953351,
		-386418134, -263173036, 250414929, -793446191, -429891284, 516044276, 943988811, 658134417, 483752977,
		-324559879, 42134480, -80138760, 568240637 };
	int i;
	for( i = 0; i < sizeof( testvalues ) / sizeof( testvalues[0] ); i++ ) {
		err += test_func1( "abs", _abs, testvalues[i], testvalues[i] < 0 ? -testvalues[i] : testvalues[i] );
	}
	if( err ) {
		printf( "test_abs: FAILED!\n" );
		errors++;
	} else {
		printf( "test_abs: OK\n" );
	}
	tests++;
}

int main( int argc, char* argv[] )
{
	mem = testmem;
	imem_max = sizeof( testmem ) / sizeof( testmem[0] );
	id = long_header;
	sp = 0;
	test_abs();
	printf( "Tests run: %d, Failed: %d\n", tests, errors );
	return errors;
}
