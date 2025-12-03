#include "logger.h"
#include <stdio.h>
#include <string>
#include <chrono>


//make logger run on a thread?


std::unordered_map<Logger::LogCategory, Logger::LogSettings> Logger::logSettings = {
    { Logger::LogCategory::General,     { LogLevel::Debug, LogLevel::Debug, true,  true, "General"}},
    { Logger::LogCategory::Engine,      { LogLevel::Debug, LogLevel::Debug, true,  true, "Engine" } },
    { Logger::LogCategory::Graphics,    { LogLevel::Debug, LogLevel::Debug, true,  true, "Graphics" } },
    { Logger::LogCategory::InputOutput, { LogLevel::Debug, LogLevel::Debug, true,  true, "InputOutput" } },
    { Logger::LogCategory::Network,     { LogLevel::Debug, LogLevel::Debug, true,  true, "Network" } },
    { Logger::LogCategory::Scene,       { LogLevel::Debug, LogLevel::Debug, true,  true, "Scene" } },
    { Logger::LogCategory::Sound,       { LogLevel::Debug, LogLevel::Debug, true,  true, "Sound" } },
};

void clearLogs()
{
    for (auto& entry : Logger::logSettings)
    {
        auto cat = entry.first;
        std::string settings = entry.second.name;

        std::string path = "logs/" + settings + ".log";
        FILE* f;
        fopen_s(&f, path.c_str(), "w");
        if (f) fclose(f);
    }
}

void writeToFile(Logger::LogCategory cat, const char* text)
{
    std::string name = Logger::logSettings[cat].name;
    std::string path = "logs/" + name + ".log";

    FILE* f;
    fopen_s(&f, path.c_str(), "a");
    if (!f) 
    { 
		printf("Failed to open log file: %s\n", path.c_str());
        return;
    }

    fprintf(f, "%s\n", text);
    fclose(f);
}

void Logger::log(LogCategory cat, LogLevel level, const char* fmt, ...)
{
    const auto& cfg = logSettings[cat];

    bool toConsole = cfg.consoleEnabled && (level >= cfg.consoleLevel);
    bool toFile = cfg.fileEnabled && (level >= cfg.fileLevel);

    if (!toConsole && !toFile)
        return;

    char msgBuffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, args);
    va_end(args);

    // timestamp
    time_t t = time(nullptr);
    struct tm lt;
    localtime_s(&lt, &t);

    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "[%Y-%m-%d %H:%M:%S] ", &lt);

    char finalBuffer[1200];
    snprintf(finalBuffer, sizeof(finalBuffer), "%s%s", timeBuffer, msgBuffer);

    if (toConsole)
        printf("%s\n", finalBuffer);

    if (toFile)
        writeToFile(cat, finalBuffer);
}

bool Logger::init()
{
    clearLogs();

    return true;
}