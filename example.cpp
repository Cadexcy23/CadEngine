#include "engine.h"
#include "example.h"
#include <string>

//EXAMPLE CODE
std::vector<std::shared_ptr<Example::velObject>> objs;
bool follow = false;


//ENGINE OBJECTS FUNCS
void drawDebug(std::shared_ptr<Engine::engineObject> obj) //add this to the base engine
{
	if (Engine::debugLevel >= 2)
	{

		float w, h;
		std::string objS = "OBJ Count: " + std::to_string(objs.size());
		SDL_Texture* obj = Engine::loadText(objS.c_str(), Engine::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(obj, &w, &h);
		Engine::drawTex(obj, { 0, h * 1, w, h }, 0, false);


		std::string camS = "CamX: " + std::to_string(Engine::camPos.x) + "Y: " + std::to_string(Engine::camPos.y);
		SDL_Texture* cam = Engine::loadText(camS.c_str(), Engine::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(cam, &w, &h);
		Engine::drawTex(cam, { 0, h * 2, w, h }, 0, false);

		std::string zoomS = "Zoom: " + std::to_string(Engine::zoom);
		SDL_Texture* zoom = Engine::loadText(zoomS.c_str(), Engine::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(zoom, &w, &h);
		Engine::drawTex(zoom, { 0, h * 3, w, h }, 0, false);

		std::string rouseS = "Rouse X: " + std::to_string(Engine::rawMousePos.x) + "Y: " + std::to_string(Engine::mousePos.y);;
		SDL_Texture* rouse = Engine::loadText(rouseS.c_str(), Engine::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(rouse, &w, &h);
		Engine::drawTex(rouse, { 0, h * 4, w, h }, 0, false);

		std::string mouseS = "Mouse X: " + std::to_string(Engine::mousePos.x) + "Y: " + std::to_string(Engine::mousePos.y);;
		SDL_Texture* mouse = Engine::loadText(mouseS.c_str(), Engine::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(mouse, &w, &h);
		Engine::drawTex(mouse, { 0, h * 5, w, h }, 0, false);

		std::string screenBoundsS = "Screen Bounds: L: " + std::to_string(Engine::screenBounds.x) + " R: " + std::to_string(Engine::screenBounds.w)
			+ " U: " + std::to_string(Engine::screenBounds.y) + " D: " + std::to_string(Engine::screenBounds.h);
		SDL_Texture* screenBounds = Engine::loadText(screenBoundsS.c_str(), Engine::loadFont("resource/font/segoeuithibd.ttf", 32), { 255, 255, 255, 255 });
		SDL_GetTextureSize(screenBounds, &w, &h);
		Engine::drawTex(screenBounds, { 0, h * 6, w, h }, 0, false);
	}
}

void quitProgram()
{
	Engine::quit = true;
}

void tempUpdateFunc(std::shared_ptr<Engine::engineObject> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get downcast object
		auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);

		obj->hull.x += devObj->vel.x * Engine::deltaSeconds;
		obj->hull.y += devObj->vel.y * Engine::deltaSeconds;
		devObj->vel.x -= (devObj->vel.x * Engine::deltaSeconds) / 2;
		devObj->vel.y -= (devObj->vel.y * Engine::deltaSeconds) / 2;

		obj->rot += devObj->spin * Engine::deltaSeconds;
		devObj->spin -= (devObj->spin * Engine::deltaSeconds) / 2;
	}
}

void keepInScreen(std::shared_ptr<Engine::engineObject> obj)
{
	//get downcast object
	auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);

	int left = 0 + (obj->hull.w / 2) * obj->scale;
	int right = Engine::baseRes.x - (obj->hull.w / 2) * obj->scale;
	int up = 0 + (obj->hull.h / 2) * obj->scale;
	int down = Engine::baseRes.y - (obj->hull.h / 2) * obj->scale;
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

void keyboardControl(std::shared_ptr<Engine::engineObject> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get downcast object
		auto devObj = std::dynamic_pointer_cast<Example::velObject>(obj);
		if (Engine::keyStates[SDL_SCANCODE_COMMA] == 1)
		{
			obj->texIndex--;
			if (obj->texIndex < 0)
				obj->texIndex = obj->tex.size() - 1;
		}
		if (Engine::keyStates[SDL_SCANCODE_PERIOD] == 1)
		{
			obj->texIndex++;
			obj->texIndex = obj->texIndex % obj->tex.size();
		}
		if (Engine::keyStates[SDL_SCANCODE_MINUS])
			obj->scale *= 0.95;
		if (Engine::keyStates[SDL_SCANCODE_EQUALS])
			obj->scale *= 1.05;
		if (devObj)
		{
			if (Engine::keyStates[SDL_SCANCODE_W])
				devObj->vel.y -= 100 * Engine::deltaSeconds;
			if (Engine::keyStates[SDL_SCANCODE_S])
				devObj->vel.y += 100 * Engine::deltaSeconds;
			if (Engine::keyStates[SDL_SCANCODE_A])
				devObj->vel.x -= 100 * Engine::deltaSeconds;
			if (Engine::keyStates[SDL_SCANCODE_D])
				devObj->vel.x += 100 * Engine::deltaSeconds;
			if (Engine::keyStates[SDL_SCANCODE_Q])
				devObj->spin -= 360 * Engine::deltaSeconds;
			if (Engine::keyStates[SDL_SCANCODE_E])
				devObj->spin += 360 * Engine::deltaSeconds;
			if (Engine::keyStates[SDL_SCANCODE_SPACE] == 1)
			{
				devObj->vel.x *= 2;
				devObj->vel.y *= 2;
			}
		}
	}
}

void moveToMouse(std::shared_ptr<Engine::engineObject> obj)
{
	obj->hull.x = Engine::mousePos.x;
	obj->hull.y = Engine::mousePos.y;
}

void engineControls()
{

	if (follow && objs.size())
	{
		Engine::camPos.x = objs.front()->hull.x;
		Engine::camPos.y = objs.front()->hull.y;
	}

	if (Engine::wheelStates[0])
	{
		Engine::zoom *= 1.05;
	}
	if (Engine::wheelStates[1])
	{
		Engine::zoom *= .95;
	}
	if (Engine::keyStates[SDL_SCANCODE_UP])
	{
		Engine::camPos.y -= 5.0 / Engine::zoom;
	}
	if (Engine::keyStates[SDL_SCANCODE_DOWN])
	{
		Engine::camPos.y += 5.0 / Engine::zoom;
	}
	if (Engine::keyStates[SDL_SCANCODE_LEFT])
	{
		Engine::camPos.x -= 5.0 / Engine::zoom;
	}
	if (Engine::keyStates[SDL_SCANCODE_RIGHT])
	{
		Engine::camPos.x += 5.0 / Engine::zoom;
	}
	if (Engine::keyStates[SDL_SCANCODE_ESCAPE])
		Engine::quit = true;
	if (Engine::keyStates[SDL_SCANCODE_1] == 1)
	{
		Engine::toggleVsync();
	}
	if (Engine::keyStates[SDL_SCANCODE_2] == 1)
	{
		if (Engine::showFPS)
			Engine::showFPS = false;
		else
			Engine::showFPS = true;
	}
	if (Engine::keyStates[SDL_SCANCODE_3] == 1)
	{
		if (Engine::debugLevel > 1)
			Engine::debugLevel = 0;
		else
			Engine::debugLevel++;
	}
	if (Engine::keyStates[SDL_SCANCODE_4] == 1)
	{
		if (Engine::engineState != Engine::STATE_PAUSE)
			Engine::engineState = Engine::STATE_PAUSE;
		else
			Engine::engineState = Engine::STATE_DEFAULT;
	}
	if (Engine::keyStates[SDL_SCANCODE_P] == 1)
	{
		printf("Object count: %i\n", objs.size());
	}
	if (Engine::keyStates[SDL_SCANCODE_F] == 1)
	{
		follow = !follow;
	}
	if (Engine::mouseStates[0])
	{
		SDL_Texture* tex = Engine::loadTex("resource/test2.png");
		SDL_Texture* tex2 = Engine::loadTex("resource/test.png");
		SDL_Texture* tex3 = Engine::loadTex("resource/icon.png");
		float w, h;
		SDL_GetTextureSize(tex, &w, &h);
		SDL_FRect hull = { Engine::mousePos.x, Engine::mousePos.y, w / 10, h / 10 };
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
		Engine::addObject(velObj);
	}
	if (Engine::mouseStates[2])
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
	Engine::camPos = { float(Engine::baseRes.x / 2), float(Engine::baseRes.y / 2) };

	std::shared_ptr<Engine::engineObject> bg = Engine::addObject(std::make_shared<Engine::engineObject>(Engine::engineObject({ 0, 0, float(Engine::baseRes.x), float(Engine::baseRes.y) }, Engine::loadTex("resource/bg.png"), 0, false)));
	bg->depth = 1;

	//std::shared_ptr<Engine::engineObject> me = Engine::addObject(std::make_shared<Engine::engineObject>(Engine::engineObject({ float(rand() % Engine::baseRes.x), float(rand() % Engine::baseRes.y) , 16, 16 }, Engine::loadTex("resource/icon.png"))));
	//me->depth = -1;
	//me->updateFuncs.push_back(moveToMouse);

	TTF_Font* bold = Engine::loadFont("resource/font/segoeuithibd.ttf", 32);
	Engine::loadFont("resource/font/segoeuithisi.ttf", 32);

	SDL_Texture* tex = Engine::loadText("CadEngine", bold, { 255, 255, 255, 255 });
	float w, h = 0;
	SDL_GetTextureSize(tex, &w, &h);
	SDL_FRect hull = { 0, 0, w, h };
	std::shared_ptr<Engine::engineObject> watermark = Engine::addObject(std::make_shared<Engine::engineObject>(Engine::engineObject(hull, tex, 0, false, true)));
	watermark->depth = -1;
	watermark->drawFuncs.push_back(drawDebug);

	//quit button
	SDL_Texture* quitTex = Engine::loadText("Quit", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(quitTex, &w, &h);
	SDL_FRect quitHull = { Engine::baseRes.x - w, 0, w, h };
	std::shared_ptr<Engine::buttonObject> quitButton = std::make_shared<Engine::buttonObject>(Engine::buttonObject(quitHull, quitTex, 0, false, true));
	quitButton->onClick = quitProgram;
	std::shared_ptr<Engine::engineObject> quitObject = Engine::addObject(quitButton);
	quitObject->depth = -1;

	//controls
	SDL_Texture* conTex = Engine::loadText("Left Click - Spawn Object   Right Click - Delete Object   1 - Vsync Toggle   2 - FPS Toggle   3 - Debug Level   4 - Pause Updates", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(conTex, &w, &h);
	SDL_FRect conHull = { 0, Engine::baseRes.y - h, w, h };
	std::shared_ptr<Engine::engineObject> conObj = std::make_shared<Engine::engineObject>(Engine::engineObject(conHull, conTex, 0, false, true));
	conObj->depth = -1;
	Engine::addObject(conObj);

	//controls 2
	SDL_Texture* conBTex = Engine::loadText("WASD - Steer Objects   -/+ - Adjust Object Size   Q/E - Spin   Space - Speed Boost   P - Print Object Count   F - Toggle Follow", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(conBTex, &w, &h);
	SDL_FRect conBHull = { 0, Engine::baseRes.y - h - 40, w, h };
	std::shared_ptr<Engine::engineObject> conBObj = std::make_shared<Engine::engineObject>(Engine::engineObject(conBHull, conBTex, 0, false, true));
	conBObj->depth = -1;
	Engine::addObject(conBObj);

	//controls 3
	SDL_Texture* conCTex = Engine::loadText("Arrow Keys - Move Camera   Mouse Wheel - Zoom Camera   ,/. - Cycle Texture", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(conCTex, &w, &h);
	SDL_FRect conCHull = { 0, Engine::baseRes.y - h - 80, w, h };
	std::shared_ptr<Engine::engineObject> conCObj = std::make_shared<Engine::engineObject>(Engine::engineObject(conCHull, conCTex, 0, false, true));
	conCObj->depth = -1;
	Engine::addObject(conCObj);

	//TEMP
	//quit button
	SDL_Texture* quitTestTex = Engine::loadText("Quit", bold, { 255, 255, 255, 255 });
	SDL_GetTextureSize(quitTestTex, &w, &h);
	SDL_FRect quitTestHull = { Engine::baseRes.x - w, 0, w, h };
	std::shared_ptr<Engine::buttonObject> quitTestButton = std::make_shared<Engine::buttonObject>(Engine::buttonObject(quitTestHull, quitTestTex, 0, false, false));
	quitTestButton->onClick = quitProgram;
	std::shared_ptr<Engine::engineObject> quitTestObject = Engine::addObject(quitTestButton);
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
		Engine::controller();

		//Update modules here
		engineControls();

		//Render scene
		Engine::draw();
	}

	return 0;
}