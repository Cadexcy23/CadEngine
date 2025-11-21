#pragma once
#include "object.h"


static class Scene {
public:
	static bool objectsModified;
	static std::vector<std::shared_ptr<Object::engineObject>> activeObjects;
	static std::vector<std::shared_ptr<Object::engineObject>> addObjects;

	static std::shared_ptr<Object::engineObject> addObject(std::shared_ptr<Object::engineObject> obj);
	static void removeAllObjects();
	static void updateObjects();

};