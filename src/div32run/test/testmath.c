#include "test.h"

// Funciones de f.cpp
void get_angle( void );
void get_dist( void );
void get_disx( void );
void get_disy( void );
void _abs( void );

extern int data_get_angle[125];
extern int data_get_dist[130];
extern int data_get_distx[624];
extern int data_get_disty[624];
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

void test_get_distx()
{
	TEST_START();
	int i;
	for( i = 0; i < COUNT( data_get_distx ); i += 3 ) {
		err += test_func2( "get_distx", get_disx, data_get_distx[i], data_get_distx[i + 1], data_get_distx[i + 2] );
	}
	TEST_END();
}

void test_get_disty()
{
	TEST_START();
	int i;
	for( i = 0; i < COUNT( data_get_disty ); i += 3 ) {
		err += test_func2( "get_disty", get_disx, data_get_disty[i], data_get_disty[i + 1], data_get_disty[i + 2] );
	}
	TEST_END();
}

void test_abs()
{
	TEST_START();
	int i;
	for( i = 0; i < COUNT( data_abs ); i += 2 ) {
		err += test_func1( "abs", _abs, data_abs[i], data_abs[i + 1] );
	}
	TEST_END();
}

void test_math()
{
	test_get_angle();
	test_get_dist();
	test_get_distx();
	test_get_disty();
	test_abs();
}