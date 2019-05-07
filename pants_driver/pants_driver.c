#include <stdio.h>
#include <stdlib.h>
#include "libusb-1.0/libusb.h"

int main(void)
{
  int r;
  libusb_device_handle* belt;

  r = libusb_init(NULL);
  if (r < 0)
    return r;
  printf("Locating belt... ");
  belt = libusb_open_device_with_vid_pid(NULL, 0xbeef, 0xf00d);
  if (!belt){
    printf("failed to locate belt, have you tried sudo?\n");
    return 1;
  }
  printf("belt located!\n");
  printf("Trying to fasten belt... ");
  if (libusb_claim_interface(belt, 0) < 0){
    printf("failed to claim interface\n");
    return 1;
  }
  printf("the belt has been secured!\n");
  printf("Press [Enter] to toggle pants, or send [e] to exit the pants.\n");
  unsigned char toggle_byte = 0x80;
  int transferred = 0;
  char read = 0;

  while ((read = getc(stdin)) != 'e')
          libusb_interrupt_transfer(belt, 1, &toggle_byte, 1, &transferred, 0);

  printf("Thank you for trying the pants!\n");
  libusb_exit(NULL);
  return 0;
}