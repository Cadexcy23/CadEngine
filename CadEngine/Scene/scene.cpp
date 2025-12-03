#include "scene.h"


bool Scene::objectsModified = true;
std::vector<std::shared_ptr<Object::engineObject>> Scene::activeObjects; //private?
std::vector<std::shared_ptr<Object::engineObject>> Scene::addObjects; //private?

std::shared_ptr<Object::engineObject> Scene::addObject(std::shared_ptr<Object::engineObject> obj)//make one of these privatee or seomthing
{
	addObjects.push_back(obj);
	return addObjects.back();
}

std::shared_ptr<Object::engineObject> registerObject(std::shared_ptr<Object::engineObject> obj)
{
	Scene::objectsModified = true;
	Scene::activeObjects.push_back(obj);
	return Scene::activeObjects.back();
}

void removeObject(std::shared_ptr<Object::engineObject> obj)
{
	//execute all of the objects despawn functions
	for (auto& func : obj->despawnFuncs)
		func(obj);

	//remove from active objects
	auto it = std::remove(Scene::activeObjects.begin(), Scene::activeObjects.end(), obj);
	Scene::activeObjects.erase(it, Scene::activeObjects.end());
}

void Scene::removeAllObjects()
{
	for (auto& obj : Scene::activeObjects) {
		obj->remove = true;
	}
}

void Scene::updateObjects()
{
	std::vector<std::shared_ptr<Object::engineObject>> remObjs = {};
	for (auto obj = activeObjects.rbegin(); obj != activeObjects.rend(); ++obj) {
		(*obj)->update();
		if ((*obj)->remove)
			remObjs.push_back(*obj);
	}
	while (remObjs.size() > 0)
	{
		removeObject(remObjs.back());
		remObjs.pop_back();
	}
	while (addObjects.size() > 0)
	{
		registerObject(addObjects.back());
		addObjects.pop_back();
	}
}