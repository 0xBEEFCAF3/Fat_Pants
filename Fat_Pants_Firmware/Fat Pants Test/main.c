/************************************************************************
	main.c

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

#ifndef MAIN_C
#define MAIN_C

// Local includes
#include "HardwareProfile.h"
#include "./debug.h"
#include <p18f2450.h>

#include "./usb.h"
#include "./usb_function_hid.h"
// Ensure we have the correct target PIC device family
#if !defined(__18F2450) && !defined(__18F4450)
	#error "This firmware only supports either the PIC18F4450 or PIC18F2450 microcontrollers."
#endif

// Define the globals for the USB data in the USB RAM of the PIC18F*550
#pragma udata
#pragma udata USB_VARIABLES=0x480
unsigned char ReceivedDataBuffer[64];
unsigned char ToSendDataBuffer[64];
#pragma udata

USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;
BOOL blinkStatusValid = FLAG_TRUE;

// PIC18F4450/PIC18F2450 configuration for the WFF Generic HID test device
#pragma config PLLDIV   = 5         // 20Mhz external oscillator
#pragma config CPUDIV   = OSC1_PLL2   
#pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
#pragma config FOSC     = HSPLL_HS
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = OFF
#pragma config BOR      = ON
#pragma config BORV     = 21 // 2.1v brown-out voltage
#pragma config VREGEN   = ON
#pragma config WDT      = OFF // no watch dog
#pragma config WDTPS    = 32768
#pragma config MCLRE    = ON
#pragma config LPT1OSC  = OFF
#pragma config PBADEN   = OFF
#pragma config STVREN   = ON
#pragma config LVP      = OFF
#pragma config XINST    = OFF
#pragma config CP0      = OFF
#pragma config CP1      = OFF
#pragma config CPB      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
#pragma config WRTB     = OFF
#pragma config WRTC     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
#pragma config EBTRB    = OFF

// Private function prototypes
static void initialisePic(void);
void processUsbCommands(void);
void applicationInit(void);
void USBCBSendResume(void);
void highPriorityISRCode();
void lowPriorityISRCode();
void setLedOneDimness(unsigned char pwm);

// Remap vectors for compatibilty with Microchip USB boot loaders
#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018

extern void _startup (void);
#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
void _reset (void)
{
    _asm goto _startup _endasm
}

#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
void Remapped_High_ISR (void)
{
     _asm goto highPriorityISRCode _endasm
}

#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
void Remapped_Low_ISR (void)
{
     _asm goto lowPriorityISRCode _endasm
}

#pragma code HIGH_INTERRUPT_VECTOR = 0x08
void High_ISR (void)
{
     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
}

#pragma code LOW_INTERRUPT_VECTOR = 0x18
void Low_ISR (void)
{
     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
}

#pragma code

// High-priority ISR handling function
#pragma interrupt highPriorityISRCode
void highPriorityISRCode()
{
	// Application specific high-priority ISR code goes here
}

// Low-priority ISR handling function
#pragma interruptlow lowPriorityISRCode
void lowPriorityISRCode()
{
	// Application specific low-priority ISR code goes here
}

void main(void)
{   
    initialisePic();

    mStatusLED0_on();
    setLedOneDimness(0x64);
    while(1);
//    while(1)
//    {
//        #if defined(USB_POLLING)
//			// If we are in polling mode the USB device tasks must be processed here
//			// (otherwise the interrupt is performing this task)
//	        USBDeviceTasks();
//        #endif
//    	
//        processUsbCommands();  
//    }
}

static void initialisePic(void)
{
	// Default all pins to digital
    ADCON1 = 0xFF;

	// Configure all ports as outputs
	TRISA = 0b00000000;
	TRISB = 0b00000000;
	TRISC = 0b00000000;

	// Clear all ports
	PORTA = 0b00000000;
	PORTB = 0b00000000;
	PORTC = 0b00000000;

    applicationInit();
    USBDeviceInit();
}

// Application specific device initialization
void applicationInit(void)
{
	// Initialize the status LEDs
	mInitStatusLeds();
	
    // Initialize the variable holding the USB handle for the last transmission
    USBOutHandle = 0;
    USBInHandle = 0;
}

void processUsbCommands(void)
{   
    // Check if we are in the configured state; otherwise just return
    if((USBDeviceState < CONFIGURED_STATE) || (USBSuspendControl == 1))
    {
	    return;
	}

	// Check if data was received from the host.
    if(!HIDRxHandleBusy(USBOutHandle))
    {   
		// Command mode    
        switch(ReceivedDataBuffer[0])
		{
            case 0x80:
				mStatusLED0_Toggle();
            	break;
            
            case 0x81: //Get PWM
                //we should expect an unsigned char on the recieve buffer
                //Something between 0-100
                //setLedOneDimness(ReceivedDataBuffer[1]); //Our command we send should be something like : 0x81 0xnn
                break;
                
            case 0x82:
				// Get the LED state and put it in the send buffer
				ToSendDataBuffer[0] = mStatusLED0_Get();
				
				// Transmit the response to the host
                if(!HIDTxHandleBusy(USBInHandle))
				{
					USBInHandle = HIDTxPacket(HID_EP,(BYTE*)&ToSendDataBuffer[0],64);
				}
            	break;

            default:	// Unknown command received
           		break;
		}
		 
        // Re-arm the OUT endpoint for the next packet
        USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&ReceivedDataBuffer,64);
  	}
}

// USB Callback handling routines -----------------------------------------------------------

//void __attribute__ ((__interrupt__, no_auto_psv)) _T0Interrupt(){
//    //mStatusLED0_Toggle();
//    mStatusLED0_off();
//    //flag
//    INTCONbits.TMR0IF = 0;
//}


void interrupt isr(void){
    if(INTCONbits.T0IF == 1){
        mStatusLED0_off();    
    }
}

void setLedOneDimness(unsigned char pwm){
    
    T0CONbits.TMR0ON = 0;//turn off timer
    T0CONbits.T08BIT = 1; //8 bit timer couter
    T0CONbits.T0CS = 0; //internal clock source
    T0CONbits.T0SE = 1; 
    T0CONbits.PSA = 0; //Declare we are using a prescaler 
    //todo clear timer value
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    TMR0H = 0;
    TMR0L = 0;
    RCONbits.IPEN = 0;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;


    T0CONbits.T0PS2 = 0b111; //256 prescaler
    
    //lastly turn on the timer
    T0CONbits.TMR0ON = 1;//turn on timer

}



// Call back that is invoked when a USB suspend is detected
void USBCBSuspend(void)
{
}

// This call back is invoked when a wakeup from USB suspend is detected.
void USBCBWakeFromSuspend(void)
{
}

// The USB host sends out a SOF packet to full-speed devices every 1 ms.
void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here. Callback caller is already doing that.
}

// The purpose of this callback is mainly for debugging during development.
// Check UEIR to see which error causes the interrupt.
void USBCBErrorHandler(void)
{
    // No need to clear UEIR to 0 here.
    // Callback caller is already doing that.
}

// Check other requests callback
void USBCBCheckOtherReq(void)
{
    USBCheckHIDRequest();
}

// Callback function is called when a SETUP, bRequest: SET_DESCRIPTOR request arrives.
void USBCBStdSetDscHandler(void)
{
    // You must claim session ownership if supporting this request
}

//This function is called when the device becomes initialized
void USBCBInitEP(void)
{
    // Enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    
    // Re-arm the OUT endpoint for the next packet
    USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&ReceivedDataBuffer,64);
}

// Send resume call-back
void USBCBSendResume(void)
{
    static WORD delay_count;
    
    // Verify that the host has armed us to perform remote wakeup.
    if(USBGetRemoteWakeupStatus() == FLAG_TRUE) 
    {
        // Verify that the USB bus is suspended (before we send remote wakeup signalling).
        if(USBIsBusSuspended() == FLAG_TRUE)
        {
            USBMaskInterrupts();
            
            // Bring the clock speed up to normal running state
            USBCBWakeFromSuspend();
            USBSuspendControl = 0; 
            USBBusIsSuspended = FLAG_FALSE;

            // Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            // device must continuously see 5ms+ of idle on the bus, before it sends
            // remote wakeup signalling.  One way to be certain that this parameter
            // gets met, is to add a 2ms+ blocking delay here (2ms plus at 
            // least 3ms from bus idle to USBIsBusSuspended() == FLAG_TRUE, yeilds
            // 5ms+ total delay since start of idle).
            delay_count = 3600U;        
            do
            {
                delay_count--;
            } while(delay_count);
            
            // Start RESUME signaling for 1-13 ms
            USBResumeControl = 1;
            delay_count = 1800U;
            do
            {
                delay_count--;
            } while(delay_count);
            USBResumeControl = 0;

            USBUnmaskInterrupts();
        }
    }
}

// USB callback function handler
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_TRANSFER:
            // Application callback tasks and functions go here
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        default:
            break;
    }      
    return FLAG_TRUE; 
}

#endif
