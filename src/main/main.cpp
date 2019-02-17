/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * Copyright 2017 Benjamin Aigner <beni@asterics-foundation.org>
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "driver/gpio.h"
#include "Greeting.h"
#include "Max3421e.h"
#include "Usb.h"
#include "hiduniversal.h"
//#include "Test.h"

#include "config.h"
#include "HID_kbdmousejoystick.h"

#include "esp_gap_ble_api.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "keyboard.h"

#ifndef ESP32
#define ESP32
#endif


#define BUFSIZE 256    //buffer size
#define LOBYTE(x) ((char*)(&(x)))[0]
#define HIBYTE(x) ((char*)(&(x)))[1]



/* Print strings in Program Memory */
const char Gen_Error_str[] PROGMEM = "\r\nRequest error. Error code:\t"; 
const char Dev_Header_str[] PROGMEM ="\r\nDevice descriptor: ";
const char Dev_Length_str[] PROGMEM ="\r\nDescriptor Length:\t";
const char Dev_Type_str[] PROGMEM ="\r\nDescriptor type:\t";
const char Dev_Version_str[] PROGMEM ="\r\nUSB version:\t\t";
const char Dev_Class_str[] PROGMEM ="\r\nDevice class:\t\t";
const char Dev_Subclass_str[] PROGMEM ="\r\nDevice Subclass:\t";
const char Dev_Protocol_str[] PROGMEM ="\r\nDevice Protocol:\t";
const char Dev_Pktsize_str[] PROGMEM ="\r\nMax.packet size:\t";
const char Dev_Vendor_str[] PROGMEM ="\r\nVendor  ID:\t\t";
const char Dev_Product_str[] PROGMEM ="\r\nProduct ID:\t\t";
const char Dev_Revision_str[] PROGMEM ="\r\nRevision ID:\t\t";
const char Dev_Mfg_str[] PROGMEM ="\r\nMfg.string index:\t";
const char Dev_Prod_str[] PROGMEM ="\r\nProd.string index:\t";
const char Dev_Serial_str[] PROGMEM ="\r\nSerial number index:\t";
const char Dev_Nconf_str[] PROGMEM ="\r\nNumber of conf.:\t";
const char Conf_Trunc_str[] PROGMEM ="Total length truncated to 256 bytes";
const char Conf_Header_str[] PROGMEM ="\r\nConfiguration descriptor:";
const char Conf_Totlen_str[] PROGMEM ="\r\nTotal length:\t\t";
const char Conf_Nint_str[] PROGMEM ="\r\nNum.intf:\t\t";
const char Conf_Value_str[] PROGMEM ="\r\nConf.value:\t\t";
const char Conf_String_str[] PROGMEM ="\r\nConf.string:\t\t";
const char Conf_Attr_str[] PROGMEM ="\r\nAttr.:\t\t\t";
const char Conf_Pwr_str[] PROGMEM ="\r\nMax.pwr:\t\t";
const char Int_Header_str[] PROGMEM ="\r\n\r\nInterface descriptor:";
const char Int_Number_str[] PROGMEM ="\r\nIntf.number:\t\t";
const char Int_Alt_str[] PROGMEM ="\r\nAlt.:\t\t\t";
const char Int_Endpoints_str[] PROGMEM ="\r\nEndpoints:\t\t";
const char Int_Class_str[] PROGMEM ="\r\nIntf. Class:\t\t";
const char Int_Subclass_str[] PROGMEM ="\r\nIntf. Subclass:\t\t";
const char Int_Protocol_str[] PROGMEM ="\r\nIntf. Protocol:\t\t";
const char Int_String_str[] PROGMEM ="\r\nIntf.string:\t\t";
const char End_Header_str[] PROGMEM ="\r\n\r\nEndpoint descriptor:";
const char End_Address_str[] PROGMEM ="\r\nEndpoint address:\t";
const char End_Attr_str[] PROGMEM ="\r\nAttr.:\t\t\t";
const char End_Pktsize_str[] PROGMEM ="\r\nMax.pkt size:\t\t";
const char End_Interval_str[] PROGMEM ="\r\nPolling interval:\t";
const char Unk_Header_str[] PROGMEM = "\r\nUnknown descriptor:";
const char Unk_Length_str[] PROGMEM ="\r\nLength:\t\t";
const char Unk_Type_str[] PROGMEM ="\r\nType:\t\t";
const char Unk_Contents_str[] PROGMEM ="\r\nContents:\t";
 
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif
#ifndef pgm_read_float
#define pgm_read_float(addr) (*(const float *)(addr))
#endif

#ifndef pgm_read_byte_near
#define pgm_read_byte_near(addr) pgm_read_byte(addr)
#endif
#ifndef pgm_read_word_near
#define pgm_read_word_near(addr) pgm_read_word(addr)
#endif
#ifndef pgm_read_dword_near
#define pgm_read_dword_near(addr) pgm_read_dword(addr)
#endif
#ifndef pgm_read_float_near
#define pgm_read_float_near(addr) pgm_read_float(addr)
#endif
#ifndef pgm_read_byte_far
#define pgm_read_byte_far(addr) pgm_read_byte(addr)
#endif
#ifndef pgm_read_word_far
#define pgm_read_word_far(addr) pgm_read_word(addr)
#endif
#ifndef pgm_read_dword_far
#define pgm_read_dword_far(addr) pgm_read_dword(addr)
#endif
#ifndef pgm_read_float_far
#define pgm_read_float_far(addr) pgm_read_float(addr)
#endif

#ifndef pgm_read_pointer
#define pgm_read_pointer
#endif




class Shape {
	
	public :
		void Init(int length, int width, int height){
			this->length = length;
			this->width = width;
			this->height = height;
		}
		
		int length;
		int width;
		int height;
	};
	
class Box : public Shape{
	
	public:
		Box(int length, int width, int height){
			Init(length, width, height);
		}
		virtual void test_method();
};


class HIDUniversal2 : public HIDUniversal
{
public:
    HIDUniversal2(USB *usb) : HIDUniversal(usb) {};

protected:
    uint8_t OnInitSuccessful();
};

uint8_t HIDUniversal2::OnInitSuccessful()
{ 
    printf("rcode : init success\n");

    return 0;
}



extern "C" void app_main();

void uhs_test(void *pvParameter){ 
   
	USB Usb;
	Usb.Init();
	vTaskDelay(100 / 100);
	
	while (1){ 
		uint8_t a = 0;
		vTaskDelay(100 / 100);
		
		a = Usb.regRd(rREVISION);
		
		printf ("Revision Number : %x\n", a);
		
	}
}

void halt55() {

        printf("\r\nUnrecoverable error - test halted!!\n");
        printf("\r\n0x55 pattern is transmitted via SPI\n");
		printf("\r\nPress RESET to restart test\n");

        while(1) {
                //Usb.regWr(0x55, 0x55); 
                vTaskDelay(100 / 100);
        }
}


/* Print a string from Program Memory directly to save RAM */
void printProgStr(const char* str)
{
  char c;
  if (!str) return;
  while ((c = pgm_read_byte(str++)))
    printf("%c",c);
}

void  print_hex(int hex_value){
	printf("%x",hex_value);
};


uint8_t isGetConfDescrSuccess(USB *usb, uint8_t addr, uint8_t conf )
{

  uint8_t buf[ BUFSIZE ]; 
  uint8_t rcode; 
  uint16_t total_length;
  rcode = usb->getConfDescr( addr, 0, 4, conf, buf );  //get total length
  LOBYTE( total_length ) = buf[ 2 ];
  HIBYTE( total_length ) = buf[ 3 ];
  if ( total_length > 256 ) {   //check if total length is larger than buffer
    printf("Total length truncated to 256 bytes");
    total_length = 256;
  }
  return ( rcode );
}

/* function to print configuration descriptor */
void printconfdescr( uint8_t* descr_ptr )
{
  USB_CONFIGURATION_DESCRIPTOR* conf_ptr = ( USB_CONFIGURATION_DESCRIPTOR* )descr_ptr;
  printProgStr(Conf_Header_str);
  printProgStr(Conf_Totlen_str);
  print_hex( conf_ptr->wTotalLength);
  printProgStr(Conf_Nint_str);
  print_hex( conf_ptr->bNumInterfaces);
  printProgStr(Conf_Value_str);
  print_hex( conf_ptr->bConfigurationValue );
  printProgStr(Conf_String_str);
  print_hex( conf_ptr->iConfiguration);
  printProgStr(Conf_Attr_str);
  print_hex( conf_ptr->bmAttributes);
  printProgStr(Conf_Pwr_str);
  print_hex( conf_ptr->bMaxPower);
  return;
}
/* function to print interface descriptor */
void printintfdescr( uint8_t* descr_ptr )
{
  USB_INTERFACE_DESCRIPTOR* intf_ptr = ( USB_INTERFACE_DESCRIPTOR* )descr_ptr;
  printProgStr(Int_Header_str);
  printProgStr(Int_Number_str);
  print_hex( intf_ptr->bInterfaceNumber );
  printProgStr(Int_Alt_str);
  print_hex( intf_ptr->bAlternateSetting);
  printProgStr(Int_Endpoints_str);
  print_hex( intf_ptr->bNumEndpoints );
  printProgStr(Int_Class_str);
  print_hex( intf_ptr->bInterfaceClass);
  printProgStr(Int_Subclass_str);
  print_hex( intf_ptr->bInterfaceSubClass);
  printProgStr(Int_Protocol_str);
  print_hex( intf_ptr->bInterfaceProtocol);
  printProgStr(Int_String_str);
  print_hex( intf_ptr->iInterface );
  return;
}
/* function to print endpoint descriptor */
void printepdescr( uint8_t* descr_ptr )
{
  USB_ENDPOINT_DESCRIPTOR* ep_ptr = ( USB_ENDPOINT_DESCRIPTOR* )descr_ptr;
  printProgStr(End_Header_str);
  printProgStr(End_Address_str);
  print_hex( ep_ptr->bEndpointAddress );
  printProgStr(End_Attr_str);
  print_hex( ep_ptr->bmAttributes);
  printProgStr(End_Pktsize_str);
  print_hex( ep_ptr->wMaxPacketSize );
  printProgStr(End_Interval_str);
  print_hex( ep_ptr->bInterval );

  return;
}
/*function to print unknown descriptor */
void printunkdescr( uint8_t* descr_ptr )
{
  uint8_t length = *descr_ptr;
  uint8_t i;
  printProgStr(Unk_Header_str);
  printProgStr(Unk_Length_str);
  print_hex( *descr_ptr );
  printProgStr(Unk_Type_str);
  print_hex( *(descr_ptr + 1 ) );
  printProgStr(Unk_Contents_str);
  descr_ptr += 2;
  for ( i = 0; i < length; i++ ) {
    print_hex( *descr_ptr);
    descr_ptr++;
  }
}



uint8_t getconfdescr(USB *usb, uint8_t addr, uint8_t conf )
{
  uint8_t buf[ BUFSIZE ];
  uint8_t* buf_ptr = buf;
  uint8_t rcode;
  uint8_t descr_length;
  uint8_t descr_type;
  uint16_t total_length;
  rcode = usb->getConfDescr( addr, 0, 4, conf, buf );  //get total length
  LOBYTE( total_length ) = buf[ 2 ];
  HIBYTE( total_length ) = buf[ 3 ];
  if ( total_length > 256 ) {   //check if total length is larger than buffer
    printf("Total length truncated to 256 bytes");
    total_length = 256;
  }
  rcode = usb->getConfDescr( addr, 0, total_length, conf, buf ); //get the whole descriptor
  while ( buf_ptr < buf + total_length ) { //parsing descriptors
    descr_length = *( buf_ptr );
    descr_type = *( buf_ptr + 1 );
    switch ( descr_type ) {
      case ( USB_DESCRIPTOR_CONFIGURATION ):
        printconfdescr( buf_ptr );
        break;
      case ( USB_DESCRIPTOR_INTERFACE ):
        printintfdescr( buf_ptr );
        break;
      case ( USB_DESCRIPTOR_ENDPOINT ):
        printepdescr( buf_ptr );
        break;
      case 0x29:
        printf("No Hub descriptor handler");
        break;
      default:
        printunkdescr( buf_ptr );
        break;
    }//switch( descr_type
    buf_ptr = ( buf_ptr + descr_length );    //advance buffer pointer
  }//while( buf_ptr <=...
  return ( rcode );
}


void uhs_complete_test(void *pvParameter){ 
	
	/* variables */
	uint8_t rcode;
	uint8_t usbstate;
	uint8_t laststate = 0;
	//uint8_t buf[sizeof(USB_DEVICE_DESCRIPTOR)];
	USB_DEVICE_DESCRIPTOR buf;

   
	USB Usb;
	HIDUniversal2 Hid(&Usb);
	Usb.Init();
	vTaskDelay(1000 / 100);
	{
		printf("\r\nReading REVISION register... Die revision ");
		uint8_t tmpbyte = Usb.regRd(rREVISION);
		switch(tmpbyte) {
			case( 0x01): //rev.01
				printf("01");
				break;
			case( 0x12): //rev.02
				printf("02");
				break;
			case( 0x13): //rev.03
				printf("03");
				break;
			default:
				printf("invalid. Value returned: %x", tmpbyte);
				halt55();
				break;
		}//switch( tmpbyte...
	}//check revision register
    
    /* SPI long test 
	{
		printf("\r\nSPI long test. Transfers 1MB of data. Each dot is 64K");
		uint8_t sample_wr = 0;
		uint8_t sample_rd = 0;
		uint8_t gpinpol_copy = Usb.regRd(rGPINPOL);
		for(uint8_t i = 0; i < 16; i++) { 
			for(uint16_t j = 0; j < 65535; j++) {
				Usb.regWr(rGPINPOL, sample_wr);
				sample_rd = Usb.regRd(rGPINPOL);
				if(sample_rd != sample_wr) {
					printf("\r\nTest failed.  ");
					halt55();
				}
				sample_wr++;
			}
			printf(".");
		}
		Usb.regWr(rGPINPOL, gpinpol_copy);
		printf(" SPI long test passed");
	}//SPI long test */
	
	{
		printf("\r\nPLL test. 100 chip resets will be performed");
		/* check current state of the oscillator */
		if(!(Usb.regRd(rUSBIRQ) & bmOSCOKIRQ)) { //wrong state - should be on
			printf("\r\nCurrent oscillator state unexpected.");
				halt55();
			}
		/* Restart oscillator */
		printf("\r\nResetting oscillator\r\n") ;
		for(uint16_t i = 0; i < 100; i++) { 
			printf("\rReset number ") ;
			printf("%d", i);
			Usb.regWr(rUSBCTL, bmCHIPRES); //reset
			if(Usb.regRd(rUSBIRQ) & bmOSCOKIRQ) { //wrong state - should be off
				printf("\r\nCurrent oscillator state unexpected.");
				halt55();
			}
			Usb.regWr(rUSBCTL, 0x00); //release from reset
			uint16_t j = 0;
			for(j = 1; j < 65535; j++) { //tracking off to on time
				if(Usb.regRd(rUSBIRQ) & bmOSCOKIRQ) {
					printf(" Time to stabilize - ");
					printf("%d",j); 
					printf(" cycles\r\n");
					break;
				}
			}//for( uint16_t j = 0; j < 65535; j++
			if(j == 0) {
				printf("PLL failed to stabilize");
				halt55();
			}
		}//for( uint8_t i = 0; i < 255; i++
		
		if(Usb.InitChip() == -1) {
                printf("\r\nOSCOKIRQ failed to assert");
                halt55();
        }
        printf("\r\nChecking USB device communication.\r\n");
        vTaskDelay(5000 / 100);

	}//PLL test
	
	/*
	while (1){ 
		uint8_t a = 0;
		vTaskDelay(100 / 100);
		
		a = Usb.regRd(rREVISION);
		
		printf ("Revision Number : %x\n", a);
		
	}
	*/
	
	while (1){
		vTaskDelay(200 / 100);
        Usb.Task();
        usbstate = Usb.getUsbTaskState();
        if(usbstate != laststate) {
                laststate = usbstate;
                /**/
                switch(usbstate) {
                        case( USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE):
                                printf("\r\nWaiting for device...");
                                break;
                        case( USB_ATTACHED_SUBSTATE_RESET_DEVICE):
                                printf("\r\nDevice connected. Resetting...");
                                break;
                        case( USB_ATTACHED_SUBSTATE_WAIT_SOF):
                                printf("\r\nReset complete. Waiting for the first SOF...");
                                break;
                        case( USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE):
                                printf("\r\nSOF generation started. Enumerating device...");
                                break;
                        case( USB_STATE_ADDRESSING):
                                printf("\r\nSetting device address...");
                                break;
                        case( USB_STATE_RUNNING):
                                printf("\r\nGetting device descriptor");
                                rcode = Usb.getDevDescr(1, 0, sizeof (USB_DEVICE_DESCRIPTOR), (uint8_t*) & buf);

                                if(rcode) {
                                        printf("\r\nError reading device descriptor. Error code ");
                                        print_hex(rcode);
                                } else {
                                        /**/
                                        printf("\r\nDescriptor Length:\t");
                                        print_hex(buf.bLength);
                                        printf("\r\nDescriptor type:\t");
                                        print_hex(buf.bDescriptorType);
                                        printf("\r\nUSB version:\t\t");
                                        print_hex(buf.bcdUSB);
                                        printf("\r\nDevice class:\t\t");
                                        print_hex(buf.bDeviceClass);
                                        printf("\r\nDevice Subclass:\t");
                                        print_hex(buf.bDeviceSubClass);
                                        printf("\r\nDevice Protocol:\t");
                                        print_hex(buf.bDeviceProtocol);
                                        printf("\r\nMax.packet size:\t");
                                        print_hex(buf.bMaxPacketSize0);
                                        printf("\r\nVendor  ID:\t\t");
                                        print_hex(buf.idVendor);
                                        printf("\r\nProduct ID:\t\t");
                                        print_hex(buf.idProduct);
                                        printf("\r\nRevision ID:\t\t");
                                        print_hex(buf.bcdDevice);
                                        printf("\r\nMfg.string index:\t");
                                        print_hex(buf.iManufacturer);
                                        printf("\r\nProd.string index:\t");
                                        print_hex(buf.iProduct);
                                        printf("\r\nSerial number index:\t");
                                        print_hex(buf.iSerialNumber);
                                        printf("\r\nNumber of conf.:\t");
                                        print_hex(buf.bNumConfigurations);
                                        
                                        rcode = isGetConfDescrSuccess(&Usb,1,0);
                                        if (rcode) {
										  printf("\r\nError reading conf descriptor. Error code ");
										  print_hex(rcode);
										} else {
										  printf("\r\nSuccess reading conf descriptor.");
										  getconfdescr(&Usb,1,0);
										}
                                        
                                        /**/
                                        while(1) {
												vTaskDelay(1000 / 100);
												printf("\r\n\nAll tests passed. Press RESET to restart test");
                                                yield(); // needed in order to reset the watchdog timer on the ESP8266
                                        }
                                }
                                break;
                        case( USB_STATE_ERROR):
                                printf("\r\nUSB state machine reached error state");
                                break;

                        default:
								printf("USB State : %s\n",Usb.printUSBState(usbstate) );
                                break;
                } 
        }
	}
}


void app_main(){
	
	Box kotak(3,2,2);
	printf("Volume : %d\n", kotak.length*kotak.width*kotak.height);
    // now start the tasks for proc	essing UART input and indicator LED  
    //xTaskCreate(&max_rev, "max_rev", 4096, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(&uhs_test, "uhs_test", 4096, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(&uhs_complete_test, "uhs_complt_test", 4096, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(&dumb_task, "dumb_task", 2048, NULL, configMAX_PRIORITIES, NULL);
}
