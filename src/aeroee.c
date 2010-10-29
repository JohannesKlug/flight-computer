#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

/*
  Perform default initialization for Aerocomm module.
  This basically sets up the module for transparent serial with any other unit.
 */
int main()
{
  int fd;
  struct termios newtio;
  unsigned char aero_tx[6] = {'A','T','+','+','+','\r'};
  unsigned char ee[] = { 0xCC, 0xC1, 0xC1, 0x01, 0x90 };
  unsigned char rx[4];

  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd < 0) {
    perror("Unable to open device, ensure drivers are loaded\n");
    return 1;
  }

 
  /* check if an aero radio is connected */
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = B57600 | CS8 | CLOCAL | CREAD;
  tcflush(fd, TCIOFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);

  sleep(1);       //1 sec guard time
  write(fd, aero_tx, 6);
  sleep(1);       //1 sec guard time

  if(read(fd, rx, 4) == 4) {
    if(rx[0]==0xCC && rx[1]==0x43 && rx[2]==0x4F && rx[3]==0x4D) {
      printf("detected aerocomm radio, initializing...\n");
      write(fd,ee,5);
      sleep(1);
      return;
    }
  }
  close(fd);
  return 0;
}

