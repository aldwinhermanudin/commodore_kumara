
#ifndef _usb_setup_packet_
#define _usb_setup_packet_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/* USB Setup Packet Structure   */
typedef struct {

        union { // offset   description
                uint8_t bmRequestType; //   0      Bit-map of request type

                struct {
                        uint8_t recipient : 5; //          Recipient of the request
                        uint8_t type : 2; //          Type of request
                        uint8_t direction : 1; //          Direction of data X-fer
                } __attribute__((packed));
        } ReqType_u;
        uint8_t bRequest; //   1      Request

        union {
                uint16_t wValue; //   2      Depends on bRequest

                struct {
                        uint8_t wValueLo;
                        uint8_t wValueHi;
                } __attribute__((packed));
        } wVal_u;
        uint16_t wIndex; //   4      Depends on bRequest
        uint16_t wLength; //   6      Depends on bRequest
} __attribute__((packed)) SETUP_PKT, *PSETUP_PKT;

#endif
