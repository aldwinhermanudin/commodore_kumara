#include "Greeting.h"

Greeting::Greeting(std::string log_tag){
	this->log_tag = log_tag;
 }

void Greeting::helloEnglish() {
	ESP_LOGD(log_tag.c_str(), "Hello %s", name.c_str());
}

void Greeting::helloFrench() {
	ESP_LOGD(log_tag.c_str(), "Bonjour %s", name.c_str());
}

void Greeting::setName(std::string name) {
	this->name = name;
}
