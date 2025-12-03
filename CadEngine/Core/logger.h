#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>

static class Logger {
public:
    enum class LogCategory {
        General,
        Engine,
        Graphics,
        InputOutput,
        Network,
        Scene,
        Sound
    };
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warn,
        Error,
        Fatal,
    };
    struct LogSettings {
        LogLevel consoleLevel;
        LogLevel fileLevel;
        bool consoleEnabled;
        bool fileEnabled;
        const char* name;
    };
    static std::unordered_map<LogCategory, LogSettings> logSettings;


	static void log(LogCategory cat, LogLevel level, const char* fmt, ...);
    static bool init();
};