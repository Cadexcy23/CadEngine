#include "scene.h"
#include "../Core/logger.h"

bool Scene::objectsModified = true;
std::vector<std::shared_ptr<Object::engineObjectBase>> Scene::activeObjects;
std::vector<std::shared_ptr<Object::engineObjectBase>> Scene::addObjects;


void Scene::removeAllObjects()
{
	for (auto& obj : Scene::activeObjects) {
		obj->remove = true;
	}
}

void Scene::updateObjects() {
    // Update all objects
    for (auto& obj : activeObjects) {
        obj->update();
    }

    // Remove objects marked for removal
    auto it = std::remove_if(activeObjects.begin(), activeObjects.end(),
        [](auto& obj) { 
            return obj->remove; });
    activeObjects.erase(it, activeObjects.end());

    // Add new objects
    for (auto& obj : addObjects) {
        activeObjects.push_back(obj);
        Scene::objectsModified = true;
    }
    addObjects.clear();
}