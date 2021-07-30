#include "test.h"

// Funciones de f.cpp
void get_dist( void );
void get_angle( void );
void _abs( void );

extern int data_get_angle[125];
extern int data_get_dist[130];
extern int data_abs[22];

void test_get_angle()
{
	TEST_START();

	int i;
	for( i = 0; i < COUNT( data_get_angle ); i += 5 ) {
		mem[id + _X] = data_get_angle[i];
		mem[id + _Y] = data_get_angle[i + 1];
		mem[proc2 + _X] = data_get_angle[i + 2];
		mem[proc2 + _Y] = data_get_angle[i + 3];
		err = test_func1( "get_angle", get_angle, proc2, data_get_angle[i + 4] );
	}

	TEST_END();
}

void test_get_dist()
{
	TEST_START();

	int i;
	for( i = 0; i < COUNT( data_get_dist ); i += 5 ) {
		mem[id + _X] = data_get_dist[i];
		mem[id + _Y] = data_get_dist[i + 1];
		mem[proc2 + _X] = data_get_dist[i + 2];
		mem[proc2 + _Y] = data_get_dist[i + 3];
		err = test_func1( "get_dist", get_dist, proc2, data_get_dist[i + 4] );
	}

	TEST_END();
}

void test_abs()
{
	TEST_START();

	int i;
	for( i = 0; i < COUNT(data_abs); i+=2 ) {
		err += test_func1( "abs", _abs, data_abs[i], data_abs[i+1] );
	}

	TEST_END();
}

void test_math()
{
	test_get_angle();
	test_get_dist();
	test_abs();
}