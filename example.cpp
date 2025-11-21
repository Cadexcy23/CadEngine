#include "example.h"
#include "CadEngine/Core/engine.h"
#include <string>


//EXAMPLE CODE
std::vector<std::shared_ptr<Example::velObject>> objs;
bool follow = false;


//ENGINE OBJECTS FUNCS
void quitProgram()
{
	Engine::quit = true;
}

void tempUpdateFunc(std::shared_ptr<Object::engineObject> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get downcast object
		auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);

		obj->hull.x += devObj->vel.x * Timer::deltaSeconds;
		obj->hull.y += devObj->vel.y * Timer::deltaSeconds;
		devObj->vel.x -= (devObj->vel.x * Timer::deltaSeconds) / 2;
		devObj->vel.y -= (devObj->vel.y * Timer::deltaSeconds) / 2;

		obj->rot += devObj->spin * Timer::deltaSeconds;
		devObj->spin -= (devObj->spin * Timer::deltaSeconds) / 2;
	}
}

void keepInScreen(std::shared_ptr<Object::engineObject> obj)
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

void keyboardControl(std::shared_ptr<Object::engineObject> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get downcast object
		auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);
		if (Input::keyStates[SDL_SCANCODE_COMMA] == 1)
		{
			obj->texIndex--;
			if (obj->texIndex < 0)
				obj->texIndex = obj->tex.size() - 1;
		}
		if (Input::keyStates[SDL_SCANCODE_PERIOD] == 1)
		{
			obj->texIndex++;
			obj->texIndex = obj->texIndex % obj->tex.size();
		}
		if (Input::keyStates[SDL_SCANCODE_MINUS])
			obj->scale *= 0.95;
		if (Input::keyStates[SDL_SCANCODE_EQUALS])
			obj->scale *= 1.05;
		if (devObj)
		{
			if (Input::keyStates[SDL_SCANCODE_W])
				devObj->vel.y -= 100 * Timer::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_S])
				devObj->vel.y += 100 * Timer::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_A])
				devObj->vel.x -= 100 * Timer::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_D])
				devObj->vel.x += 100 * Timer::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_Q])
				devObj->spin -= 360 * Timer::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_E])
				devObj->spin += 360 * Timer::deltaSeconds;
			if (Input::keyStates[SDL_SCANCODE_SPACE] == 1)
			{
				devObj->vel.x *= 2;
				devObj->vel.y *= 2;
			}
		}
	}
}

void moveToMouse(std::shared_ptr<Object::engineObject> obj)
{
	obj->hull.x = Input::mousePos.x;
	obj->hull.y = Input::mousePos.y;
}

void engineControls()
{

	if (follow && objs.size())
	{
		Renderer::camPos.x = objs.front()->hull.x;
		Renderer::camPos.y = objs.front()->hull.y;
	}

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
		Logger::log(Logger::LogCategory::General, Logger::LogLevel::Info, "Object count: %i\n", objs.size());
	}
	if (Input::keyStates[SDL_SCANCODE_F] == 1)
	{
		follow = !follow;
	}
	if (Input::mouseStates[0])
	{
		SDL_Texture* tex = Texture::loadTex("resource/test2.png");
		SDL_Texture* tex2 = Texture::loadTex("resource/test.png");
		SDL_Texture* tex3 = Texture::loadTex("resource/icon.png");
		float w, h;
		SDL_GetTextureSize(tex, &w, &h);
		SDL_FRect hull = { Input::mousePos.x, Input::mousePos.y, w / 10, h / 10 };
		SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
		double rot = rand() % 360;
		float scale = float(rand() % 1000) / 1000.0 + 0.5;
		double spin = double(rand() % 20000) / 1000.0 - 10.0;

		std::shared_ptr<Example::velObject> velObj = std::make_shared<Example::velObject>(
			hull, tex, rot, true, false, SDL_FLIP_NONE, scale, vel, spin);
		velObj->updateFuncs.push_back(keepInScreen);
		velObj->updateFuncs.push_back(tempUpdateFunc);
		velObj->updateFuncs.push_back(keyboardControl);
		objs.push_back(velObj);
		velObj->tex.push_back(tex2);
		velObj->tex.push_back(tex3);
		Scene::addObject(velObj);
	}
	if (Input::mouseStates[2])
	{
		if (objs.size())
		{
			objs.back()->remove = true;
			objs.pop_back();
		}
	}
}

void exampleInit()
{
	//set cam pos
	Renderer::camPos = { float(Renderer::baseRes.x / 2), float(Renderer::baseRes.y / 2) };

	std::shared_ptr<Object::engineObject> bg = Scene::addObject(std::make_shared<Object::engineObject>(Object::engineObject({ 0, 0, float(Renderer::baseRes.x), float(Renderer::baseRes.y) }, Texture::loadTex("resource/bg.png"), 0, false)));
	bg->depth = 1;

	//std::shared_ptr<Object::engineObject> me = Engine::addObject(std::make_shared<Object::engineObject>(Object::engineObject({ float(rand() % Renderer::baseRes.x), float(rand() % Renderer::baseRes.y) , 16, 16 }, Engine::loadTex("resource/icon.png"))));
	//me->depth = -1;
	//me->updateFuncs.push_back(moveToMouse);

	TTF_Font* bold = Text::loadFont("resource/font/segoeuithibd.ttf", 32);
	Text::loadFont("resource/font/segoeuithisi.ttf", 32);

	SDL_Texture* tex = Text::loadText("CadEngine", bold, { 255, 255, 255, 255 });
	float w, h = 0;
	SDL_GetTextureSize(tex, &w, &h);
	SDL_FRect hull = { 0, 0, w, h };
	std::shared_ptr<Object::engineObject> watermark = Scene::addObject(std::make_shared<Object::engineObject>(Object::engineObject(hull, tex, 0, false, true)));
	watermark->depth = -1;

	//quit button
	SDL_Texture* quitTex = Text::loadText("Quit", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(quitTex, &w, &h);
	SDL_FRect quitHull = { Renderer::baseRes.x - w, 0, w, h };
	std::shared_ptr<Object::buttonObject> quitButton = std::make_shared<Object::buttonObject>(Object::buttonObject(quitHull, quitTex, 0, false, true));
	quitButton->onClick = quitProgram;
	std::shared_ptr<Object::engineObject> quitObject = Scene::addObject(quitButton);
	quitObject->depth = -1;

	//controls
	SDL_Texture* conTex = Text::loadText("Left Click - Spawn Object   Right Click - Delete Object   1 - Vsync Toggle   2 - FPS Toggle   3 - Debug Level   4 - Pause Updates", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(conTex, &w, &h);
	SDL_FRect conHull = { 0, Renderer::baseRes.y - h, w, h };
	std::shared_ptr<Object::engineObject> conObj = std::make_shared<Object::engineObject>(Object::engineObject(conHull, conTex, 0, false, true));
	conObj->depth = -1;
	Scene::addObject(conObj);

	//controls 2
	SDL_Texture* conBTex = Text::loadText("WASD - Steer Objects   -/+ - Adjust Object Size   Q/E - Spin   Space - Speed Boost   P - Print Object Count   F - Toggle Follow", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(conBTex, &w, &h);
	SDL_FRect conBHull = { 0, Renderer::baseRes.y - h - 40, w, h };
	std::shared_ptr<Object::engineObject> conBObj = std::make_shared<Object::engineObject>(Object::engineObject(conBHull, conBTex, 0, false, true));
	conBObj->depth = -1;
	Scene::addObject(conBObj);

	//controls 3
	SDL_Texture* conCTex = Text::loadText("Arrow Keys - Move Camera   Mouse Wheel - Zoom Camera   ,/. - Cycle Texture", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(conCTex, &w, &h);
	SDL_FRect conCHull = { 0, Renderer::baseRes.y - h - 80, w, h };
	std::shared_ptr<Object::engineObject> conCObj = std::make_shared<Object::engineObject>(Object::engineObject(conCHull, conCTex, 0, false, true));
	conCObj->depth = -1;
	Scene::addObject(conCObj);

	//TEMP
	//quit button
	SDL_Texture* quitTestTex = Text::loadText("Quit", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(quitTestTex, &w, &h);
	SDL_FRect quitTestHull = { Renderer::baseRes.x - w, 0, w, h };
	std::shared_ptr<Object::buttonObject> quitTestButton = std::make_shared<Object::buttonObject>(Object::buttonObject(quitTestHull, quitTestTex, 0, false, false));
	quitTestButton->onClick = quitProgram;
	std::shared_ptr<Object::engineObject> quitTestObject = Scene::addObject(quitTestButton);
	quitTestObject->depth = -10;
}
//EXAMPLE CODE END


int main(int argc, char* argv[])
{
	//Initialize Engine 
	Engine::initEngine();

	//Initialize modules
	exampleInit();

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