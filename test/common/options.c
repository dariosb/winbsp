/*
 * 	options.c
 * 		Management of test options
 * 		This is dependent on test
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "external.h"
#include "terminal.h"
#include "options.h"
#include "log.h"
#include "serial.h"
#include "messages.h"
#include "getopt.h"

/*
 * 	Macros and definitions
 */

#define COMMENT	'#'

#define show_parity(par)	invert_meaning((par),parcon)
#define show_stop(stop)		invert_meaning((stop),stopcon)
#define convert_parity(par_text)	convert_meaning((par_text),parcon)
#define convert_stop(stop_text)		convert_meaning((stop_text),stopcon)

#define sep					",\n"

enum
{
	COMM_OPTIONS,
	SAFE_COMM = COMM_OPTIONS,
	TEST_COMM,
	VAL_COMM,
	PRINTER_COMM,
	VERSION_TXT,
	FSYS_FRMT_T,
	TICKET_T,
	MAX_OPTS
};

enum
{
	NO_LOG, LOG
};

/*
 * 	Typedefs
 */

typedef struct
{
	int letter;
	int value;
} CONVERT_T;

/*
 * 	Initialized static const tables
 */

/*
 * 	For conversion between external
 * 	declaration and internal values
 */

/*
 * 	Parity values
 */

static const CONVERT_T parcon[] =
{
	{ 'N',	PAR_NONE	},
	{ 'O',	PAR_ODD		},
	{ 'E',  PAR_EVEN	},
	{ 'M',	PAR_MARK	},
	{ 'S',	PAR_SPACE	},
	{ '\0' }
};

/*
 * 	Stop values
 */

static const CONVERT_T stopcon[] =
{
	{ '1',	STOP_1		},
	{ '5',	STOP_1HALF	},
	{ '2',	STOP2		},
	{ '\0' }
};

/*
 * 	Available commands for optdef.txt
 */

static const char *commands[ MAX_OPTS ] =
{
	"safe_comm",
	"test_comm",
	"val_comm",
	"printer_comm",
	"lower_version",
	"fsys_format_time",
	"print_ticket"
};

/*
 * 	Command line letter options
 */

static char *opts = "f:dhl:"; 

/*
 * 	Initialized static 'not constant' tables
 */

/*
 * 	Option structure
 */

OPTIONS_T options =
{
	{ "COM6", 38400,	8,	PAR_NONE,	STOP_1, 5000,	100		},
	{ "COM7", 38400,	8,	PAR_NONE,	STOP_1, 0, 		0		},
	{ "COM5", 9600,		8,	PAR_EVEN,	STOP_1, 0,		0		},
	{ "COM1", 9600,		8,	PAR_NONE,	STOP_1, 0,		0		},
	{ "SW00.16.00" },
	{ 210 },
	{ 0, "HWXX.XXAX.X", 'X', 'X' }
};

/*
 * 	Static uninitialized variables
 */

static char buffer[ 100 ];			/*	buffer: used for field separation	*/	
static int line;					/*	For errors logging in optdef.txt	*/

/*
 * 	extern variables
 */

extern int debug_flag;

TICKET_STR ticket;

/*
 * 	Default option file
 */

char option_file[ MAX_OPTION_FILE ] = "optdef.txt";


/*
 *  ------- Treatment of line option command
 */

/*
 * 	convert_meaning
 * 		Converts from external declaration
 * 		to internal definition.
 * 		Used for parity and stop
 */

static
int
convert_meaning( int letter, const CONVERT_T *p )
{
	for(  ; p->letter != '\0' ; ++p )
		if( p->letter == letter )
			return p->value;
	return -1;
}

/*
 * 	invert_meaning:
 * 		Converts from internal value to
 * 		external declaration.
 * 		Used for parity and stop
 */

static
int
invert_meaning( int value, const CONVERT_T *p )
{
	for(  ; p->letter != '\0' ; ++p )
		if( p->value == value )
			return p->letter;
	return -1;
}

/*
 * -----	Here begins all action routines
 *  		for treatment of each field of
 *  		communication setting on optdef.txt
 */

static
int
process_com_name( COMM_T *pc, char *ptext )
{
	strcpy( pc->com_name, ptext );
	return 0;
}

static
int
process_baud( COMM_T *pc, char *ptext )
{
	pc->baud = atoi( ptext );
	return 0;
}

static
int
process_bitnum( COMM_T *pc, char *ptext )
{
	pc->bit_num = atoi( ptext );
	return 0;
}

static
int
process_parity( COMM_T *pc, char *ptext )
{
	int par;

	if( ( par = convert_parity( *ptext ) ) < 0 )
		return -1;
	pc->parity = par;
	return 0;
}

static
int
process_stopnum( COMM_T *pc, char *ptext )
{
	int stop;

	if( ( stop = convert_stop( *ptext ) ) < 0 )
		return -1;
	pc->stop_num = stop;
	return 0;
}

static
int
process_timeout1( COMM_T *pc, char *ptext )
{
	pc->timeout1 = atol( ptext );
	return 0;
}

static
int
process_timeout2( COMM_T *pc, char *ptext )
{
	pc->timeout2 = atol( ptext );
	return 0;
}

/*
 * 	Here inidrection table to action routines
 * 	for each field of communication definition in
 * 	optdef.txt
 */

static int (* const process_comm[ COMM_T_MEMBERS ])( COMM_T *pc, char *ptext ) =
{
	process_com_name, process_baud, process_bitnum, process_parity, process_stopnum,
	process_timeout1, process_timeout2
};

/*
 * 	options_comm:
 * 		Special treatment of communication options
 * 		Receives pointer to options data and pointer
 * 		to tail of options line
 */

static
int
options_comm( COMM_T *pcomm, char *ptail )
{
	int member;

	for( member = 0; member < COMM_T_MEMBERS ; ++member )
		if( ( ptail = strtok( NULL, sep ) ) == NULL )
			return -1;
		else if( (*process_comm[member])( pcomm, ptail ) < 0 )
			return -1;
	return 0;
}

/*
 * 	options_comm:
 * 		Special treatment of communication options
 * 		Receives pointer to options data and pointer
 * 		to tail of options line
 */

static
int
options_version( VERSION_T *pcomm, char *ptail )
{
	if( ( ptail = strtok( NULL, sep ) ) == NULL )
		return -1;

	memcpy( lower_version, ptail, strlen(ptail) );
	return 0;
}

static
int
options_fsys_fmt_t( PARAMETER_T *pcomm, char *ptail )
{
	if( ( ptail = strtok( NULL, sep ) ) == NULL )
		return -1;

	fsys_format_time = atol(ptail);
	return 0;
}

static
int
proc_ticket_ena( TICKET_STR *pt, char *ptext )
{
	ticket.print_enable = atoi( ptext );
	return 0;
}

static
int
proc_hardver( TICKET_STR *pt, char *ptext )
{
	if( strlen(ptext) != (HW_VER_LENGHT - 1) )
		return -1;
	strcpy( ticket.hw_version, ptext );
	return 0;
}

static
int
proc_int1( TICKET_STR *pt, char *ptext )
{
	ticket.val1_type = *ptext;
	return 0;
}

static
int
proc_int2( TICKET_STR *pt, char *ptext )
{
	ticket.val2_type = *ptext;
	return 0;
}

/*
 * 	Here inidrection table to action routines
 * 	for each field of communication definition in
 * 	optdef.txt
 */

static int (* const process_ticket[ TICKET_STR_MEMBERS ])( TICKET_STR *pt, char *ptext ) =
{
	proc_ticket_ena, proc_hardver, proc_int1, proc_int2
};

static
int
options_ticket_t( PARAMETER_T *ptick, char *ptail )
{
	int member;

	for( member = 0; member < TICKET_STR_MEMBERS ; ++member )
		if( ( ptail = strtok( NULL, sep ) ) == NULL )
			return -1;
		else if( (*process_ticket[member])( ptick, ptail ) < 0 )
			return -1;
	return 0;
}

/*
 * 		validate_command:
 * 			Verifies that the command name is 
 * 			correct; returns index or
 * 			negative number in case of error
 */

static
int
validate_command( const char *pcomm )
{
	static const char **p;

	for( p = commands; p < commands + MAX_OPTS ; ++p )
		if( strcmp( *p, pcomm ) == 0 )
			return p - commands;
	return -1;
}

/*
 * 		process_opt:
 * 			Process an option from the option file
 * 			If bad option, returns negative, else returns 0
 */

int
process_opt( const char *pcomm, char *parg )
{
	int index;

	if( ( index = validate_command( pcomm ) ) < 0 )
	   return -1;
	switch( index )
	{
		case SAFE_COMM:
			return options_comm( &options.safe, parg );
		case TEST_COMM:
			return options_comm( &options.test, parg );
		case VAL_COMM:
			return options_comm( &options.val, parg );
		case PRINTER_COMM:
			return options_comm( &options.printer, parg );
		case VERSION_TXT:
			return options_version( &options.version, parg );
		case FSYS_FRMT_T:
			return options_fsys_fmt_t( &options.fsys_format_time, parg );
		case TICKET_T:
			return options_ticket_t( &options.ticket, parg );
		default:
			return -1;

	}
}

/*
 * 	show_comm_data:
 * 		Used for help in case of communication option
 */

static
void
show_comm_data( const char *channel, COMM_T *p, int log_it )
{
	fprintf( stderr, msg_comm_channel[ lang ],
		channel, p->com_name, p->baud,  p->bit_num, show_parity(p->parity),show_stop(p->stop_num),
		p->timeout1, p->timeout2 );
	fputc( '\n', stderr );
	if( log_it )
		log( msg_comm_channel[ lang ],
			channel, p->com_name, p->baud,  p->bit_num, show_parity(p->parity),show_stop(p->stop_num),
			p->timeout1, p->timeout2 );
}


/*
 * -----------	Treatment of option file
 */

static
void
show_all_comm_data( int log_it )
{
	show_comm_data( "safe", &options.safe, log_it );
	show_comm_data( "test", &options.test, log_it );
	show_comm_data( "val ", &options.val, log_it );
	show_comm_data( "printer", &options.printer, log_it );
}

/*
 * 		read_option_file:
 * 			Opens and read options from the options
 * 			file
 */

void
read_option_file( char *option_file )
{
	FILE *f;
	char *p;
	char command[50];
	int num;

	if( ( f = fopen( option_file, "rt" ) ) == NULL )
		log( no_options_file[ lang ], option_file );
	else
	{
		log( getting_options_from[ lang ], option_file );
		for( line = 1; fgets( buffer, sizeof( buffer ), f ) != NULL; ++line )
		{
			if( ( num = strlen( buffer ) ) == 0 || num == 1 || *buffer == COMMENT || ( p = strtok( buffer, sep ) ) == NULL )
				continue;
			strncpy( command, p, sizeof( command ) );
			str2lower( command );
			if( process_opt( command, p ) < 0 )
			{
				log_both( error_in_options_file[ lang ], option_file, line );
				fclose( f );
				exit( EXIT_FAILURE );
			}
		}
	}
	show_all_comm_data( LOG );
	write_comm_tables();
	sak();
}


/*
 * 	show_help:
 */

void
show_help( void )
{
	fprintf( stderr, help_message[lang] );
	show_all_comm_data( NO_LOG );
}

/*
 * 	evaluate_args
 */

void
evaluate_args( int argc, char **argv )
{
	int c;

	while( ( c = getopt( argc, argv, opts ) ) != EOF )
		switch( c )
		{
			case 'f':
				strncpy( option_file, optarg, sizeof( option_file ) );
				log( option_line_command[ lang ], option_file );
				break;
			case 'd':
				debug_flag = 1;
				break;
			case 'h':
				show_help();
				exit( EXIT_SUCCESS );
			case 'l':
				switch( tolower(*optarg) )
				{
					case 'e':
						set_language( ENGLISH );
						break;
					case 's':
						set_language( SPANISH );
						break;
					default:
						usage( argv[0] );
				}
				break;
			case '?':
				usage( argv[0] );
				break;
		}
}

/*
 * 	usage:
 */

void
usage( char *name )
{
	fprintf( stderr, usage_msg[ lang ], name );
	exit( EXIT_FAILURE );
}

#ifdef PRINTER

void
do_print_test( void )
{
	char buffer[200];

	sprintf( buffer, "%s %s", options.printer_on, get_this_name() );
	system( buffer );
	if( strcmp( options.printer_off, "NULL" ) != 0 )
		system( options.printer_off );
}

#endif

