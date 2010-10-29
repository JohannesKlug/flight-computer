#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

/*
  This program reads one byte at a time from an Aerocomm device. 
  2010-10-29 Johannes Klug (johannes.klug@logica.com)
 */
int main()
{
  int fd;
  struct termios newtio;
  //unsigned char aero_tx[6] = {'A','T','+','+','+','\r'};
 // unsigned char ee[] = { 0xCC, 0xC1, 0xC1, 0x01, 0x90 };
 // unsigned char ee[] = { 0xCC, 0x41, 0x54, 0x4F, 0x0D };
//  unsigned char rx[1];
  unsigned char aero_tx[1] = {0x60};

  fd = open("/dev/ttyTS1", O_RDWR | O_NOCTTY | O_NDELAY);
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
//  write(fd, aero_tx, 6);
//  sleep(1);       //1 sec guard time
  while(1) {
    write(fd, aero_tx, 1);
    sleep(1);       //1 sec guard time
  }
/*
  if(read(fd, rx, 4) == 4) {
    if(rx[0]==0xCC && rx[1]==0x43 && rx[2]==0x4F && rx[3]==0x4D) {
      printf("detected aerocomm radio, initializing...\n");
      write(fd,ee,5);
      sleep(1);
      if(read(fd, rx, 4) == 4) {
        if(rx[0]==0xCC && rx[1]==0x44 && rx[2]==0x41 && rx[3]==0x54) {
          printf("Succesfully left commandmode. Device is ready to use now.\n");
          close(fd);
          return 0;
        } else {
          printf("Received something strange while trying to leave command mode.");
          close(fd);
          return 1;
        }
      } else {
        printf("Didn't read 4 bytes in reply to the Exit AT Command Mode command. Something is wrong.");
        close(fd);
        return 1;
      }


 //     return 0;
    }
  }
*/
  close(fd);
  return 0;
}

