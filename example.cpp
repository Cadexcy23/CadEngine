#include "engine.h"

//EXAMPLE CODE
#include "example.h"
void engineControls()
{
	//TEMP ZONE
	if (Engine::keyStates[SDL_SCANCODE_ESCAPE])
		Engine::quit = true;
	if (Engine::keyStates[SDL_SCANCODE_V] == 1)
	{
		Engine::toggleVsync();
	}
	if (Engine::keyStates[SDL_SCANCODE_P] == 1)
	{
		if (Engine::engineState != Engine::STATE_PAUSE)
			Engine::engineState = Engine::STATE_PAUSE;
		else
			Engine::engineState = Engine::STATE_DEFAULT;
	}
}
//ENGINE OBJECTS FUNCS
void tempUpdateFunc(std::shared_ptr<Engine::engineObject> obj)
{
	if (Engine::engineState != Engine::STATE_PAUSE)
	{
		//get the thing blah blah
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
		if (Engine::wheelStates[0])
			obj->scale *= 1.05;
		if (Engine::wheelStates[1])
			obj->scale *= 0.95;
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

void exampleInit()
{
	//TEMP ZONE
	std::shared_ptr<Engine::engineObject> bg = Engine::registerObject(std::make_shared<Engine::engineObject>(Engine::engineObject({ 0, 0, 1920, 1080 }, Engine::loadTex("resource/bg.png"), 0, false)));
	bg->depth = 1;

	std::shared_ptr<Engine::engineObject> me = Engine::registerObject(std::make_shared<Engine::engineObject>(Engine::engineObject({ float(rand() % Engine::baseRes.x), float(rand() % Engine::baseRes.y) , 200, 200 }, Engine::loadTex("resource/icon.png"))));
	me->depth = -1;
	me->updateFuncs.push_back(keepInScreen);
	me->updateFuncs.push_back(keyboardControl);

	for (int i = 0; i < 1000; i++)
	{
		SDL_FRect hull = { float(rand() % Engine::baseRes.x), float(rand() % Engine::baseRes.y), 100, 100 };
		SDL_Texture* tex = Engine::loadTex("resource/test2.png");
		SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
		double rot = rand() % 360;
		float scale = float(rand() % 1000) / 1000.0 + 0.5;
		double spin = double(rand() % 20000) / 1000.0 - 10.0;

		std::shared_ptr<Example::velObject> velObj = std::make_shared<Example::velObject>(
			hull, tex, rot, true, SDL_FLIP_NONE, scale, vel, spin);
		velObj->updateFuncs.push_back(keepInScreen);
		velObj->updateFuncs.push_back(tempUpdateFunc);
		velObj->updateFuncs.push_back(keyboardControl);
		Engine::registerObject(velObj);

	}

	TTF_Font* bold = Engine::loadFont("resource/font/segoeuithibd.ttf", 128);
	Engine::loadFont("resource/font/segoeuithisi.ttf", 128);

	SDL_Texture* tex = Engine::loadText("CadEngine", bold, { 255, 255, 255, 255 });
	float w, h = 0;
	SDL_GetTextureSize(tex, &w, &h);
	SDL_FRect hull = { 0, Engine::baseRes.y - h * 0.25, w, h };
	SDL_FPoint vel = { rand() % 1000 - 500, rand() % 1000 - 500 };
	std::shared_ptr<Engine::engineObject> watermark = Engine::registerObject(std::make_shared<Engine::engineObject>(Engine::engineObject(hull, tex)));
	watermark->scale = 0.25;
	watermark->centered = false;
	watermark->depth = -1;

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