/************************************************************************
	HardwareProfile.h

	WFF USB Generic HID Demonstration 3
    usbGenericHidCommunication reference firmware 3_0_0_0
    Copyright (C) 2011 Simon Inns

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Email: simon.inns@gmail.com

************************************************************************/

#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

// USB stack hardware selection options ----------------------------------------------------------------

// (This section is the set of definitions required by the MCHPFSUSB framework.)

// Uncomment the following define if you wish to use the self-power sense feature 
// and define the port, pin and tris for the power sense pin below:
#define tris_self_power     TRISAbits.TRISA2
#define self_power          1

// Uncomment the following define if you wish to use the bus-power sense feature 
// and define the port, pin and tris for the power sense pin below:
#define tris_usb_bus_sense  TRISAbits.TRISA1
#define USB_BUS_SENSE       1

// Uncomment the following line to make the output HEX of this project work with the HID Bootloader
#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER		

// Application specific hardware definitions ------------------------------------------------------------

// Oscillator frequency (48Mhz with a 20Mhz external oscillator)
#define CLOCK_FREQ 48000000

// Device Vendor Identifier
#define USB_VID	0xbeef

// Device Product Identifier (PID)
#define USB_PID	0xf00d

// Manufacturer string descriptor
#define MSDLENGTH	13
#define MSD		'S','t','e','v','a','r','m','i','n','c', 'h', 'e', 't'

// Product String descriptor
#define PSDLENGTH	14
#define PSD		'F','A','T',' ','P','A','N','T','A','L','O','O','N','S'

// Device serial number string descriptor
#define DSNLENGTH	7
#define DSN		'F','A','T','P','A','N','T'

// Common useful definitions
#define INPUT_PIN 1
#define OUTPUT_PIN 0
#define FLAG_FALSE 0
#define FLAG_TRUE 1

// Led control macros
#define mInitStatusLeds()		LATA &= 0b00000001; TRISA &= 0b00000001;
#define mStatusLED0				LATAbits.LATA0
#define mStatusLED0_on()		mStatusLED0 = 1;
#define mStatusLED0_off()		mStatusLED0 = 0;
#define mStatusLED0_Toggle()	mStatusLED0 = !mStatusLED0;
#define mStatusLED0_Get()       mStatusLED0

#endif
