/*** interface functions for aerocomm communications ***/

#ifndef _AEROCOMM_H
#define _AEROCOMM_H

#define SERIALPORT_AEROCOMM "/dev/ttyS3"
#define BAUDRATE_AEROCOMM B57600

int aerocomm_open();
int aerocomm_sendTM();

#endif /* _AEROCOMM_H */

/*** End of file ***/

