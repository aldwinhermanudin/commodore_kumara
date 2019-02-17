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

/* derived from Konstantin Chizhov's AVR port templates */

#if !defined(_usb_h_) || defined(_avrpins_h_)
#error "Never include avrpins.h directly; include Usb.h instead"
#else
#define _avrpins_h_

#define MAKE_PIN(className, pin) \
class className { \
public: \
  static void Set() { \
    ESP32CPP::GPIO::high((gpio_num_t) pin); \
  } \
  static void Clear() { \
    ESP32CPP::GPIO::low((gpio_num_t) pin); \
  } \
  static void SetDirRead() { \
	ESP32CPP::GPIO::setInput((gpio_num_t) pin); \
  } \
  static void SetDirWrite() { \
    ESP32CPP::GPIO::setOutput((gpio_num_t) pin); \
  } \
  static uint8_t IsSet() { \
    return ESP32CPP::GPIO::read((gpio_num_t) pin); \
  } \
};

// Workaround strict-aliasing warnings
#ifdef pgm_read_word
#undef pgm_read_word
#endif
#ifdef pgm_read_dword
#undef pgm_read_dword
#endif
#ifdef  pgm_read_float
#undef pgm_read_float
#endif
#ifdef  pgm_read_ptr
#undef pgm_read_ptr
#endif

#define pgm_read_word(addr) ({ \
  typeof(addr) _addr = (addr); \
  *(const unsigned short *)(_addr); \
})
#define pgm_read_dword(addr) ({ \
  typeof(addr) _addr = (addr); \
  *(const unsigned long *)(_addr); \
})
#define pgm_read_float(addr) ({ \
  typeof(addr) _addr = (addr); \
  *(const float *)(_addr); \
})
#define pgm_read_ptr(addr) ({ \
  typeof(addr) _addr = (addr); \
  *(void * const *)(_addr); \
})

// Pinout for ESP32 dev module

MAKE_PIN(P0, 0);
MAKE_PIN(P1, 1); // TX0
MAKE_PIN(P10, 10); // TX1
MAKE_PIN(P3, 3); // RX0
MAKE_PIN(P21, 21); // SDA
MAKE_PIN(P22, 22); // SCL
MAKE_PIN(P19, 19); // MISO
MAKE_PIN(P23, 23); // MOSI
MAKE_PIN(P18, 18); // SCK
MAKE_PIN(P5, 5); // SS
MAKE_PIN(P17, 17); // INT

#undef MAKE_PIN

// pgm_read_ptr is not defined in the ESP32, so we have to undef the diffinition from version_helper.h
#ifdef pgm_read_pointer
#undef pgm_read_pointer
#endif
#define pgm_read_pointer(p) pgm_read_ptr(p)

#endif //_avrpins_h_
