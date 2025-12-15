#include "lua.h"
#include <filesystem>
#include "../Core/logger.h"


sol::state Lua::lua;
std::unordered_map<std::string, Lua::TypeRegistration> Lua::typeRegistrations;
std::unordered_map<std::string, Lua::EngineFunctionRegistration> Lua::engineFunctionRegistrations;
std::unordered_map<std::string, sol::environment> Lua::scriptEnvironments;
bool Lua::initialized = false;


void Lua::init() {
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table,
        sol::lib::string, sol::lib::os);
    Lua::initialized = true;

    // Register engineObject to Lua
    lua.new_usertype<Object::engineObjectBase>("engineObject",
        // Properties
        "scale", &Object::engineObjectBase::scale,
        "rotation", &Object::engineObjectBase::rot,
        "depth", &Object::engineObjectBase::depth,
        "drawFlag", &Object::engineObjectBase::drawFlag,
        "updateFlag", &Object::engineObjectBase::updateFlag,

        // Methods
        "getPosition", [](Object::engineObjectBase& obj) {
            return std::make_tuple(obj.hull.x, obj.hull.y);
        },
        "setPosition", [](Object::engineObjectBase& obj, float x, float y) {
            obj.hull.x = x;
            obj.hull.y = y;
        },
        "getSize", [](Object::engineObjectBase& obj) {
            return std::make_tuple(obj.hull.w, obj.hull.h);
        },
        "setTexture", [](Object::engineObjectBase& obj, int index) {
            if (index >= 0 && index < obj.textures.size()) {
                obj.texIndex = index;
            }
        }
    );

    // Create a table for engine functions
    lua["engine"] = lua.create_table_with(
        "log", [](const std::string& message) {
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Info,
                "%s", message.c_str());
        },
        "getDeltaTime", []() {
            // Return your game's delta time
            return 0.016f; // Placeholder
        }
    );
}

bool Lua::loadScript(const std::string& scriptPath) {
    try {
        if (!std::filesystem::exists(scriptPath)) {
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Error,
                "Script file not found: %s", scriptPath.c_str());
            return false;
        }

        // Create a new environment for this script
        sol::environment env(lua, sol::create, lua.globals());
        scriptEnvironments[scriptPath] = env;

        // v3.3.0 way to load and execute
        sol::protected_function_result scriptResult = lua.safe_script_file(scriptPath, env);

        if (!scriptResult.valid()) {
            sol::error err = scriptResult;
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Error,
                "Failed to load script %s: %s",
                scriptPath.c_str(), err.what());
            return false;
        }

        return true;
    }
    catch (const sol::error& e) {
        Logger::log(Logger::LogCategory::InputOutput,
            Logger::LogLevel::Error,
            "Script error in %s: %s", scriptPath.c_str(), e.what());
        return false;
    }
    catch (const std::exception& e) {
        Logger::log(Logger::LogCategory::InputOutput,
            Logger::LogLevel::Error,
            "Std exception in %s: %s", scriptPath.c_str(), e.what());
        return false;
    }
}

//void Lua::attachScript(const std::string& scriptPath,
//    std::shared_ptr<Object::engineObject> obj) {
//
//    auto it = scriptEnvironments.find(scriptPath);
//    if (it == scriptEnvironments.end()) {
//        if (!loadScript(scriptPath)) {
//            return;
//        }
//        it = scriptEnvironments.find(scriptPath);
//    }
//
//    sol::environment& env = it->second;
//
//    // Check if the script has an update function
//    sol::optional<sol::protected_function> updateFunc = env["update"];
//    if (updateFunc) {
//        obj->updateFuncs.push_back([env, updateFunc](std::shared_ptr<Object::engineObject> o) {
//            try {
//                sol::protected_function_result result = (*updateFunc)(o);
//                if (!result.valid()) {
//                    sol::error err = result;
//                    Logger::log(Logger::LogCategory::InputOutput,
//                        Logger::LogLevel::Error,
//                        "Update function error: %s", err.what());
//                }
//            }
//            catch (const sol::error& e) {
//                Logger::log(Logger::LogCategory::InputOutput,
//                    Logger::LogLevel::Error,
//                    "Update function exception: %s", e.what());
//            }
//            });
//    }
//
//    // Call onCreate if it exists
//    sol::optional<sol::protected_function> onCreateFunc = env["onCreate"];
//    if (onCreateFunc) {
//        try {
//            sol::protected_function_result result = (*onCreateFunc)(obj);
//            if (!result.valid()) {
//                sol::error err = result;
//                Logger::log(Logger::LogCategory::InputOutput,
//                    Logger::LogLevel::Error,
//                    "onCreate function error: %s", err.what());
//            }
//        }
//        catch (const sol::error& e) {
//            Logger::log(Logger::LogCategory::InputOutput,
//                Logger::LogLevel::Error,
//                "onCreate function exception: %s", e.what());
//        }
//    }
//}