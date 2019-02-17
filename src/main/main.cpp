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

void  print_hex(int hex_value){
	printf("%x",hex_value);
};


void uhs_complete_test(void *pvParameter){ 
	
	/* variables */
	uint8_t rcode;
	uint8_t usbstate;
	uint8_t laststate = 0;
	//uint8_t buf[sizeof(USB_DEVICE_DESCRIPTOR)];
	USB_DEVICE_DESCRIPTOR buf;

   
	USB Usb;
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
        printf("USB State : %x\n",usbstate );
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
                                        /**/
                                        printf("\r\n\nAll tests passed. Press RESET to restart test");
                                        while(1) {
                                                yield(); // needed in order to reset the watchdog timer on the ESP8266
                                        }
                                }
                                break;
                        case( USB_STATE_ERROR):
                                printf("\r\nUSB state machine reached error state");
                                break;

                        default:
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
