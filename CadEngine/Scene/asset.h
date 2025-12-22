#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include "../Scene/object.h"
#include "../json.hpp"
#include "../Core/logger.h"
#include "../InputOutput/lua.h"

using json = nlohmann::json;


static class Asset {
public:

    enum class assetType {
        Unknown = 0,
        DefaultObject,
        ButtonObject,
        Texture,
        Sound,
        COUNT
        // custom types added dynamically starting at COUNT
    };

    struct assetLoader {
        std::string name;
        assetType type;
        std::function<void(const json j, std::shared_ptr<Object::engineObjectBase> obj)> loader;
        std::function<std::shared_ptr<Object::engineObjectBase>()> creator;
	};

    struct assetInfo {
        std::string id;
        assetType type;
        std::string assetFilePath;
    };

   
    static void scanAssetDirectory(const std::string& folder);
    static std::optional<Asset::assetInfo> loadMetadata(const std::string& fullPath);
    static const assetInfo* get(std::string id);
    static std::string GenerateUUID();
    static void CreateDummyAsset();
    //static void registerObjectType(std::string name, std::function<void(const json j, std::shared_ptr<Object::engineObjectBase> obj)> loader, Asset::assetType type = assetType::Unknown);
    static void init();

    static std::unordered_map<std::string, assetLoader> loaders;
    static std::unordered_map<std::string, assetInfo> registry;
    static std::unordered_map<std::string, std::weak_ptr<Object::engineObjectBase>> cache;
    
    
    
    template<typename Derived>
    static void registerObjectType(std::string name, std::function<void(const json j, std::shared_ptr<Object::engineObjectBase> obj)> loader, Asset::assetType type = assetType::Unknown) {
    if (type == assetType::Unknown)
            type = static_cast<assetType>(int(assetType::COUNT) + loaders.size());
        loaders[name] = { name, type, loader,  
        []() -> std::shared_ptr<Object::engineObjectBase> { 
                return std::make_shared<Derived>();
            } 
        };

        Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Info, "Registered asset type: %s ID: %i", name.c_str(), type);
    }

    static void defaultLoad(const json& j, std::shared_ptr<Object::engineObjectBase> obj) {
        // load textures
        if (j.contains("textures")) {
            for (const auto& texPath : j["textures"]) {
                SDL_Texture* t = Texture::loadTex(texPath.get<std::string>().c_str());
                if (t) obj->textures.push_back(t);
            }
        }
        if (!obj->textures.size())
        {
            Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
                "Asset with ID %s has no textures, default loaded.", j.value("id", "NULL").c_str());
            obj->textures.push_back(Texture::loadTex("resource/icon.png"));
        }

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

        // load functions
        if (j.contains("functions")) {
            obj->loadFunctions(j);
        }

        // load scripts
        /*if (j.contains("scripts")) {
            for (const auto& scriptPath : j["scripts"]) {
                std::string path = scriptPath.get<std::string>();
                Lua::attachScript(path, obj);
            }
        }*/
    }
    static std::shared_ptr<Object::engineObjectBase> load(const std::string& assetID) {
        const assetInfo* info = get(assetID);
        if (!info) {
            Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
                "Asset with ID %s not found in registry", assetID.c_str());
            return nullptr;
        }

        // load metadata
        std::ifstream f(info->assetFilePath);
        json j;
        f >> j;

        // Get type
        std::string typeS = j["type"].get<std::string>();

        // Create the derived object
        auto obj = loaders.find(typeS)->second.creator();
        //obj = std::dynamic_pointer_cast<Object::engineObject>(obj);

        // Default load
        defaultLoad(j, obj);

        // Check if the derived type has a custom loader
        if (auto cusLoad = loaders[typeS].loader)
            cusLoad(j, obj);

        return obj;

        //// Get type from JSON
        //std::string typeName = j["type"].get<std::string>();

        //// Look up the loader
        //auto it = loaders.find(typeName);
        //if (it == loaders.end()) {
        //    Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
        //        "Unregistered object type: %s", typeName.c_str());
        //    return nullptr;
        //}

        //// Create using registered creator
        //auto obj = it->second.creator();
        //if (!obj) {
        //    return nullptr;
        //}

        //// Default load
        //defaultLoad(j, obj);

        //// Custom loader if available
        //if (it->second.loader) {
        //    it->second.loader(j, obj);
        //}

        //return obj;
    }
    template<typename T>
    static std::shared_ptr<T> load(const std::string& assetID) {
        static_assert(std::is_base_of_v<Object::engineObjectBase, T>,
            "T must inherit from engineObjectBase");
        
        // check cache first then load from prototype?
        //if (auto it = cache.find(id); it != cache.end()) {
        //    if (auto existing = it->second.lock()) {
        //        return existing; //make a seperate path that makes a copy?
        //    }
        //}
        
        const assetInfo* info = get(assetID);
        if (!info) {
            Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
                "Asset with ID %s not found in registry", assetID.c_str());
            return nullptr;
        }
        
        // load metadata
        std::ifstream f(info->assetFilePath);
        json j;
        f >> j;
        
        // Get type
        std::string typeS = j["type"].get<std::string>();

        // Create the derived object
        auto obj = std::make_shared<T>();

        // Default load
        defaultLoad(j, obj);
        
	    // Check if the derived type has a custom loader
        if(auto cusLoad = loaders[typeS].loader)
            cusLoad(j, obj);
        
        return obj;
    }
};