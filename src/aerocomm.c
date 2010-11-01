/*** interface functions for aerocomm communications ***/


#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>

#include "gpio.h"
#include "dictionary.h"
#include "exception.h"
#include "iohandler.h"
#include "log.h"
#include "aerocomm.h"


int ac_FileDesc = -1;
struct termios ac_AerocommTTY;


/* Handle any data uplinked to us - We totally ignore this */
void aerocomm_io_handler(int status)
{
	char temp_buffer[1024];
	int len;
	
	/* Read any data to empty the OS buffer */
	len = read(ac_FileDesc, temp_buffer, sizeof(temp_buffer)-1);

	return;
} /* aerocomm_io_handler */

/** Opens the serial comm connection to the Aerocomm modem **/
int aerocomm_open()
{
	int result = -1;
	
	/* Enable data pin on Aerocomm */
	(void) gpio_init();
	result = gpio_set( GPIO_AEROCOMM, 1 ); 
	EXCEPTION_IF( result < 0, enableFailed );
	
	/* open port */
	ac_FileDesc = open(SERIALPORT_AEROCOMM, O_RDWR | O_NOCTTY | O_NONBLOCK);
	EXCEPTION_IF( ac_FileDesc < 0, openFailed );

	log_printf( INFO, "AEROCOMM", "Opened ttyS3 on file descriptor %d.\n", ac_FileDesc );
	
	bzero(&ac_AerocommTTY, sizeof(ac_AerocommTTY));    
	ac_AerocommTTY.c_cflag = BAUDRATE_AEROCOMM;
	ac_AerocommTTY.c_iflag = IGNPAR;
	ac_AerocommTTY.c_oflag = 0;
	ac_AerocommTTY.c_lflag = 0;

	/* apply new settings */
	tcflush(ac_FileDesc, TCIFLUSH);                 /* flush old data */
	tcsetattr(ac_FileDesc, TCSANOW, &ac_AerocommTTY); /* apply new settings */
	fcntl(ac_FileDesc, F_SETOWN, getpid());         /* enable our PID to receive serial interrupts */
	fcntl(ac_FileDesc, F_SETFL, FASYNC);

	/* add io handler */
	result = iohandler_add( aerocomm_io_handler );
	EXCEPTION_IF( result < 0, installFailed );

	result = 0;
	
EXCEPTION_CATCH( enableFailed ):
	log_printf( ERROR, "AEROCOMM", "Failed to enable modem.\n" );	
	
EXCEPTION_CATCH( installFailed ):
	log_printf( ERROR, "AEROCOMM", "Failed to install IO handler!\n" );
	
EXCEPTION_CATCH( openFailed ):
	log_printf( ERROR, "AEROCOMM", "Failed to open %s.\n", SERIALPORT_AEROCOMM);

EXIT_BLOCK:
	return result;
} /* aerocomm_open */



/** defines for SLIP protocol **/
#define END     0300 /* begin/end marker */
#define ESC     0333 /* start escape sequence */
#define ESC_END 0334 /* to designate an escaped 0300 value */
#define ESC_ESC 0335 /* to designate an escaped 0333 value */

/* convert a message to a SLIP packet */
int convertToSLIP(unsigned char *dst, unsigned char *src, int src_len)
{
	int i, dst_len = 0;
	unsigned char *p, *q;
	
	p = src;
	q = dst;
	
	*q = END; q++;
	for ( i = 0; i < src_len; i++ )
	{
		switch ( *p )
		{
		case END:
			*q = ESC; q++; *q = ESC_END;
			break;
		case ESC:
			*q = ESC; q++; *q = ESC_ESC;
			break;
		default:
			*q = *p;
		}
		p++;
		q++;
	}
	*q = END; *q++;
	dst_len = (q - dst);
	
	return dst_len;
} /* convertToSLIP */


/* Prepare and send the TM */
/* Implements FOX_GROUND_ICD_v1.txt */
int aerocomm_sendTM()
{
	int TMessage[23];
	unsigned char SLIP_TM[186]; /* Max. theoretical size: 2+23*4*2 = 186 bytes */
	int slen = 0;
	
	/* 1. Assemble TM */
	bzero(TMessage, sizeof(TMessage)); /* zero message */
	
	TMessage[0]  = dict_getValue( DICT_TIME_SEC );
	TMessage[1]  = dict_getValue( DICT_TIME_MSEC );
	TMessage[2]  = dict_getValue( DICT_STATE );
	TMessage[3]  = dict_getValue( DICT_XLOC );
	TMessage[4]  = dict_getValue( DICT_YLOC );
	TMessage[5]  = dict_getValue( DICT_ALTITUDE );
	TMessage[6]  = dict_getValue( DICT_XVEL );
	TMessage[7]  = dict_getValue( DICT_YVEL );
	TMessage[8]  = dict_getValue( DICT_ZVEL );
	TMessage[9]  = dict_getValue( DICT_XINCL_201 );
	TMessage[10] = dict_getValue( DICT_YINCL_201 );
	TMessage[11] = 0; /* reserved */
	TMessage[12] = dict_getValue( DICT_CURR_TEMP );
	TMessage[13] = dict_getValue( DICT_CURR_PRESSURE );
	TMessage[14] = dict_getValue( DICT_LIGHT_SENS_1 );
	TMessage[15] = dict_getValue( DICT_LIGHT_SENS_2 );
	TMessage[16] = dict_getValue( DICT_GPS_LAT );
	TMessage[17] = dict_getValue( DICT_GPS_LONG );
	TMessage[18] = dict_getValue( DICT_GPS_ALT );
	TMessage[19] = dict_getValue( DICT_GPS_NUMSATS );
	TMessage[20] = dict_getValue( DICT_GPS_LOCKED );
	TMessage[21] = 0; /* Misc event word */
	TMessage[22] = 0x12345678; /* Should be CRC32 */ 
	
	/* 2. Convert to SLIP */
	slen = convertToSLIP(SLIP_TM, (unsigned char *)TMessage, 92);

	log_printf( DEBUG, "AEROCOMM", "Downlinking %d bytes.\n", slen );	
	/* 3. Send TM */
	write(ac_FileDesc, SLIP_TM, slen);
	
	return 0;
} /* aerocomm_sendTM */


/*** End of File ***/

