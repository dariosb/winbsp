/*
 * 	maintst.c
 *		Example of timer and serial usage
 *		This program uses also some routines defined in
 *		utils library.
 *
 *	The test consists in using two serial devices, named
 *	MAIN_DEVICE (0) and SEC_DEVICE
 *
 *	SEC_DEVICE must be connected to a loopback (2 & 3 shorted)
 *	MAIN_DEVICE is any device that responds to serial commands
 *	(as for example a connection to a modem or a mobile phone)
 *
 *	Typing in PC keyboard, byte goes to SEC_DEVICE which, when
 *	receives the same byte through the loopbac, transmits it
 *	to the MAIN_DEVICE. All that is received from main device
 *	is sent to console output.
 *
 *	Each time a byte is sent to SEC_DEVICE, a timer is loaded
 *	in order to know if the loopback is there
 *
 *	When an EOF (^Z Enter) is received from keyboard, all hardware
 *	is closed and deinited. With ESC, the program is terminated.
 *	Any other key reinits testing loop
 */

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#include "rerror.h"
#include "getopt.h"
#include "serial.h"
#include "timer.h"
#include "mydefs.h"


enum
{
	MAIN_DEVICE, SEC_DEVICE
};

#define who_am_i()	fprintf( stderr, "%s\n", __FUNCTION__ )

static SERIAL_CBACK_T serial_callback;

void
main_rx_byte( unsigned char byte )
{
	putchar( byte );
}

void
sec_rx_byte( unsigned char byte )
{
	tx_data( SEC_DEVICE, byte );
}

void
main_connect( void )
{
	who_am_i();
}

void
main_disconnect( void )
{
	who_am_i();
}

void
sec_connect( void )
{
	who_am_i();
}

void
sec_disconnect( void )
{
	who_am_i();
}

static char *args = "d:D:hH";

static
void
show_help( char *name )
{
	fatal( "Usage: %s [-d {0|1} -h]", name );
}

static
void
evaluate_args( int argc, char **argv )
{
	int c;

	while( ( c = getopt( argc, argv, args ) ) != EOF )
	{
		switch( c )
		{
			case 'd':
			case 'D':
				debugging = atoi( optarg );
				break;
			case 'h':
			case 'H':
				show_help( *argv );
				break;
			default:
				show_help( *argv );
				break;
		}
	}
}


int
main( int argc, char **argv )
{
	int c;

	evaluate_args( argc, argv );

	do
	{
		serial_callback.rx					= main_rx_byte;
		serial_callback.serial_connect		= main_connect;
		serial_callback.serial_disconnect	= main_disconnect;
		init_serial_hard( MAIN_DEVICE, &serial_callback );

		serial_callback.rx					= sec_rx_byte;
		serial_callback.serial_connect		= sec_connect;
		serial_callback.serial_disconnect	= sec_disconnect;
		init_serial_hard( SEC_DEVICE, &serial_callback );

		connect_serial( MAIN_DEVICE );
		connect_serial( SEC_DEVICE );

		enable_timer_interrupts();

		while( ( c = _getch() ) != ESC )
			tx_data( MAIN_DEVICE, c == '\n'? '\r': (char)c );

		disable_timer_interrupts();
		clearerr( stdin );
		disconnect_serial( MAIN_DEVICE );
		deinit_serial_hard( MAIN_DEVICE );
		disconnect_serial( SEC_DEVICE );
		deinit_serial_hard( SEC_DEVICE );
		fprintf( stderr, "Any key to continue, ESC to abort...." );
		c = _getch();
		putchar( '\n' );
	} while( c != ESC );

	return EXIT_SUCCESS;
}

