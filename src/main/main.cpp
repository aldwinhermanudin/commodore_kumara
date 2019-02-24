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

void uhs_complete_test(void *pvParameter){ 
	
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
				while(1);
				break;
		}//switch( tmpbyte...
	}//check revision register
     		
	while (1){
		//vTaskDelay(100 / 100);
		Usb.Task();
	}
}


void app_main(){
	
    xTaskCreate(&uhs_complete_test, "uhs_complt_test", 4096, NULL, configMAX_PRIORITIES, NULL);
}
