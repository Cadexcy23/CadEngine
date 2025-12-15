#pragma once
#include "object.h"


static class Scene {
public:
	static std::vector<std::shared_ptr<Object::engineObjectBase>> activeObjects;
	static std::vector<std::shared_ptr<Object::engineObjectBase>> addObjects;
	static bool objectsModified; //MOVE TO RENDERER and make private with a func to "dirty"
	//static std::shared_ptr<Object::engineObject> addObject(std::shared_ptr<Object::engineObject> obj);
	template<typename T>
	static std::shared_ptr<T> addObject(std::shared_ptr<T> obj) {
		// Static assert to ensure T derives from engineObjectBase
		static_assert(std::is_base_of_v<Object::engineObjectBase, T>,
			"T must inherit from engineObjectBase");
		addObjects.push_back(obj);
		return obj;
	}
	static void updateObjects();
	
	static void removeAllObjects();

};