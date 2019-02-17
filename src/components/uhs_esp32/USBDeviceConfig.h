#ifndef _usb_device_config_h_
#define _usb_device_config_h_

class USBDeviceConfig {
public:

        virtual uint8_t Init(uint8_t parent __attribute__((unused)), uint8_t port __attribute__((unused)), bool lowspeed __attribute__((unused))) {
                return 0;
        }

        virtual uint8_t ConfigureDevice(uint8_t parent __attribute__((unused)), uint8_t port __attribute__((unused)), bool lowspeed __attribute__((unused))) {
                return 0;
        }

        virtual uint8_t Release() {
                return 0;
        }

        virtual uint8_t Poll() {
                return 0;
        }

        virtual uint8_t GetAddress() {
                return 0;
        }

        virtual void ResetHubPort(uint8_t port __attribute__((unused))) {
                return;
        } // Note used for hubs only!

        virtual bool VIDPIDOK(uint16_t vid __attribute__((unused)), uint16_t pid __attribute__((unused))) {
                return false;
        }

        virtual bool DEVCLASSOK(uint8_t klass __attribute__((unused))) {
                return false;
        }

        virtual bool DEVSUBCLASSOK(uint8_t subklass __attribute__((unused))) {
                return true;
        }
};

#endif
