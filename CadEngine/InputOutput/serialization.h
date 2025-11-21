#pragma once
#include <string>


static class Serialization {
public:
	static std::string getSetting(const std::string& setting);
	static void setSetting(const std::string& setting, const std::string& newValue);
};