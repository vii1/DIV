/*
 * Constantes para la comunicaci¢n wstub <--> IDE
 */

#ifndef __WSTUB_H__
#define __WSTUB_H__

enum {
	DIV_RETVAL_EXEC_GAME = 1,
	DIV_RETVAL_TEST_MODE = 2,
    DIV_RETVAL_ENOMEM = 243,
    DIV_RETVAL_EINT = 256
};

#endif	 // __WSTUB_H__
