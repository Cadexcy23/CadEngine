#include "serialization.h"
#include "../Core/logger.h"
#include <regex>
#include <fstream>
#include <sstream>

//convert to using json

void generateDefaultSettingsFile()
{
	Logger::log(Logger::LogCategory::Engine, Logger::LogLevel::Info, "Generating default settings file");
	//check if there is one, back it up if there is
	if (std::ifstream("resource/settings.cfg"))
	{
		Logger::log(Logger::LogCategory::Engine, Logger::LogLevel::Warn, "settings.cfg already exists, backing up");
		std::ifstream file("resource/settings.cfg");
		std::string line;
		std::string fileData;
		while (std::getline(file, line))
		{
			fileData += line + "\n";
		}
		file.close();
		std::ofstream backup("resource/settings.cfg.bak");
		backup << fileData;
		backup.close();
	}

	std::ofstream file("resource/settings.cfg");
	file << "[Resolution] 960 540\n";
	file << "[Vsync] 1\n";
	file.close();
}

std::string Serialization::getSetting(const std::string& setting)
{
	std::ifstream file("resource/settings.cfg");
	if (!file) {
		Logger::log(Logger::LogCategory::InputOutput, Logger::LogLevel::Error, "Failed to open settings file for reading.");
		file.close();
		generateDefaultSettingsFile();
		file.open("resource/settings.cfg");
	}

	std::string line;
	std::string settingValue;

	while (std::getline(file, line))
	{
		if (line.find("[" + setting + "]") != std::string::npos)
		{
			// Match everything after the "]" with optional leading spaces
			std::regex rgx("\\[" + setting + "\\]\\s*(.*)");
			std::smatch match;
			if (std::regex_search(line, match, rgx) && match.size() > 1)
			{
				settingValue = match[1].str(); // Capture the entire value
				break; // No need to continue searching once found
			}
		}
	}

	file.close();
	return settingValue;
}

void Serialization::setSetting(const std::string& setting, const std::string& newValue)
{
	std::ifstream fileIn("resource/settings.cfg");
	if (!fileIn) {
		Logger::log(Logger::LogCategory::InputOutput, Logger::LogLevel::Error, "Failed to open settings file for reading.");
		return;
	}

	std::ostringstream buffer;
	std::string line;
	bool settingFound = false;

	while (std::getline(fileIn, line))
	{
		if (line.find("[" + setting + "]") != std::string::npos)
		{
			std::regex rgx("\\[" + setting + "\\].*");
			if (std::regex_search(line, rgx)) {
				line = "[" + setting + "] " + newValue;
				settingFound = true;
			}
		}

		buffer << line << "\n";
	}

	fileIn.close();

	if (!settingFound) {
		buffer << "[" << setting << "] " << newValue << "\n";
	}

	std::ofstream fileOut("resource/settings.cfg");
	if (!fileOut) {
		Logger::log(Logger::LogCategory::InputOutput, Logger::LogLevel::Error, "Failed to open settings file for writing.");
		return;
	}

	fileOut << buffer.str();
	fileOut.close();
}

