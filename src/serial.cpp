/*
 * 	serial.cpp
 * 		Hardware dependent
 * 		serial port management
 */

#include <stdio.h>
#include <stdarg.h>

#include "serial.h"
#include "serdefs.h"

#include "Tserial_event.h"
#include "rerror.h"

static SERIAL_CBACK_T call_backs[ NUM_SERIALS ];

static
int
search_dev( void *com_object )
{
	SERIAL_T *p;

	for( p = serials ; p < serials + NUM_SERIALS ; ++p )
	{
		if( *p->com_name == '\0' )
			continue;
		if( p->com == com_object )
			return p - serials;
	}
	return -1;
}

static
void
SerialEventManager( uint32 object, uint32 event )
{
    char *buffer;
    int   size, dev_no;
    Tserial_event *com;
	SERIAL_CBACK_T *p;

	com = (Tserial_event *)object;
	if( ( dev_no = search_dev( com ) ) < 0 )
		fatal( "%s.Can't find device", __FUNCTION__ );
	p = &call_backs[dev_no];
    if( ( com = (Tserial_event *)object ) != 0 )
    {
        switch(event)
        {
            case  SERIAL_CONNECTED :
				if( p->serial_connect != NULL )
					(*p->serial_connect)();
				debug( "serial_connected( %d )", dev_no );
				break;
            case  SERIAL_DISCONNECTED  :
				if( p->serial_disconnect != NULL )
					(*p->serial_disconnect)();
				debug( "serial_disconnected( %d )", dev_no );
				break;
            case  SERIAL_DATA_SENT  :
				if( p->serial_data_sent != NULL )
					(*p->serial_data_sent)();
				debug( "serial_data_sent( %d )", dev_no );
				break;
            case  SERIAL_RING:
				if( p->serial_ring != NULL )
					(*p->serial_ring)();
				debug( "serial_ring( %d )", dev_no );
				break;
            case  SERIAL_CD_ON:
				if( p->serial_cd_on != NULL )
					(*p->serial_cd_on)();
				debug( "serial_cd_on( %d )", dev_no );
				break;
            case  SERIAL_CD_OFF     :
				if( p->serial_cd_off != NULL )
					(*p->serial_cd_off)();
				debug( "serial_cd_off( %d )", dev_no );
				break;
            case  SERIAL_DATA_ARRIVAL  :
                size   = com->getDataInSize();
                buffer = com->getDataInBuffer();
				//debug( "serial_data_arrival ( %d ) %02.2X %c", dev_no, (unsigned char)(*buffer), isprint(*buffer) ? *buffer: '.' );
				if( p->rx != NULL )
					(*p->rx)( *buffer );
                com->dataHasBeenRead();
                break;
        }
    }
}

static
char *
patch_com( char *name )
{
	static char true_com[20];

	sprintf_s( true_com, "\\\\.\\%s", name );
	return true_com;
}

static
SERIAL_T *
get_serial_dev( int device_no )
{
	SERIAL_T *pdev;

	if( device_no >= NUM_SERIALS )
		fatal( "%s.Bad device number %d", __FUNCTION__, device_no );
	if( *( pdev = &serials[ device_no ] )->com_name == '\0' )
		fatal( "%s.Device %d without data", __FUNCTION__, device_no );
	return pdev;
}

static
Tserial_event *
get_serial_com( int device_no )
{
	SERIAL_T *pdev;

	pdev = get_serial_dev( device_no );
	return (Tserial_event *)(pdev->com);
}


/*
 * 		Public functions
 */

void
init_serial_hard( int device_no, SERIAL_CBACK_T *p )
{
	SERIAL_T *pdev;
	Tserial_event *com;

	pdev = get_serial_dev( device_no );

	call_backs[ device_no ] = *p;
	if( ( pdev->com = new Tserial_event() ) == 0 )
		fatal( "%s.Can't create serial object", __FUNCTION__ );
	com = (Tserial_event *)(pdev->com);
	com->setManager( SerialEventManager );
}

void
connect_serial( int device_no )
{
	int error;
	SERIAL_T *pdev;
	Tserial_event *com;

	pdev = get_serial_dev( device_no );

	if( ( com = (Tserial_event *)pdev->com ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	if( ( error = com->connect
			(
				patch_com( pdev-> com_name),
				pdev->baud,
				pdev->bit_num,
				pdev->parity,
				pdev->stop_num,
				pdev->is_modem
			)
		) != 0 )
	{
		delete( com );
		pdev->com = 0;
		fatal( "%s.Error %u trying to open %s", __FUNCTION__, error, pdev->com_name );
	}
}

void
tx_data( int device_no, char byte )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );

	while( !com->sendData( &byte, 1) )
		Sleep(0);
}

void
reset_rts( int device_no )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	com->resetRts();
}

void
set_rts( int device_no )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	com->setRts();
}

void
reset_dtr( int device_no )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	com->resetDtr();
}

void
set_dtr( int device_no )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	com->setDtr();
}

void
disconnect_serial( int device_no )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	com->disconnect();
}

void
deinit_serial_hard( int device_no )
{
	Tserial_event *com;

	if( ( com = get_serial_com( device_no ) ) == 0 )
		fatal( "%s: com object doesn't exists", __FUNCTION__ );
	delete(com);
}
	




