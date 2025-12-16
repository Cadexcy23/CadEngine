#include "example.h"
#include "CadEngine/Core/engine.h"
#include "CadEngine/InputOutput/lua.h"
#include <string>


//EXAMPLE CODE
std::vector<std::shared_ptr<Network::netObject>> netObjs;
bool follow = false;


//ENGINE OBJECTS FUNCS
void quitProgram()
{
	Engine::quit = true;
}

void tempUpdateFunc(std::shared_ptr<Object::engineObjectBase> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get downcast object
		auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);

		obj->hull.x += devObj->vel.x * Time::deltaSeconds;
		obj->hull.y += devObj->vel.y * Time::deltaSeconds;
		devObj->vel.x -= (devObj->vel.x * Time::deltaSeconds) / 2;
		devObj->vel.y -= (devObj->vel.y * Time::deltaSeconds) / 2;

		obj->rot += devObj->spin * Time::deltaSeconds;
		devObj->spin -= (devObj->spin * Time::deltaSeconds) / 2;
	}
}

void keepInScreen(std::shared_ptr<Example::velObject> obj)
{
	//get downcast object
	auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);

	int left = 0 + (obj->hull.w / 2) * obj->scale;
	int right = Renderer::baseRes.x - (obj->hull.w / 2) * obj->scale;
	int up = 0 + (obj->hull.h / 2) * obj->scale;
	int down = Renderer::baseRes.y - (obj->hull.h / 2) * obj->scale;
	if (obj->hull.x < left)
	{
		obj->hull.x = left;
		if (devObj)
			devObj->vel.x *= -1;
	}
	else if (obj->hull.x > right)
	{
		obj->hull.x = right;
		if (devObj)
			devObj->vel.x *= -1;
	}
	if (obj->hull.y < up)
	{
		obj->hull.y = up;
		if (devObj)
			devObj->vel.y *= -1;
	}
	else if (obj->hull.y > down)
	{
		obj->hull.y = down;
		if (devObj)
			devObj->vel.y *= -1;
	}
}

void keyboardControl(std::shared_ptr<Object::engineObjectBase> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get downcast object
		auto velObj = std::dynamic_pointer_cast<Example::velObject>(obj);
		if (Input::keyStates[SDL_SCANCODE_COMMA] == 1)
		{
			obj->texIndex--;
			if (obj->texIndex < 0)
				obj->texIndex = obj->textures.size() - 1;
		}
		if (Input::keyStates[SDL_SCANCODE_PERIOD] == 1)
		{
			obj->texIndex++;
			obj->texIndex = obj->texIndex % obj->textures.size();
		}
		if (Input::keyStates[SDL_SCANCODE_MINUS])
			obj->scale *= 0.95;
		if (Input::keyStates[SDL_SCANCODE_EQUALS])
			obj->scale *= 1.05;
		if (velObj)
		{
			if (Input::keyStates[SDL_SCANCODE_W])
				velObj->vel.y -= 100 * Time::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_S])
				velObj->vel.y += 100 * Time::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_A])
				velObj->vel.x -= 100 * Time::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_D])
				velObj->vel.x += 100 * Time::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_Q])
				velObj->spin -= 360 * Time::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_E])
				velObj->spin += 360 * Time::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_SPACE] == 1)
			{
				velObj->vel.x *= 2;
				velObj->vel.y *= 2;
			}
		}
	}
}

void spawnTest(float x, float y, const std::string& texturePath)  {
	auto p = std::make_shared<Example::velObject>(SDL_FRect{ x,y,32,32 });
	// set server update funcs
	p->addUpdateFunc(keepInScreen);
	/*p->updateFuncs.push_back(keepInScreen);
	p->updateFuncs.push_back(tempUpdateFunc);
	p->updateFuncs.push_back(keyboardControl);
	p->updateFuncs.push_back([](std::shared_ptr<Object::engineObject> obj) {
		if (clock() >= obj->timeCreated + 1000) {
			obj->remove = true;
		}
		});

	p->despawnFuncs.push_back([](std::shared_ptr<Object::engineObject> obj) {
		int randomChance = rand() % 2;
		if (randomChance == 1)
			spawnTest(obj->hull.x, obj->hull.y, "resource/test.png");
	});*/
		

	SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
	p->vel = vel;
	p->rot = rand() % 360;
	p->scale = float(rand() % 1000) / 1000.0 + 0.5;
	p->spin = (double(rand() % 20000) / 1000.0 - 10.0) *100;

	//only run here when running a server with rendering active
	p->textures.push_back(Texture::loadTex(texturePath.c_str()));
	
	//netObjs.push_back(Network::server.registerAndSpawnNetworkObject(Scene::addObject(p), texturePath));
}

void despawnTest() {
	if(netObjs.size() > 0)
	{
		netObjs.back()->obj->remove = true;
		Network::server.broadcastDespawn(netObjs.back()->netID);
		netObjs.pop_back();
	}
}

void engineControls()
{

	/*if (follow && objs.size())
	{
		Renderer::camPos.x = objs.front()->hull.x;
		Renderer::camPos.y = objs.front()->hull.y;
	}*/

	if (Input::wheelStates[0])
	{
		Renderer::zoom *= 1.05;
	}
	if (Input::wheelStates[1])
	{
		Renderer::zoom *= .95;
	}
	if (Input::keyStates[SDL_SCANCODE_UP])
	{
		Renderer::camPos.y -= 5.0 / Renderer::zoom;
	}
	if (Input::keyStates[SDL_SCANCODE_DOWN])
	{
		Renderer::camPos.y += 5.0 / Renderer::zoom;
	}
	if (Input::keyStates[SDL_SCANCODE_LEFT])
	{
		Renderer::camPos.x -= 5.0 / Renderer::zoom;
	}
	if (Input::keyStates[SDL_SCANCODE_RIGHT])
	{
		Renderer::camPos.x += 5.0 / Renderer::zoom;
	}
	if (Input::keyStates[SDL_SCANCODE_ESCAPE])
		Engine::quit = true;
	if (Input::keyStates[SDL_SCANCODE_1] == 1)
	{
		Renderer::toggleVsync();
	}
	if (Input::keyStates[SDL_SCANCODE_2] == 1)
	{
		if (Engine::showFPS)
			Engine::showFPS = false;
		else
			Engine::showFPS = true;
	}
	if (Input::keyStates[SDL_SCANCODE_3] == 1)
	{
		if (Engine::debugLevel > 1)
			Engine::debugLevel = 0;
		else
			Engine::debugLevel++;
	}
	if (Input::keyStates[SDL_SCANCODE_4] == 1)
	{
		if (Engine::engineState != Engine::STATE_PAUSE)
			Engine::engineState = Engine::STATE_PAUSE;
		else
			Engine::engineState = Engine::STATE_DEFAULT;
	}
	if (Input::keyStates[SDL_SCANCODE_P] == 1)
	{
		Logger::log(Logger::LogCategory::General, Logger::LogLevel::Info, "Object count: %i", Scene::activeObjects.size());
	}
	if (Input::keyStates[SDL_SCANCODE_F] == 1)
	{
		follow = !follow;
	}
	
	if (Input::mouseStates[0])
	{
		spawnTest(Input::mousePos.x, Input::mousePos.y, "resource/test.png");
		/*SDL_Texture* tex = Texture::loadTex("resource/test2.png");
		SDL_Texture* tex2 = Texture::loadTex("resource/test.png");
		SDL_Texture* tex3 = Texture::loadTex("resource/icon.png");*/
	}
	if (Input::mouseStates[2])
	{
		despawnTest();
	}

	//Networking
	if (Input::keyStates[SDL_SCANCODE_Z] == 1)
	{
		//Start server
		if(!Network::server.isRunning())
		{
			Network::server.start(27015);
			Time::timer* t = Time::createTimer(16.67, -1, nullptr);
			t->setCallback([t]() {
				
				Network::server.broadcastSnapshotToAllClients(t->getCurrent());
				});
		}
		else
		{
			Network::server.stop();
		}
	}
	if (Input::keyStates[SDL_SCANCODE_X] == 1)
	{
		//Start client
		if (!Network::client.isConnected())
		{
			Network::client.connectTo("10.0.0.139", 27015);
		}
		else
		{
			Network::client.disconnect();
		}
	}
	if (Input::keyStates[SDL_SCANCODE_C] == 1)
	{
		Asset::CreateDummyAsset();
	}
	if (Input::keyStates[SDL_SCANCODE_V] == 1)
	{
		auto obj = Scene::addObject(Asset::load<Example::velObject>("51eef2418bd189a9977230ec58838030"));
		/*obj->updateFuncs.push_back(keepInScreen);
		obj->updateFuncs.push_back(tempUpdateFunc);
		obj->updateFuncs.push_back(keyboardControl);*/
	}
	

	// custom process network messages
	Network::NetworkEvent e;
	while (Network::server.pollEvent(e)) {
		switch (e.type) {
		case Network::NetworkEventType::ClientConnected:
			// track client, send welcome, etc.
			Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Info, "Client connected: %s", e.data);
			break;
		case Network::NetworkEventType::MessageReceived:
			// e.socket is client socket (server) or server socket (client)
			// e.message contains payload
			Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Info, "Client said: %s", e.data);
			break;
		case Network::NetworkEventType::Error:
			// log or handle
			Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Error, "Client error: %s", e.data);
			break;
		default: break;
		}
	}

	while (Network::client.pollEvent(e)) {
		if (e.type == Network::NetworkEventType::MessageReceived) {
			
			/*size_t idx = 0;
			Network::NetMsgType type = static_cast<Network::NetMsgType>(Serialization::read_u8(e.data, idx));
			std::string msg(reinterpret_cast<const char*>(e.data.data()), e.data.size());
			Logger::log(Logger::LogCategory::Network, Logger::LogLevel::Info, "Server said: %s", msg);*/
		}
	}
}

void velLoad(const json j, std::shared_ptr<Object::engineObjectBase> obj)
{
	//downcast object
	auto dObj = std::dynamic_pointer_cast<Example::velObject>(obj);

	//load custom properties
	dObj->vel = { 0,0 };
	if (j.contains("velocity")) {
		auto v = j["velocity"];
		dObj->vel = { v[0], v[1] };
	}
	dObj->spin = j.value("spin", 0);
}

void exampleInit()
{
	// Load custom object types
	Asset::registerObjectType("VelObject", velLoad);
	// register SDL_FPoint as a lua type
	Lua::registerTypeSimple<SDL_FPoint>("SDL_FPoint",
		"x", &SDL_FPoint::x,
		"y", &SDL_FPoint::y,
		"__tostring", [](const SDL_FPoint& v) {
			return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
		}
	);
	Lua::registerTypeSimple<SDL_FRect>("SDL_FRect",
		"x", &SDL_FRect::x,
		"y", &SDL_FRect::y,
		"w", &SDL_FRect::w,
		"h", &SDL_FRect::h
	);
	// register velObject as a lua type
	Lua::registerType<Example::velObject>("VelObject",
		"vel", &Example::velObject::vel,
		"spin", &Example::velObject::spin,
		"flipVel", &Example::velObject::flipVel,
		"flipVelX", [](Example::velObject& obj) { obj.vel.x *= -1; },
		"flipVelY", [](Example::velObject& obj) { obj.vel.y *= -1; }
	);
	//// register custom engine functions
	Lua::registerEngineFunction("isKeyPressed",
		[](const char* key) -> bool {
			return Input::keyStates[SDL_GetScancodeFromName(key)];
		});
	
	//{
	//	//set cam pos
	//	Renderer::camPos = { float(Renderer::baseRes.x / 2), float(Renderer::baseRes.y / 2) };

	//	std::shared_ptr<Object::engineObject> bg = Scene::addObject(std::make_shared<Object::engineObject>(Object::engineObject({ 0, 0, float(Renderer::baseRes.x), float(Renderer::baseRes.y) }, { Texture::loadTex("resource/bg.png") }, 0, false)));
	//	bg->depth = 1;

	//	TTF_Font* bold = Text::loadFont("resource/font/segoeuithibd.ttf", 32);
	//	Text::loadFont("resource/font/segoeuithisi.ttf", 32);

	//	SDL_Texture* textures = Text::loadText("CadEngine", bold, { 255, 255, 255, 255 });
	//	float w, h = 0;
	//	SDL_GetTextureSize(textures, &w, &h);
	//	SDL_FRect hull = { 0, 0, w, h };
	//	std::shared_ptr<Object::engineObject> watermark = Scene::addObject(std::make_shared<Object::engineObject>(Object::engineObject(hull, { textures }, 0, false, true)));
	//	watermark->depth = -1;

	//	//quit button
	//	SDL_Texture* quitTex = Text::loadText("Quit", bold, { 255, 255, 255, 255 });
	//	SDL_GetTextureSize(quitTex, &w, &h);
	//	SDL_FRect quitHull = { Renderer::baseRes.x - w, 0, w, h };
	//	std::shared_ptr<Object::buttonObject> quitButton = std::make_shared<Object::buttonObject>(Object::buttonObject(quitHull, { quitTex }, 0, false, true));
	//	quitButton->onClick = quitProgram;
	//	auto quitObject = Scene::addObject(quitButton);
	//	quitObject->depth = -1;

	//	//controls
	//	SDL_Texture* conTex = Text::loadText("Left Click - Spawn Object   Right Click - Delete Object   1 - Vsync Toggle   2 - FPS Toggle   3 - Debug Level   4 - Pause Updates", bold, { 255, 255, 255, 255 });
	//	SDL_GetTextureSize(conTex, &w, &h);
	//	SDL_FRect conHull = { 0, Renderer::baseRes.y - h, w, h };
	//	std::shared_ptr<Object::engineObject> conObj = std::make_shared<Object::engineObject>(Object::engineObject(conHull, { conTex }, 0, false, true));
	//	conObj->depth = -1;
	//	Scene::addObject(conObj);

	//	//controls 2
	//	SDL_Texture* conBTex = Text::loadText("WASD - Steer Objects   -/+ - Adjust Object Size   Q/E - Spin   Space - Speed Boost   P - Print Object Count   F - Toggle Follow", bold, { 255, 255, 255, 255 });
	//	SDL_GetTextureSize(conBTex, &w, &h);
	//	SDL_FRect conBHull = { 0, Renderer::baseRes.y - h - 40, w, h };
	//	std::shared_ptr<Object::engineObject> conBObj = std::make_shared<Object::engineObject>(Object::engineObject(conBHull, { conBTex }, 0, false, true));
	//	conBObj->depth = -1;
	//	Scene::addObject(conBObj);

	//	//controls 3
	//	SDL_Texture* conCTex = Text::loadText("Arrow Keys - Move Camera   Mouse Wheel - Zoom Camera   ,/. - Cycle Texture   Z - Start Server   X - Connect to Server", bold, { 255, 255, 255, 255 });
	//	SDL_GetTextureSize(conCTex, &w, &h);
	//	SDL_FRect conCHull = { 0, Renderer::baseRes.y - h - 80, w, h };
	//	std::shared_ptr<Object::engineObject> conCObj = std::make_shared<Object::engineObject>(Object::engineObject(conCHull, { conCTex }, 0, false, true));
	//	conCObj->depth = -1;
	//	Scene::addObject(conCObj);

	//	//controls 4
	//	/*SDL_Texture* conDTex = Text::loadText("Arrow Keys - Move Camera   Mouse Wheel - Zoom Camera   ,/. - Cycle Texture", bold, { 255, 255, 255, 255 });
	//	SDL_GetTextureSize(conDTex, &w, &h);
	//	SDL_FRect conDHull = { 0, Renderer::baseRes.y - h - 120, w, h };
	//	std::shared_ptr<Object::engineObject> conDObj = std::make_shared<Object::engineObject>(Object::engineObject(conDHull, conDTex, 0, false, true));
	//	conDObj->depth = -1;
	//	Scene::addObject(conDObj);*/

	//	//TEMP
	//	//quit button
	//	SDL_Texture* quitTestTex = Text::loadText("Quit", bold, { 255, 255, 255, 255 });
	//	SDL_GetTextureSize(quitTestTex, &w, &h);
	//	SDL_FRect quitTestHull = { Renderer::baseRes.x - w, 0, w, h };
	//	std::shared_ptr<Object::buttonObject> quitTestButton = std::make_shared<Object::buttonObject>(Object::buttonObject(quitTestHull, { quitTestTex }, 0, false, false));
	//	quitTestButton->onClick = quitProgram;
	//	std::shared_ptr<Object::buttonObject> quitTestObject = Scene::addObject(quitTestButton);
	//	Scene::addObjects.push_back(std::make_unique<Scene::sceneObject<Object::buttonObject>>(quitTestButton));
	//	quitTestObject->depth = -10;
	//}
}
//EXAMPLE CODE END


int main(int argc, char* argv[])
{
	//Initialize Engine 
	Engine::initEngine();

	//TEMP INIT
	Lua::init();

	//Initialize modules
	exampleInit();

	//TEMP INIT
	Asset::init();

	while (!Engine::quit) {
		//Process input
		Engine::update();

		//Update modules here
		engineControls();

		//Render scene
		Renderer::draw();
	}

	return 0;
}