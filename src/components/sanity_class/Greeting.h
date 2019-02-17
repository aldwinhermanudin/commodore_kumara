


#ifndef SANITY_CLASS
#define SANITY_CLASS

#include <string>
#include <esp_log.h>
#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

class Greeting {
public:

	Greeting(std::string log_tag);
	void helloEnglish();

	void helloFrench();

	void setName(std::string name);
private:
	std::string name;
	std::string log_tag;
};

#endif
