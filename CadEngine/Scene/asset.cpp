#include "asset.h"
#include <random>
#include <chrono>
#include <filesystem>
#include "../Core/engine.h"
#include "../InputOutput/lua.h"


std::unordered_map<std::string, Asset::assetLoader> Asset::loaders;
std::unordered_map<std::string, Asset::assetInfo> Asset::registry;
std::unordered_map<std::string, std::weak_ptr<Object::engineObjectBase>> Asset::cache;


inline std::string Asset::GenerateUUID()
{
    std::string hex_string;

    for (int i = 0; i < 32; i++) {
        // Generate random number 0-15
        int num = static_cast<int>(Engine::randInRange({ 0, 15.9999f }));

        // Convert to hex character
        hex_string += (num < 10) ? ('0' + num) : ('a' + (num - 10));
    }

    return hex_string;
}

void Asset::scanAssetDirectory(const std::string& folder) {
    size_t loaded = 0;

    for (auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
        if (entry.path().extension() == ".cea") {
            auto meta = loadMetadata(entry.path().string());

            if (!meta.has_value()) {
                continue;
            }

            registry[meta->id] = *meta;
            loaded++;
        }
    }

    Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Info,
        "%i assets loaded from %s", (int)loaded, folder.c_str());
}

std::optional<Asset::assetInfo> Asset::loadMetadata(const std::string& fullPath) {
    // Check file exists
    std::ifstream f(fullPath);
    if (!f.is_open()) {
        Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
            "Asset error: Could not open asset file: %s", fullPath.c_str());
        return std::nullopt;
    }
    // Initialize JSON parser and pass file to it
    json j;
    try {
        f >> j;
    }
    catch (const std::exception& e) {
        Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
            "Asset error: JSON parse failure in %s — %s",
            fullPath.c_str(), e.what());
        return std::nullopt;
    }
    // Validate required fields
    if (!j.contains("id") || !j.contains("type")) {
        Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
            "Asset error: Missing required fields ('id', 'type') in %s",
            fullPath.c_str());
        return std::nullopt;
    }
	// Validate field types
    if (!j["id"].is_string() || !j["type"].is_string()) {
        Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
            "Asset error: 'id' and 'type' must be strings in %s",
            fullPath.c_str());
        return std::nullopt;
    }

    std::string typeStr = j["type"].get<std::string>();

    // Validate loader exists
    if (loaders.find(typeStr) == loaders.end()) {
        Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
            "Asset error: Unknown asset type '%s' in %s",
            typeStr.c_str(), fullPath.c_str());
        return std::nullopt;
    }

    // Construct metadata
    assetInfo info;
    info.id = j["id"].get<std::string>();
    info.assetFilePath = fullPath;
    info.type = loaders[typeStr].type;

    return info;
}

const Asset::assetInfo* Asset::get(std::string id) {
    auto it = registry.find(id);
    if (it == registry.end()) return nullptr;
    return &it->second;
}

void Asset::CreateDummyAsset()
{
    // Check if directory exists
    std::filesystem::path dirPath = "resource";
    if (!std::filesystem::exists(dirPath)) {
        Logger::log(Logger::LogCategory::InputOutput, Logger::LogLevel::Error, "Failed to open directory: %s", dirPath.c_str());
        return;
    }

    std::string id = GenerateUUID();

    nlohmann::ordered_json j;
    j["id"] = id;
    j["type"] = "defaultObject";
    j["textures"] = { "resource/test.png", "resource/test2.png" };
    j["texIndex"] = 0;
    j["hull"] = { 0, 0, 64, 64 };
    j["centered"] = true;
    j["fixed"] = false;
    j["flip"] = SDL_FLIP_NONE;
    j["scale"] = 1.0f;
    j["rot"] = 0.0;
    j["depth"] = 0;
    j["drawDefault"] = true;
    j["drawFlag"] = true;
    j["updateFlag"] = true;
    j["scripts"] = {"resource/dummy.lua"};

    std::ofstream f("resource/dummy.cea");
    if (!f.is_open()) {
        Logger::log(Logger::LogCategory::InputOutput, Logger::LogLevel::Error, "Failed to create file: ../resource/dummy.cea");
        return;
    }

    f << j.dump(4);
    f.close();
}

void Asset::init()
{
    // Register asset types
    registerObjectType<Object::defaultObject>("defaultObject", nullptr, assetType::DefaultObject);//change to defaultObject
    registerObjectType<Object::buttonObject>("buttonObject", nullptr, assetType::ButtonObject);

	// Scan library
    scanAssetDirectory("resource/");

    // Generate new dummy
    CreateDummyAsset();
}