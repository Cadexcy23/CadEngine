#include "lua.h"
#include <filesystem>
#include "../InputOutput/input.h"


sol::state Lua::lua;
std::unordered_map<std::string, std::function<void(sol::state&)>> Lua::typeRegistrations;
std::unordered_map<std::string, std::function<void(sol::state&)>> Lua::engineFunctionRegistrations;
std::unordered_map<std::string, sol::environment> Lua::scriptEnvironments;
bool Lua::initialized = false;


void Lua::init() {
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table,
        sol::lib::string, sol::lib::os);
    Lua::initialized = true;

    // register extra types to lua
    Lua::registerTypeSimple<SDL_FPoint>("SDL_FPoint",
        "x", &SDL_FPoint::x,
        "y", &SDL_FPoint::y,
        "__tostring", [](const SDL_FPoint& v) {
            return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        }
    );
    Lua::registerTypeSimple<SDL_FRect>("SDL_FRect",
        "x", &SDL_FRect::x,
        "y", &SDL_FRect::y,
        "w", &SDL_FRect::w,
        "h", &SDL_FRect::h
    );

    // Create a table for engine functions
    lua["engine"] = lua.create_table_with(
        "log", [](const std::string& message) {
            Logger::log(Logger::LogCategory::InputOutput,
                Logger::LogLevel::Info,
                "%s", message.c_str());
        },
        "getDeltaTime", []() {
            return Time::deltaTime;
        },
        "getDeltaSeconds", []() {
            return Time::deltaSeconds;
        },
        "keyState", [](const char* key) {
            int i = Input::keyStates[SDL_GetScancodeFromName(key)];
            if (i > 0)
                return i;
            return NULL;
        },
        "screenBounds", Renderer::screenBounds
    );

    // Register engineObject to Lua
    lua.new_usertype<Object::engineObjectBase>("engineObject",
        // Properties
        "texIndex", &Object::engineObjectBase::texIndex,
        "hull", &Object::engineObjectBase::hull,
        "centered", &Object::engineObjectBase::centered,
        "fixed", &Object::engineObjectBase::fixed,
        "flip", &Object::engineObjectBase::flip,
        "scale", &Object::engineObjectBase::scale,
        "rotation", &Object::engineObjectBase::rot,
        "depth", &Object::engineObjectBase::depth,
        "drawDefault", &Object::engineObjectBase::drawDefault,
        "drawFlag", &Object::engineObjectBase::drawFlag,
        "updateFlag", &Object::engineObjectBase::updateFlag,
        "remove", &Object::engineObjectBase::remove,
        "timeCreated", sol::readonly(&Object::engineObjectBase::timeCreated),

        // Methods
        "inScreen", & Object::engineObjectBase::inScreen,
        "getBounds", &Object::engineObjectBase::getBounds,
        "mouseInBounds", & Object::engineObjectBase::mouseInBounds,
        "addTexture", [](Object::engineObjectBase& obj, int index) {
            if (index >= 0 && index < obj.textures.size()) {
                obj.texIndex = index;
            }
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

sol::environment& Lua::getScriptEnvironment(const std::string& scriptPath) {
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