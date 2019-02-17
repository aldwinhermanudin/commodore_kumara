
#ifndef _usb_reader_parser_
#define _usb_reader_parser_

class USBReadParser {
public:
        virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint16_t &offset) = 0;
};

#endif
