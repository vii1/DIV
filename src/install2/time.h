#ifndef __TIME_H__
#define __TIME_H__

#define TICKS_PER_SEC 100

extern unsigned int reloj;
extern unsigned int time10frames;

int timer_init();
void timer_uninit();
void measure_time();

#endif
