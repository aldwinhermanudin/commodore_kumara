#ifndef _test_h_
#define _test_h_

#include "Max3421e.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class Test : public Max3421e{
	
	public:
		Test(int input);
		uint8_t test_var_one;
	
};

#endif
