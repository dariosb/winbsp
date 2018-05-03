/*
 * 	sertbl.c
 */

#include "serdefs.h"
#include "serial.h"

SERIAL_T serials[ NUM_SERIALS ] =
{
	{	"COM4",	9600, 8, PAR_NONE,	STOP_1, 0 },
	{	"COM6",	9600, 8, PAR_NONE,	STOP_1, 0 }
};

