#include "example.h"
#include "CadEngine/Core/engine.h"
#include "CadEngine/InputOutput/lua.h"
#include <string>


//EXAMPLE CODE
std::vector<std::shared_ptr<Network::netObject>> netObjs;
bool follow = false;


//ENGINE OBJECTS FUNCS
void perpetuate(std::shared_ptr<Example::velObject> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		obj->hull.x += obj->vel.x * Time::deltaSeconds;
		obj->hull.y += obj->vel.y * Time::deltaSeconds;
		obj->vel.x -= (obj->vel.x * Time::deltaSeconds) / 2;
		obj->vel.y -= (obj->vel.y * Time::deltaSeconds) / 2;

		obj->rot += obj->spin * Time::deltaSeconds;
		obj->spin -= (obj->spin * Time::deltaSeconds) / 2;
	}
}

void keepInScreen(std::shared_ptr<Example::velObject> obj)
{
	int left = 0 + (obj->hull.w / 2) * obj->scale;
	int right = Renderer::baseRes.x - (obj->hull.w / 2) * obj->scale;
	int up = 0 + (obj->hull.h / 2) * obj->scale;
	int down = Renderer::baseRes.y - (obj->hull.h / 2) * obj->scale;
	if (obj->hull.x < left)
	{
		obj->hull.x = left;
		if (obj)
			obj->vel.x *= -1;
	}
	else if (obj->hull.x > right)
	{
		obj->hull.x = right;
		if (obj)
			obj->vel.x *= -1;
	}
	if (obj->hull.y < up)
	{
		obj->hull.y = up;
		if (obj)
			obj->vel.y *= -1;
	}
	else if (obj->hull.y > down)
	{
		obj->hull.y = down;
		if (obj)
			obj->vel.y *= -1;
	}
}

void steer(std::shared_ptr<Object::engineObjectBase> obj)
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

void spawnTest(float x, float y, const std::string& assetID)  {
	auto obj = Scene::addObject(Asset::load<Example::velObject>(assetID));
	
	//netObjs.push_back(Network::server.registerAndSpawnNetworkObject(obj, assetID));
	 
	
	//auto p = std::make_shared<Example::velObject>(SDL_FRect{ x,y,32,32 });
	// set server update funcs
	/*p->addUpdateFunc(keepInScreen);
	p->addUpdateFunc(steer);
	p->addUpdateFunc(perpetuate);*/
	obj->addUpdateFunc([](std::shared_ptr<Object::engineObjectBase> obj) {
		if (clock() >= obj->timeCreated + 1000) {
			obj->remove = true;
		}
		});

	obj->addDespawnFunc([assetID](std::shared_ptr<Object::engineObjectBase> obj) {
		int randomChance = rand() % 2;
		if (randomChance == 1)
			spawnTest(obj->hull.x, obj->hull.y, assetID);
	});
		
	obj->hull.x = x;
	obj->hull.y = y;
	SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
	obj->vel = vel;
	obj->rot = rand() % 360;
	obj->scale = float(rand() % 1000) / 1000.0 + 0.5;
	obj->spin = (double(rand() % 20000) / 1000.0 - 10.0) *100;

	//only run here when running a server with rendering active
	//p->textures.push_back(Texture::loadTex(assetID.c_str()));
	
	netObjs.push_back(Network::server.registerAndSpawnNetworkObject(obj, assetID));
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
		Renderer::hullDebugDraw = !Renderer::hullDebugDraw;
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
		spawnTest(Input::mousePos.x, Input::mousePos.y, "51eef2418bd189a9977230ec58838030");
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
			//create a timer to broadcast snapshots at 60 ticks a second (16.67 ms per tick)
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
	}
	if (Input::keyStates[SDL_SCANCODE_B] == 1)
	{
		auto obj = Scene::addObject(Asset::load<Example::velObject>("52eef2418bd189a9977230ec58838030"));
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
	Asset::registerObjectType<Example::velObject>("velObject", velLoad);

	// register velObject as a lua type
	Lua::registerType<Example::velObject>("velObject",
		"vel", &Example::velObject::vel,
		"spin", &Example::velObject::spin,
		"flipVel", &Example::velObject::flipVel,
		"flipVelX", [](Example::velObject& obj) { obj.vel.x *= -1; },
		"flipVelY", [](Example::velObject& obj) { obj.vel.y *= -1; }
	);
	// register custom engine functions
	Lua::registerEngineFunction("printObjectCount",
		[]() {
			Logger::log(Logger::LogCategory::General, Logger::LogLevel::Info, "Object count: %i", Scene::activeObjects.size());
		});

	//load font
	Text::loadFont("resource/font/segoeuithibd.ttf", 32);
	Text::loadFont("resource/font/segoeuithisi.ttf", 32);

	//register engine object functions
	Asset::registerObjectFunc<Example::velObject>("perpetuate", perpetuate);
	Asset::registerObjectFunc<Example::velObject>("keepInScreen", keepInScreen);
	Asset::registerObjectFunc<Object::engineObjectBase>("steer", steer);
}

void exampleLoad()
{
	//set cam pos
	Renderer::camPos = { float(Renderer::baseRes.x / 2), float(Renderer::baseRes.y / 2) };

	//BG
	Scene::addObject(Asset::load<Example::velObject>("8bdf7db499ca7d7e0e8b83cb5c8ee486"));
	
	//Watermark
	auto watermark = Scene::addObject(Asset::load<Example::velObject>("cb431606fdc5a2128b23bfcbdc4f042d"));
	watermark->textures[0] = Text::loadText("CadEngine", Text::loadFont("resource/font/segoeuithibd.ttf", 32));
	watermark->resetSize();
	watermark->hull.x -= watermark->hull.w;
	watermark->hull.y -= watermark->hull.h;

	// Quit button
	auto quitButton = Scene::addObject(Asset::load<Object::buttonObject>("e51d80cd2e5dab0bf9b41e7b7fc2afde"));
	quitButton->textures[0] = Text::loadText("Quit", Text::loadFont("resource/font/segoeuithibd.ttf", 32));
	quitButton->resetSize();
	quitButton->hull.x -= quitButton->hull.w;
	quitButton->onClick = []() { Engine::quit = true; };

	////controls
	//SDL_Texture* conTex = Text::loadText("Left Click - Spawn Object   Right Click - Delete Object   1 - Vsync Toggle   2 - FPS Toggle   3 - Debug Level   4 - Pause Updates", bold, { 255, 255, 255, 255 });
	//SDL_GetTextureSize(conTex, &w, &h);
	//SDL_FRect conHull = { 0, Renderer::baseRes.y - h, w, h };
	//std::shared_ptr<Object::engineObject> conObj = std::make_shared<Object::engineObject>(Object::engineObject(conHull, { conTex }, 0, false, true));
	//conObj->depth = -1;
	//Scene::addObject(conObj);

	////controls 2
	//SDL_Texture* conBTex = Text::loadText("WASD - Steer Objects   -/+ - Adjust Object Size   Q/E - Spin   Space - Speed Boost   P - Print Object Count   F - Toggle Follow", bold, { 255, 255, 255, 255 });
	//SDL_GetTextureSize(conBTex, &w, &h);
	//SDL_FRect conBHull = { 0, Renderer::baseRes.y - h - 40, w, h };
	//std::shared_ptr<Object::engineObject> conBObj = std::make_shared<Object::engineObject>(Object::engineObject(conBHull, { conBTex }, 0, false, true));
	//conBObj->depth = -1;
	//Scene::addObject(conBObj);

	////controls 3
	//SDL_Texture* conCTex = Text::loadText("Arrow Keys - Move Camera   Mouse Wheel - Zoom Camera   ,/. - Cycle Texture   Z - Start Server   X - Connect to Server", bold, { 255, 255, 255, 255 });
	//SDL_GetTextureSize(conCTex, &w, &h);
	//SDL_FRect conCHull = { 0, Renderer::baseRes.y - h - 80, w, h };
	//std::shared_ptr<Object::engineObject> conCObj = std::make_shared<Object::engineObject>(Object::engineObject(conCHull, { conCTex }, 0, false, true));
	//conCObj->depth = -1;
	//Scene::addObject(conCObj);

	////controls 4
	///*SDL_Texture* conDTex = Text::loadText("Arrow Keys - Move Camera   Mouse Wheel - Zoom Camera   ,/. - Cycle Texture", bold, { 255, 255, 255, 255 });
	//SDL_GetTextureSize(conDTex, &w, &h);
	//SDL_FRect conDHull = { 0, Renderer::baseRes.y - h - 120, w, h };
	//std::shared_ptr<Object::engineObject> conDObj = std::make_shared<Object::engineObject>(Object::engineObject(conDHull, conDTex, 0, false, true));
	//conDObj->depth = -1;
	//Scene::addObject(conDObj);*/

}
//EXAMPLE CODE END


int main(int argc, char* argv[])
{
	//Initialize Engine (subsystems, window, renderer, etc)
	Engine::initEngine();

	//Initialize example (load custom objects, lua bindings, etc)
	exampleInit();

	//Initialize asset system (after engine init and example init)
	Asset::init();

	//Load example scene (after everything is registered)
	exampleLoad();

	while (!Engine::quit) {
		//Process input
		Engine::update();

		//Update custom project specific functionality
		engineControls();

		//Render scene
		Renderer::draw();
	}

	return 0;
}