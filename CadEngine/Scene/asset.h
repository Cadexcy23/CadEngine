#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include "../Scene/object.h"
#include "../json.hpp"
#include "../Core/logger.h"

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
        std::function<void(const json j, std::shared_ptr<Object::engineObject> obj)> loader;
        assetType type;
	};

    struct assetInfo {
        std::string id;
        assetType type;
        std::string assetFilePath;  // path to the .cea metadata file
    };


    
        
   static void scanAssetDirectory(const std::string& folder);
   
   static assetInfo loadMetadata(const std::string& fullPath);

   static const assetInfo* get(std::string id);

   static void defaultLoad(json j, std::shared_ptr<Object::engineObject> obj);

   static std::string GenerateUUID();

   static void CreateDummyTextureAsset();

   static void registerObjectType(std::string name, std::function<void(const json j, std::shared_ptr<Object::engineObject> obj)> loader, Asset::assetType type = assetType::Unknown);
   
   static void init();

   
   static std::unordered_map<std::string, assetLoader> loaders;
   static std::unordered_map<std::string, assetInfo> registry;
   static std::unordered_map<std::string, std::weak_ptr<Object::engineObject>> cache;


   template<typename T> //add a load from path/name?
   static std::shared_ptr<T> load(const std::string& id) {
       static_assert(std::is_base_of_v<Object::engineObject, T>,
           "T must inherit from engineObject");

       // check cache first then load from prototype?
       //if (auto it = cache.find(id); it != cache.end()) {
       //    if (auto existing = it->second.lock()) {
       //        return existing; //make a seperate path that makes a copy?
       //    }
       //}

       const assetInfo* info = get(id);
       if (!info) {
           Logger::log(Logger::LogCategory::Scene, Logger::LogLevel::Error, "Asset with ID %s not found in registry", id.c_str());
           return nullptr;
       }

       // load metadata
       std::ifstream f(info->assetFilePath);//error check?
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