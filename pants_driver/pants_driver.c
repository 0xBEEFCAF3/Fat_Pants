#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libusb-1.0/libusb.h"

int main(void)
{
  char read[4];
  int dim_value_int;
  int r;
  int transferred = 0;
  libusb_device_handle* belt;
  unsigned char dimming_bytes[2] = {0x81, 0x00};

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
  libusb_detach_kernel_driver(belt, 0);
  if (libusb_claim_interface(belt, 0) < 0){
    printf("failed to claim interface\n");
    return 1;
  }
  printf("the belt has been secured!\n");
  printf("Type a decimal number between 0-255 (inclusive) to illuminate the pants.\n");
  printf("[Ctrl-c] to exit the pants.\n");


  while (1){
  	fgets(read, 5, stdin);
	dim_value_int = atoi(read);
	if ((dim_value_int >= 0) && (dim_value_int <= 255)){
		dimming_bytes[1] = (unsigned char) dim_value_int;
		libusb_interrupt_transfer(belt, 1, dimming_bytes, 2, &transferred, 0);
	}
	else{
		printf("Please enter decimal numbers between 0-255 (inclusive)\n");
	}
  }

  printf("Thank you for trying the pants!\n");
  libusb_exit(NULL);
  return 0;
}