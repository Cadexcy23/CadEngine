#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "../sol.hpp"
#include "../Scene/object.h"
#include "../Core/logger.h"

class Lua {
public:
    static void init();
    static void cleanup();

    static bool loadScript(const std::string& scriptPath);
    static void attachScript(const std::string& scriptPath,
        std::shared_ptr<Object::engineObject> obj);
    template<typename Func>
    static bool registerEngineFunction(const std::string& name, Func&& func) {
        try {
            EngineFunctionRegistration reg = [name, func = std::forward<Func>(func)]
            (sol::state& lua) mutable {
                lua["engine"][name] = func;
                };

            engineFunctionRegistrations[name] = reg;

            if (initialized) {
                reg(lua);
                Logger::log(Logger::LogCategory::InputOutput,
               Logger::LogLevel::Debug,
               "Registered engine function: %s", name.c_str());
            }

            return true;
        }
        catch (const sol::error& e) {
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Error,
                "Failed to register engine function %s: %s", name.c_str(), e.what());
            return false;
        }
    }
    template<typename T, typename... Args>
    static bool registerType(const std::string& typeName, Args&&... args) {
        static_assert(std::is_base_of_v<Object::engineObject, T>,
            "Lua-registered types must inherit from engineObject");
        try {
            // Store arguments by value
            auto reg = [typeName, args...](sol::state& lua) mutable {
                // Create a tuple and expand it
                auto tuple = std::make_tuple(args...);
                std::apply([&](auto&... tupleArgs) {
                    lua.new_usertype<T>(typeName,
                        sol::base_classes, sol::bases<Object::engineObject>(),
                        tupleArgs...
                        );
                    }, tuple);
                };

            typeRegistrations[typeName] = reg;

            if (initialized) {
                reg(lua);
                Logger::log(Logger::LogCategory::InputOutput,
                    Logger::LogLevel::Debug,
                    "Registered Lua type: %s", typeName.c_str());
            }

            return true;
        }
        catch (const sol::error& e) {
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Error,
                "Failed to register type %s: %s", typeName.c_str(), e.what());
            return false;
        }
    }
    template<typename T, typename... Args>
    static bool registerTypeSimple(const std::string& typeName, Args&&... args) {
        try {
            // Store arguments by value
            auto reg = [typeName, args...](sol::state& lua) mutable {
                // Create a tuple and expand it
                auto tuple = std::make_tuple(args...);
                std::apply([&](auto&&... tupleArgs) {
                    lua.new_usertype<T>(typeName,
                        std::forward<decltype(tupleArgs)>(tupleArgs)...);
                    }, tuple);
                };

            typeRegistrations[typeName] = reg;

            if (initialized) {
                reg(lua);
                Logger::log(Logger::LogCategory::InputOutput,
                    Logger::LogLevel::Debug,
                    "Registered Lua type: %s", typeName.c_str());
            }

            return true;
        }
        catch (const sol::error& e) {
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Error,
                "Failed to register type %s: %s", typeName.c_str(), e.what());
            return false;
        }
    }

    using TypeRegistration = std::function<void(sol::state&)>;
    using EngineFunctionRegistration = std::function<void(sol::state&)>;
private:

    static sol::state lua;
    static std::unordered_map<std::string, TypeRegistration> typeRegistrations;
    static std::unordered_map<std::string, EngineFunctionRegistration> engineFunctionRegistrations;
    static std::unordered_map<std::string, sol::environment> scriptEnvironments;
    static bool initialized;
};