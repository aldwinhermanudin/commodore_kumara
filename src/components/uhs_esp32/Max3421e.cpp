/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
 */
/* MAX3421E-based USB Host Library header file */

#include "Max3421e.h"

uint8_t Max3421e::vbusState = 0;

/* constructor */
	
Max3421e::Max3421e() : Max3421e(MUC_SS, MUC_MOSI, MUC_MISO, MUC_CLK, MUC_INT){};	
Max3421e::Max3421e(uint8_t ss_pin,uint8_t mosi_pin, uint8_t miso_pin, uint8_t clk_pin, uint8_t interrupt_pin) {
	this->ss_pin = ss_pin;
	this-> mosi_pin = mosi_pin;
	this->miso_pin = miso_pin;
	this-> clk_pin = clk_pin;
	this-> interrupt_pin = interrupt_pin;
}

/* write single byte into MAX3421 register */
void Max3421e::regWr(uint8_t reg, uint8_t data) {
        ssLow();
        
		SPI::transferByte(reg | 0x02);
		SPI::transferByte(data);
        
        ssHigh();
        return;
}
/* multiple-byte write                            */

/* returns a pointer to memory position after last written */
uint8_t* Max3421e::bytesWr(uint8_t reg, uint8_t nbytes, uint8_t* data_p) {
	
        ssLow();
        
		SPI::transferByte(reg | 0x02);
        while(nbytes) {
                SPI::transferByte(*data_p);
                nbytes--;
                data_p++; // advance data pointer
        } 

        ssHigh();
        return ( data_p);
}
/* GPIO write                                           */
/*GPIO byte is split between 2 registers, so two writes are needed to write one byte */

/* GPOUT bits are in the low nibble. 0-3 in IOPINS1, 4-7 in IOPINS2 */
void Max3421e::gpioWr(uint8_t data) {
        regWr(rIOPINS1, data);
        data >>= 4;
        regWr(rIOPINS2, data);
        return;
}

/* single host register read    */
uint8_t Max3421e::regRd(uint8_t reg) {
	
        ssLow();
		
		SPI::transferByte(reg); 
        uint8_t rv = SPI::transferByte(0); // Send empty byte
        
        ssHigh();
        return (rv);
}
/* multiple-byte register read  */

/* returns a pointer to a memory position after last read   */
uint8_t* Max3421e::bytesRd(uint8_t reg, uint8_t nbytes, uint8_t* data_p) {
	
        ssLow();
        
        SPI::transferByte(reg);
        while(nbytes) {
            *data_p++ = SPI::transferByte(0);
            nbytes--;
        }

        ssHigh();
        
        return ( data_p);
}
/* GPIO read. See gpioWr for explanation */

/* GPIN pins are in high nibbles of IOPINS1, IOPINS2    */
uint8_t Max3421e::gpioRd() {
        uint8_t gpin = 0;
        gpin = regRd(rIOPINS2); //pins 4-7
        gpin &= 0xf0; //clean lower nibble
        gpin |= (regRd(rIOPINS1) >> 4); //shift low bits and OR with upper from previous operation.
        return ( gpin);
}

/* reset MAX3421E. Returns number of cycles it took for PLL to stabilize after reset
  or zero if PLL haven't stabilized in 65535 cycles */ 
uint16_t Max3421e::reset() {
        uint16_t i = 0;
        regWr(rUSBCTL, bmCHIPRES);
        regWr(rUSBCTL, 0x00);
        while(++i) {
                if((regRd(rUSBIRQ) & bmOSCOKIRQ)) {
                        break;
                }
        }
        return ( i);
}

/* initialize MAX3421E. Set Host mode, pullups, and stuff. Returns 0 if success, -1 if not */ 
int8_t Max3421e::Init() {
        // Moved here.
        // you really should not init hardware in the constructor when it involves locks.
        // Also avoids the vbus flicker issue confusing some devices.
        /* pin and peripheral setup */
        
        ESP32CPP::GPIO::setOutput((gpio_num_t) ss_pin);
        ssHigh();
                
        // SPI Init
        SPI::init(mosi_pin,miso_pin,clk_pin,ss_pin);
                
        ESP32CPP::GPIO::setInput((gpio_num_t) interrupt_pin);
        /* MAX3421E - full-duplex SPI, level interrupt */
        // GPX pin on. Moved here, otherwise we flicker the vbus.
        regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL));

        if(reset() == 0) { //OSCOKIRQ hasn't asserted in time
                return ( -1);
        }

        regWr(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST); // set pull-downs, Host

        regWr(rHIEN, bmCONDETIE | bmFRAMEIE); //connection detection

        /* check if device is connected */
        regWr(rHCTL, bmSAMPLEBUS); // sample USB bus
        while(!(regRd(rHCTL) & bmSAMPLEBUS)); //wait for sample operation to finish

        busprobe(); //check if anything is connected

        regWr(rHIRQ, bmCONDETIRQ); //clear connection detect interrupt
        regWr(rCPUCTL, 0x01); //enable interrupt pin

        return ( 0);
}

/* initialize MAX3421E. Set Host mode, pullups, and stuff. Returns 0 if success, -1 if not */ 
int8_t Max3421e::InitChip() {
        // Moved here.
        // you really should not init hardware in the constructor when it involves locks.
        // Also avoids the vbus flicker issue confusing some devices.
        /* pin and peripheral setup */
        /*
        ESP32CPP::GPIO::setOutput((gpio_num_t) ss_pin);
        ssHigh();
                
        // SPI Init
        SPI::init(mosi_pin,miso_pin,clk_pin,ss_pin);
                
        ESP32CPP::GPIO::setInput((gpio_num_t) interrupt_pin);
        */
        /* MAX3421E - full-duplex SPI, level interrupt */
        // GPX pin on. Moved here, otherwise we flicker the vbus.
        regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL));

        if(reset() == 0) { //OSCOKIRQ hasn't asserted in time
                return ( -1);
        }

        regWr(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST); // set pull-downs, Host

        regWr(rHIEN, bmCONDETIE | bmFRAMEIE); //connection detection

        /* check if device is connected */
        regWr(rHCTL, bmSAMPLEBUS); // sample USB bus
        while(!(regRd(rHCTL) & bmSAMPLEBUS)); //wait for sample operation to finish

        busprobe(); //check if anything is connected

        regWr(rHIRQ, bmCONDETIRQ); //clear connection detect interrupt
        regWr(rCPUCTL, 0x01); //enable interrupt pin

        return ( 0);
}

/* initialize MAX3421E. Set Host mode, pullups, and stuff. Returns 0 if success, -1 if not */ 
int8_t Max3421e::Init(int mseconds) {
        // Moved here.
        // you really should not init hardware in the constructor when it involves locks.
        // Also avoids the vbus flicker issue confusing some devices.
        /* pin and peripheral setup */
        ESP32CPP::GPIO::setOutput((gpio_num_t) ss_pin);
        ssHigh();
        
        // SPI Init
        SPI::init(mosi_pin,miso_pin,clk_pin,ss_pin);
        
        ESP32CPP::GPIO::setInput((gpio_num_t) interrupt_pin);
        /* MAX3421E - full-duplex SPI, level interrupt, vbus off */
        regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL | GPX_VBDET));

        if(reset() == 0) { //OSCOKIRQ hasn't asserted in time
                return ( -1);
        }

        // Delay a minimum of 1 second to ensure any capacitors are drained.
        // 1 second is required to make sure we do not smoke a Microdrive!
        if(mseconds < 1000) mseconds = 1000;
        vTaskDelay(mseconds /100);

        regWr(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST); // set pull-downs, Host

        regWr(rHIEN, bmCONDETIE | bmFRAMEIE); //connection detection

        /* check if device is connected */
        regWr(rHCTL, bmSAMPLEBUS); // sample USB bus
        while(!(regRd(rHCTL) & bmSAMPLEBUS)); //wait for sample operation to finish

        busprobe(); //check if anything is connected

        regWr(rHIRQ, bmCONDETIRQ); //clear connection detect interrupt
        regWr(rCPUCTL, 0x01); //enable interrupt pin

        // GPX pin on. This is done here so that busprobe will fail if we have a switch connected.
        regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL));

        return ( 0);
}

void Max3421e::vbusPower(VBUS_t state) {
                regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL | state));
        }

uint8_t Max3421e::getVbusState(void) {
                return vbusState;
        };

/* probe bus to determine device presence and speed and switch host to this speed */ 
void Max3421e::busprobe() {
        uint8_t bus_sample;
        bus_sample = regRd(rHRSL); //Get J,K status
        bus_sample &= (bmJSTATUS | bmKSTATUS); //zero the rest of the byte
        switch(bus_sample) { //start full-speed or low-speed host
                case( bmJSTATUS):
                        if((regRd(rMODE) & bmLOWSPEED) == 0) {
                                regWr(rMODE, MODE_FS_HOST); //start full-speed host
                                vbusState = FSHOST;
                        } else {
                                regWr(rMODE, MODE_LS_HOST); //start low-speed host
                                vbusState = LSHOST;
                        }
                        break;
                case( bmKSTATUS):
                        if((regRd(rMODE) & bmLOWSPEED) == 0) {
                                regWr(rMODE, MODE_LS_HOST); //start low-speed host
                                vbusState = LSHOST;
                        } else {
                                regWr(rMODE, MODE_FS_HOST); //start full-speed host
                                vbusState = FSHOST;
                        }
                        break;
                case( bmSE1): //illegal state
                        vbusState = SE1;
                        break;
                case( bmSE0): //disconnected state
                        regWr(rMODE, bmDPPULLDN | bmDMPULLDN | bmHOST | bmSEPIRQ);
                        vbusState = SE0;
                        break;
        }//end switch( bus_sample )
}

uint8_t Max3421e::IntHandler() {
        uint8_t HIRQ;
        uint8_t HIRQ_sendback = 0x00;
        HIRQ = regRd(rHIRQ); //determine interrupt source
        //if( HIRQ & bmFRAMEIRQ ) {               //->1ms SOF interrupt handler
        //    HIRQ_sendback |= bmFRAMEIRQ;
        //}//end FRAMEIRQ handling
        if(HIRQ & bmCONDETIRQ) {
                busprobe();
                HIRQ_sendback |= bmCONDETIRQ;
        }
        /* End HIRQ interrupts handling, clear serviced IRQs    */
        regWr(rHIRQ, HIRQ_sendback);
        return ( HIRQ_sendback);
}

/* MAX3421 state change task and interrupt handler */ 
uint8_t Max3421e::Task(void) {
        uint8_t rcode = 0;
        uint8_t pinvalue;
        //USB_HOST_SERIAL.print("Vbus state: ");
        //USB_HOST_SERIAL.println( vbusState, HEX );
        pinvalue = readIntPin(); //Read();
        //pinvalue = digitalRead( MAX_INT );
        if(pinvalue == 0) {
                rcode = IntHandler();
        }
        //    pinvalue = digitalRead( MAX_GPX );
        //    if( pinvalue == LOW ) {
        //        GpxHandler();
        //    }
        //    usbSM();                                //USB state machine
        return ( rcode);
}

void Max3421e::ssHigh(){
	
	ESP32CPP::GPIO::high((gpio_num_t) ss_pin);
}

void Max3421e::ssLow(){
	
	ESP32CPP::GPIO::low((gpio_num_t) ss_pin);
}

uint8_t Max3421e::readIntPin(){
	
	return ESP32CPP::GPIO::read((gpio_num_t) interrupt_pin);
}
