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
    template<typename T>
    static void attachScript(const std::string& scriptPath, std::shared_ptr<T> obj) {
        if (!initialized) return;

        // Load or get cached environment
        sol::environment& env = getScriptEnvironment(scriptPath);

        // Handle update function
        sol::optional<sol::protected_function> updateFunc = env["update"];
        if (updateFunc) {
            // Store as protected_function
            sol::protected_function func = *updateFunc;

            obj->addUpdateFunc([func](std::shared_ptr<T> derivedObj) {
                try {
                    sol::protected_function_result result = func(derivedObj);
                    if (!result.valid()) {
                        sol::error err = result;
                        Logger::log(Logger::LogCategory::InputOutput,
                            Logger::LogLevel::Error,
                            "Update function error: %s", err.what());
                    }
                }
                catch (const sol::error& e) {
                    Logger::log(Logger::LogCategory::InputOutput,
                        Logger::LogLevel::Error,
                        "Update function exception: %s", e.what());
                }
                });
        }

        // Handle onCreate with protected_function
        sol::optional<sol::protected_function> onCreateFunc = env["onCreate"];
        if (onCreateFunc) {
            try {
                sol::protected_function_result result = (*onCreateFunc)(obj);
                if (!result.valid()) {
                    sol::error err = result;
                    Logger::log(Logger::LogCategory::InputOutput,
                        Logger::LogLevel::Error,
                        "onCreate function error: %s", err.what());
                }
            }
            catch (const sol::error& e) {
                Logger::log(Logger::LogCategory::InputOutput,
                    Logger::LogLevel::Error,
                    "onCreate function exception: %s", e.what());
            }
        }
    }
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
        static_assert(std::is_base_of_v<Object::engineObjectBase, T>,
            "Lua-registered types must inherit from engineObjectBase");
        try {
            // Store arguments by value
            auto reg = [typeName, args...](sol::state& lua) mutable {
                // Create a tuple and expand it
                auto tuple = std::make_tuple(args...);
                std::apply([&](auto&... tupleArgs) {
                    lua.new_usertype<T>(typeName,
                        sol::base_classes, sol::bases<Object::engineObjectBase>(),
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

    using TypeRegistration = std::function<void(sol::state&)>; //need?
    using EngineFunctionRegistration = std::function<void(sol::state&)>;
private:

    static sol::state lua;
    static std::unordered_map<std::string, TypeRegistration> typeRegistrations;
    static std::unordered_map<std::string, EngineFunctionRegistration> engineFunctionRegistrations;
    static std::unordered_map<std::string, sol::environment> scriptEnvironments;
    static bool initialized;

    static sol::environment& getScriptEnvironment(const std::string& scriptPath) {
        auto it = scriptEnvironments.find(scriptPath);
        if (it == scriptEnvironments.end()) {
            sol::environment env(lua, sol::create, lua.globals());
            auto result = lua.safe_script_file(scriptPath, env);
            if (!result.valid()) {
                throw std::runtime_error("Failed to load script: " + scriptPath);
            }
            scriptEnvironments[scriptPath] = env;
            it = scriptEnvironments.find(scriptPath);
        }
        return it->second;
    }
};