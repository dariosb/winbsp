/*
 * 	timetest.c
 */

#include <windows.h>
#include <conio.h>
#include <stdio.h>

#include "timer.h"
#include "rerror.h"

#define ONE_SECOND	(1000/TIME_INTER)

static unsigned long time_10ms;
static unsigned one_second;
static unsigned volatile seconds;

#define TARGET_RESOLUTION 1         // 1-millisecond target resolution

static
void
show_interrupts( void )
{
	while( !_kbhit() )
		printf( "%6u %3u\r", time_10ms, seconds );
	_getch();
	putchar( '\n' );
}

void
timer_inter( void )
{
	++time_10ms;
	if( --one_second == 0 )
	{
		one_second = ONE_SECOND;
		++seconds;
	}
}

int
main( void )
{
	init_timer_hard( timer_inter );
	seconds = 0;
	one_second = ONE_SECOND;
	printf( "Timer interrupt disabled\n" );
	show_interrupts();
	enable_timer_interrupts();
	printf( "Timer interrupt enabled\n" );
	show_interrupts();
	disable_timer_interrupts();
	printf( "Timer interrupt disabled\n" );
	show_interrupts();
	enable_timer_interrupts();
	printf( "Timer interrupt enabled\n" );
	show_interrupts();
}

