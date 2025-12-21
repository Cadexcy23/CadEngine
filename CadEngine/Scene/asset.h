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
        EngineObject,
        ButtonObject,
        Texture,
        // custom types added dynamically starting at 100
    };

    struct assetLoader {
        std::string name;
        std::function<void(const json j, std::shared_ptr<Object::engineObjectBase> obj)> loader;
        assetType type;
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
   static void registerObjectType(std::string name, std::function<void(const json j, std::shared_ptr<Object::engineObjectBase> obj)> loader, Asset::assetType type = assetType::Unknown);
   static void init();

   static std::unordered_map<std::string, assetLoader> loaders;
   static std::unordered_map<std::string, assetInfo> registry;
   static std::unordered_map<std::string, std::weak_ptr<Object::engineObjectBase>> cache;
   /*template<typename Derived>
   static std::unordered_map<std::string, std::function<void(std::shared_ptr<Derived>)>> funcRegistry;*/

   //template<typename Derived>
   //static void registerObjectFunc(const std::string& name, std::function<void(std::shared_ptr<Derived>)> func) {
	  // //add check for unique name
   //    funcRegistry[name] = func;
   //}
   /*template<typename Derived>
   static std::function<void(std::shared_ptr<Derived>)> getObjectFunc(const std::string& name) {
       auto it = funcRegistry.find(name);
       return (it != funcRegistry.end()) ? it->second : nullptr;
   }*/
   template<typename T>
   static void defaultLoad(const json& j, std::shared_ptr<T> obj) {
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
               "Asset with ID %s has no textures, default loaded.", j.value("id", "NULL"));
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

       // load scripts
       if (j.contains("scripts")) {
           for (const auto& scriptPath : j["scripts"]) {
               std::string path = scriptPath.get<std::string>();
               Lua::attachScript(path, obj);
           }
       }
   }
   template<typename T>
   static std::shared_ptr<T> load(const std::string& id) {
       static_assert(std::is_base_of_v<Object::engineObjectBase, T>,
           "T must inherit from engineObjectBase");

       // check cache first then load from prototype?
       //if (auto it = cache.find(id); it != cache.end()) {
       //    if (auto existing = it->second.lock()) {
       //        return existing; //make a seperate path that makes a copy?
       //    }
       //}

       const assetInfo* info = get(id);
       if (!info) {
           Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error,
               "Asset with ID %s not found in registry", id.c_str());
           return nullptr;
       }

       // load metadata
       std::ifstream f(info->assetFilePath);
       json j;
       f >> j;

       // Create the derived object
       auto obj = std::make_shared<T>();

       // Default load
       defaultLoad(j, obj);

	   // Check if the derived type has a custom loader
       if(auto cusLoad = loaders[j["type"].get<std::string>()].loader)
           cusLoad(j, obj);

       return obj;
   }
};