#pragma once
#include "object.h"


static class Scene {
public:
	static std::vector<std::shared_ptr<Object::engineObjectBase>> activeObjects;
	static std::vector<std::shared_ptr<Object::engineObjectBase>> addObjects;
	static bool objectsModified;


	template<typename T>
	static std::shared_ptr<T> addObject(std::shared_ptr<T> obj) {
		static_assert(std::is_base_of_v<Object::engineObjectBase, T>,
			"T must inherit from engineObjectBase");
		addObjects.push_back(obj);
		return obj;
	}
	static void updateObjects();
	static void removeAllObjects();

};