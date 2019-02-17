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


#if !defined(_usb_h_) || defined(_USBHOST_H_)
#error "Never include usbhost.h directly; include Usb.h instead"
#else
#define _USBHOST_H_

#define MAX3421E_SCK 18
#define MAX3421E_MISO 19
#define MAX3421E_MOSI 23
#define MAX3421E_SS 5
#define MAX3421E_INT 17

SPI USB_SPI;

/* SPI initialization */
template< typename SPI_CLK, typename SPI_MOSI, typename SPI_MISO, typename SPI_SS > class SPi {
public:

        static void init() {
				
				USB_SPI.init(MAX3421E_MOSI,MAX3421E_MISO,MAX3421E_SCK,MAX3421E_SS);
                //USB_SPI.begin(); // The SPI library with transaction will take care of setting up the pins - settings is set in beginTransaction()
                SPI_SS::SetDirWrite();
                SPI_SS::Set();
        }
};

/* SPI pin definitions. see avrpins.h   */
typedef SPi< P18, P23, P19, P5 > spi;

typedef enum {
        vbus_on = 0,
        vbus_off = GPX_VBDET
} VBUS_t;

template< typename SPI_SS, typename INTR > class MAX3421e /* : public spi */ {
        static uint8_t vbusState;

public:
        MAX3421e();
        void regWr(uint8_t reg, uint8_t data);
        uint8_t* bytesWr(uint8_t reg, uint8_t nbytes, uint8_t* data_p);
        void gpioWr(uint8_t data);
        uint8_t regRd(uint8_t reg);
        uint8_t* bytesRd(uint8_t reg, uint8_t nbytes, uint8_t* data_p);
        uint8_t gpioRd();
        uint16_t reset();
        int8_t Init();
        int8_t Init(int mseconds);

        void vbusPower(VBUS_t state) {
                regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL | state));
        }

        uint8_t getVbusState(void) {
                return vbusState;
        };
        void busprobe();
        uint8_t GpxHandler();
        uint8_t IntHandler();
        uint8_t Task();
};

template< typename SPI_SS, typename INTR >
        uint8_t MAX3421e< SPI_SS, INTR >::vbusState = 0;

/* constructor */
template< typename SPI_SS, typename INTR >
MAX3421e< SPI_SS, INTR >::MAX3421e() {
        // Leaving ADK hardware setup in here, for now. This really belongs with the other parts.
};

/* write single byte into MAX3421 register */
template< typename SPI_SS, typename INTR >
void MAX3421e< SPI_SS, INTR >::regWr(uint8_t reg, uint8_t data) {
	
        //USB_SPI.beginTransaction(SPISettings(26000000, MSBFIRST, SPI_MODE0)); // The MAX3421E can handle up to 26MHz, use MSB First and SPI mode 0
        SPI_SS::Clear();
        
		USB_SPI.transferByte(reg | 0x02);
		USB_SPI.transferByte(data);
        
        SPI_SS::Set();
        //USB_SPI.endTransaction();
        return;
};
/* multiple-byte write                            */

/* returns a pointer to memory position after last written */
template< typename SPI_SS, typename INTR >
uint8_t* MAX3421e< SPI_SS, INTR >::bytesWr(uint8_t reg, uint8_t nbytes, uint8_t* data_p) {
	
        //USB_SPI.beginTransaction(SPISettings(26000000, MSBFIRST, SPI_MODE0)); // The MAX3421E can handle up to 26MHz, use MSB First and SPI mode 0
        SPI_SS::Clear();
        
		USB_SPI.transferByte(reg | 0x02);
        while(nbytes) {
                USB_SPI.transferByte(*data_p);
                nbytes--;
                data_p++; // advance data pointer
        } 

        SPI_SS::Set();
        
        //USB_SPI.endTransaction();
        
        return ( data_p);
}
/* GPIO write                                           */
/*GPIO byte is split between 2 registers, so two writes are needed to write one byte */

/* GPOUT bits are in the low nibble. 0-3 in IOPINS1, 4-7 in IOPINS2 */
template< typename SPI_SS, typename INTR >
void MAX3421e< SPI_SS, INTR >::gpioWr(uint8_t data) {
        regWr(rIOPINS1, data);
        data >>= 4;
        regWr(rIOPINS2, data);
        return;
}

/* single host register read    */
template< typename SPI_SS, typename INTR >
uint8_t MAX3421e< SPI_SS, INTR >::regRd(uint8_t reg) {
	
        //USB_SPI.beginTransaction(SPISettings(26000000, MSBFIRST, SPI_MODE0)); // The MAX3421E can handle up to 26MHz, use MSB First and SPI mode 0
        SPI_SS::Clear();


		USB_SPI.transferByte(reg); 
        uint8_t rv = USB_SPI.transferByte(0); // Send empty byte
        SPI_SS::Set();
        
        //USB_SPI.endTransaction();
        return (rv);
}
/* multiple-byte register read  */

/* returns a pointer to a memory position after last read   */
template< typename SPI_SS, typename INTR >
uint8_t* MAX3421e< SPI_SS, INTR >::bytesRd(uint8_t reg, uint8_t nbytes, uint8_t* data_p) {
	
        //USB_SPI.beginTransaction(SPISettings(26000000, MSBFIRST, SPI_MODE0)); // The MAX3421E can handle up to 26MHz, use MSB First and SPI mode 0
        SPI_SS::Clear();
        
        USB_SPI.transferByte(reg);
        while(nbytes) {
            *data_p++ = USB_SPI.transferByte(0);
            nbytes--;
        }
        
        SPI_SS::Set();
        
        //USB_SPI.endTransaction();
        
        return ( data_p);
}
/* GPIO read. See gpioWr for explanation */

/* GPIN pins are in high nibbles of IOPINS1, IOPINS2    */
template< typename SPI_SS, typename INTR >
uint8_t MAX3421e< SPI_SS, INTR >::gpioRd() {
        uint8_t gpin = 0;
        gpin = regRd(rIOPINS2); //pins 4-7
        gpin &= 0xf0; //clean lower nibble
        gpin |= (regRd(rIOPINS1) >> 4); //shift low bits and OR with upper from previous operation.
        return ( gpin);
}

/* reset MAX3421E. Returns number of cycles it took for PLL to stabilize after reset
  or zero if PLL haven't stabilized in 65535 cycles */
template< typename SPI_SS, typename INTR >
uint16_t MAX3421e< SPI_SS, INTR >::reset() {
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
template< typename SPI_SS, typename INTR >
int8_t MAX3421e< SPI_SS, INTR >::Init() {
        // Moved here.
        // you really should not init hardware in the constructor when it involves locks.
        // Also avoids the vbus flicker issue confusing some devices.
        /* pin and peripheral setup */
        SPI_SS::SetDirWrite();
        SPI_SS::Set();
        spi::init();
        INTR::SetDirRead();
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
template< typename SPI_SS, typename INTR >
int8_t MAX3421e< SPI_SS, INTR >::Init(int mseconds) {
        // Moved here.
        // you really should not init hardware in the constructor when it involves locks.
        // Also avoids the vbus flicker issue confusing some devices.
        /* pin and peripheral setup */
        SPI_SS::SetDirWrite();
        SPI_SS::Set();
        spi::init();
        INTR::SetDirRead();
        /* MAX3421E - full-duplex SPI, level interrupt, vbus off */
        regWr(rPINCTL, (bmFDUPSPI | bmINTLEVEL | GPX_VBDET));

        if(reset() == 0) { //OSCOKIRQ hasn't asserted in time
                return ( -1);
        }

        // Delay a minimum of 1 second to ensure any capacitors are drained.
        // 1 second is required to make sure we do not smoke a Microdrive!
        if(mseconds < 1000) mseconds = 1000;
        vTaskDelay(mseconds /100);
        //delay(mseconds);

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

/* probe bus to determine device presence and speed and switch host to this speed */
template< typename SPI_SS, typename INTR >
void MAX3421e< SPI_SS, INTR >::busprobe() {
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

/* MAX3421 state change task and interrupt handler */
template< typename SPI_SS, typename INTR >
uint8_t MAX3421e< SPI_SS, INTR >::Task(void) {
        uint8_t rcode = 0;
        uint8_t pinvalue;
        //USB_HOST_SERIAL.print("Vbus state: ");
        //USB_HOST_SERIAL.println( vbusState, HEX );
        pinvalue = INTR::IsSet(); //Read();
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

template< typename SPI_SS, typename INTR >
uint8_t MAX3421e< SPI_SS, INTR >::IntHandler() {
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
//template< typename SPI_SS, typename INTR >
//uint8_t MAX3421e< SPI_SS, INTR >::GpxHandler()
//{
//    uint8_t GPINIRQ = regRd( rGPINIRQ );          //read GPIN IRQ register
////    if( GPINIRQ & bmGPINIRQ7 ) {            //vbus overload
////        vbusPwr( OFF );                     //attempt powercycle
////        delay( 1000 );
////        vbusPwr( ON );
////        regWr( rGPINIRQ, bmGPINIRQ7 );
////    }
//    return( GPINIRQ );
//}

#endif // _USBHOST_H_
