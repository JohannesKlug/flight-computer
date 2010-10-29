#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include "aerosetup.h"

/*
  Perform default initialisation for Aerocomm module.
  The first version of this code was taken from here:
  ftp://ftp.embeddedarm.com/pc104-peripherals/ts-rf2/ts-72xx-linux24/aeroee.c
 
  It proved to be incorrect, so I created this modified version.
  It is aimed for the TS-RF2 module with an Aerocomm AC4868-250M module attached.
  
  2010-10-29 Johannes Klug (johannes.klug@logica.com)
 */

int fd;

int main()
{
  //int fd;
  struct termios newtio;
  unsigned char aero_tx[6] = {'A','T','+','+','+','\r'};

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

  unsigned char rx[4];
  if(read(fd, rx, 4) == 4) {
    if(rx[0]==0xCC && rx[1]==0x43 && rx[2]==0x4F && rx[3]==0x4D) {
      printf("detected aerocomm radio, initializing...\n");
      printf("Querying device status ...\n");
      sleep(1);
      
      char data[2];
      readStatus(data);

        printf("Firmware version: ");
        printf("%x\n",(int)data[0]);
        if (data[1] == 0x00) { 
          printf("Aerocomm unit in Server mode\n");
        } else if (data[1] == 0x01) {
          printf("Client in range\n");
        } else if (data[1] == 0x03) {
          printf("Client out of range\n");
        } else {
          putchar(data[1]);
          printf("\nIllegal response to status query command received, exiting ...\n");
          close(fd);
          return 1;
        }
     

      printf("Temperature (in hex) is: ");
      sleep(1);
      char temperature = readTemperature();
      printf("%x\n",(int)temperature);
      sleep(1);

      exitCommandMode();
      
    }
  }
  close(fd);
  return 0;
}

char readTemperature() {
  unsigned char tx[2] = { 0xCC, 0xA4};
  unsigned char rx[2];
  write(fd, tx, 2);
  sleep(1);
  if(read(fd, rx, 2) == 2) {
    if(rx[0] != 0xCC) {
      printf("Didn't receive a valid response to Status Request, exiting ...\n");
      return -1;
    }
    return rx[1];
  }
}

int exitCommandMode() {
  unsigned char ee[] = { 0xCC, 0x41, 0x54, 0x4F, 0x0D };
  unsigned char rx[4];
  write(fd,ee,5);
  sleep(1);
  if(read(fd, rx, 4) == 4) {
    if(rx[0]==0xCC && rx[1]==0x44 && rx[2]==0x41 && rx[3]==0x54) {
      printf("Succesfully left commandmode. Device is ready to use now.\n");
      return 1;
    } else {
      printf("Received something strange while trying to leave command mode.\n");
      return 0;
    }
  } else {
    printf("Didn't read 4 bytes in response to the Exit AT Command Mode command. Something is wrong.\n");
    return 0;
  }
}

int readStatus(char* data) {
  unsigned char status_command[3] = { 0xCC, 0x00, 0x00 };
  unsigned char status_rx[3];
  write(fd, status_command, 3);
  sleep(1);
  if(read(fd, status_rx, 3) == 3) {
    if(status_rx[0] != 0xCC) {
      printf("Didn't receive a valid response to Status Request, exiting ...\n");
      return 0;
    } else {
//      char data[2];
      data[0] = status_rx[1];
      data[1] = status_rx[2];
      return 1;
    }
  } else {
    printf("Didn't read 3 bytes in response to the Status Request. Something is wrong.\n");
    return 0;
  }
}
