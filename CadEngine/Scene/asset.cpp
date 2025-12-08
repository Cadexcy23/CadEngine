#include "asset.h"
#include <random>
#include <chrono>
#include <filesystem>
#include "../Core/engine.h"


std::unordered_map<std::string, Asset::assetLoader> Asset::loaders;
std::unordered_map<std::string, Asset::assetInfo> Asset::registry;
std::unordered_map<std::string, std::weak_ptr<Object::engineObject>> Asset::cache;


namespace fs = std::filesystem;//dont do this


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
    for (auto& entry : fs::recursive_directory_iterator(folder)) {
        if (entry.path().extension() == ".cea") {
            assetInfo info = loadMetadata(entry.path().string());
            registry[info.id] = info;
        }
    }
    // Log the loaded assets
	Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Info, "%i assets loaded from %s", registry.size(), folder.c_str());
}

Asset::assetInfo Asset::loadMetadata(const std::string& fullPath) {
    std::ifstream f(fullPath);
    json j;
    f >> j;

    assetInfo info;
    info.id = j["id"].get<std::string>();
    info.assetFilePath = fullPath;

    std::string typeS = j["type"].get<std::string>();
    info.type = loaders[typeS].type;

    return info;
}

const Asset::assetInfo* Asset::get(std::string id) {
    auto it = registry.find(id);
    if (it == registry.end()) return nullptr;
    return &it->second;
}

void Asset::defaultLoad(json j, std::shared_ptr<Object::engineObject> obj) {
    // load textures
    if (j.contains("textures")) {
        for (const auto& texPath : j["textures"]) {
            SDL_Texture* t = Texture::loadTex(texPath.get<std::string>().c_str());
            if (t) obj->textures.push_back(t);
        }
    }
    if (!obj->textures.size())
        obj->textures.push_back(Texture::loadTex("resource/icon.png"));

    obj->texIndex = j.value("texIndex", 0);

    // load hull
    obj->hull = { 0,0,10,10 };
    if (j.contains("hull")) {
        auto h = j["hull"];
        obj->hull = { h[0], h[1], h[2], h[3] };
    }

    obj->centered = j.value("centered", true);
    obj->fixed = j.value("fixed", false);
    obj->flip = static_cast<SDL_FlipMode>(j.value("flip", 0));
    obj->scale = j.value("scale", 1.0f);
    obj->rot = j.value("rot", 0.0);
    obj->depth = j.value("depth", 0);
    obj->drawDefault = j.value("drawDefault", true);
    obj->drawFlag = j.value("drawFlag", true);
    obj->updateFlag = j.value("updateFlag", true);
}

void Asset::registerObjectType(std::string name, std::function<void(const json j, std::shared_ptr<Object::engineObject> obj)> loader, Asset::assetType type) {
    if (type == assetType::Unknown) 
        type = static_cast<assetType>(100 + loaders.size());
    loaders[name] = {name, loader, type};

	Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Info, "Registered asset type: %s ID: %i", name.c_str(), type);
}

void Asset::CreateDummyTextureAsset()
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
    j["type"] = "EngineObject";
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
    //register object types CHECK LOAD ORDER FOR REGISTERING CUSTOM TYPES
    registerObjectType("EngineObject", nullptr, assetType::EngineObject);
    registerObjectType("ButtonObject", nullptr, assetType::ButtonObject);

	//scan library
    scanAssetDirectory("resource/");

}