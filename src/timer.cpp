/*
 *		timer.cpp
 *			Hardware Dependent Level
 *			for timer in PC
 */
 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <stdarg.h>


#include "timer.h"

#include "rerror.h"


/*	millisecons each interrupt	*/
static HANDLE timer_handler;
static void (*timer_cback)( void );
/*
 *	timer_tick
 *		Called each timer tick
 */

static
void
timer_tick( void )
{

	(*timer_cback)();
	Sleep(0);
}

/*
 * 	timer_thread:
 * 		Inits timer with 10 milliseconds
 * 		and sets callback function as 'timer_inter'
 * 		which is external to this module
 * 		Then sits in a loop waiting and dispatching
 * 		the WM_TIMER message
 */

void
timer_thread( PVOID pvoid )
{
	int Timer;
	MSG msg;

	if( ( Timer = SetTimer( NULL, USER_TIMER_MINIMUM, TIME_INTER, (TIMERPROC)timer_tick ) ) == 0 )
		fatal( "%s: Failed to create timer" );
	while( GetMessage(&msg, NULL, 0, 0) )
	{
		switch( msg.message )
		{
			case WM_TIMER:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}
	}
}

/*
 * 	start_timer:
 * 		Starts a new thread for timer
 * 		Thread doesn't run at creation
 */

static
void
start_timer( void )
{
	timer_handler = CreateThread( NULL, 1024, (LPTHREAD_START_ROUTINE)timer_thread, NULL, CREATE_SUSPENDED, NULL );
 	if( timer_handler == 0 )
		fatal( "%s.ERROR: Could not start timer thread", __FUNCTION__ );
}

/*
 * 	Public functions
 */

/*
 * 	init_hard:
 * 		Performs all hardware initialization
 */

void
init_timer_hard( void (*timer_callback)( void ) )
{
	if( timer_callback == NULL )
		fatal( "%s.No timer callback", __FUNCTION__ );
	timer_cback = timer_callback;
	start_timer();
}

/*
 * 	enable_timer_interrupts:
 * 		Enable timer interrupts
 */

void
enable_timer_interrupts( void )
{
	ResumeThread( timer_handler );
}

/*
 * 	disable_timer_interrupts:
 * 		Disable timer interrupts
 */

void
disable_timer_interrupts( void )
{
	SuspendThread( timer_handler );
}


