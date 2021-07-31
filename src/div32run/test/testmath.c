#include "test.h"

// Funciones de f.cpp
void get_angle( void );
void get_dist( void );
void get_disx( void );
void get_disy( void );
void _abs( void );
void _sqrt( void );

extern int data_get_angle[];
extern const int data_get_angle_length;
extern int data_get_dist[];
extern const int data_get_dist_length;
extern int data_get_distx[];
extern const int data_get_distx_length;
extern int data_get_disty[];
extern const int data_get_disty_length;
extern int data_abs[];
extern const int data_abs_length;
extern int data_sqrt[];
extern const int data_sqrt_length;

void test_get_angle()
{
	TEST_START();
	int i;
	for( i = 0; i < data_get_angle_length; i += 5 ) {
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
	for( i = 0; i < data_get_dist_length; i += 5 ) {
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
	// El DIV2 original tiene un error de ñ1 seg£n es 386 o 586
	epsilon = 1;
	for( i = 0; i < data_get_distx_length; i += 3 ) {
		err += test_func2( "get_distx", get_disx, data_get_distx[i], data_get_distx[i + 1], data_get_distx[i + 2] );
	}
	TEST_END();
}

void test_get_disty()
{
	TEST_START();
	int i;
	epsilon = 1;
	for( i = 0; i < data_get_disty_length; i += 3 ) {
		err += test_func2( "get_disty", get_disy, data_get_disty[i], data_get_disty[i + 1], data_get_disty[i + 2] );
	}
	TEST_END();
}

void test_abs()
{
	TEST_START();
	int i;
	for( i = 0; i < data_abs_length; i += 2 ) {
		err += test_func1( "abs", _abs, data_abs[i], data_abs[i + 1] );
	}
	TEST_END();
}

void test_sqrt()
{
	TEST_START();
	int i;
	for( i = 0; i < data_sqrt_length; i += 2 ) {
		err += test_func1( "sqrt", _sqrt, data_sqrt[i], data_sqrt[i + 1] );
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
	test_sqrt();
}