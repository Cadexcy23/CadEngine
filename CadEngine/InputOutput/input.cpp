#include "input.h"
#include "../Core/engine.h"


const bool* SDLKeyStates;
std::vector<bool> mouseWheel;
SDL_Event event;
SDL_FPoint Input::mousePos = { 0, 0 };
SDL_FPoint Input::rawMousePos = { 0, 0 };
SDL_FPoint Input::mousePosDif = { 0, 0 };
std::vector<int> Input::mouseStates;
std::vector<int> Input::keyStates;
std::vector<int> Input::wheelStates;


void readMouse()
{
	//get mouse button state and position
	Input::mousePosDif = { Input::rawMousePos.x, Input::rawMousePos.y };
	Uint32 mouseState = SDL_GetMouseState(&Input::rawMousePos.x, &Input::rawMousePos.y);
	Input::rawMousePos.x *= Renderer::resScale.x;
	Input::rawMousePos.y *= Renderer::resScale.y;
	Input::mousePosDif = { Input::mousePosDif.x - Input::rawMousePos.x, Input::mousePosDif.y - Input::rawMousePos.y };
	Input::mousePos.x = Input::rawMousePos.x;
	Input::mousePos.y = Input::rawMousePos.y;
	//adjust mouse pos in relation to the camera pos and zoom
	Input::mousePos.x -= float(Renderer::baseRes.x / 2);
	Input::mousePos.x /= Renderer::zoom;
	Input::mousePos.x += Renderer::camPos.x;
	Input::mousePos.y -= float(Renderer::baseRes.y / 2);
	Input::mousePos.y /= Renderer::zoom;
	Input::mousePos.y += Renderer::camPos.y;

	//loop through each button on the mouse
	for (int i = 0; i < Input::mouseStates.size(); i++)
	{
		//if the button is pressed
		if (mouseState & SDL_BUTTON_MASK(i + 1))
		{
			//if it wasnt held down set it to just pressed
			if (Input::mouseStates[i] == 0 || Input::mouseStates[i] == 3)
				Input::mouseStates[i] = 1;
			//if it was already pressed mark as held down
			else if (Input::mouseStates[i] == 1 || Input::mouseStates[i] == 2)
				Input::mouseStates[i] = 2;
		}
		//if not and the button state was non zero
		else if (Input::mouseStates[i])
		{
			//if it was held down mark as just released
			if (Input::mouseStates[i] == 1 || Input::mouseStates[i] == 2)
				Input::mouseStates[i] = 3;
			//if already released mark as clear
			else
				Input::mouseStates[i] = 0;
		}
	}

	//loop through each direction of the mouse wheel
	for (int i = 0; i < mouseWheel.size(); i++)
	{
		//if the key is pressed
		if (mouseWheel[i])
		{
			//if it wasnt held down set it to just pressed
			if (Input::wheelStates[i] == 0 || Input::wheelStates[i] == 3)
				Input::wheelStates[i] = 1;
			//if it was already pressed mark as held down
			else if (Input::wheelStates[i] == 1 || Input::wheelStates[i] == 2)
				Input::wheelStates[i] = 2;
		}
		//if not and the key state was non zero
		else if (Input::wheelStates[i])
		{
			//if it was held down mark as just released
			if (Input::wheelStates[i] == 1 || Input::wheelStates[i] == 2)
				Input::wheelStates[i] = 3;
			//if already released mark as clear
			else
				Input::wheelStates[i] = 0;
		}
	}
	//reset mouseWheel
	std::fill(mouseWheel.begin(), mouseWheel.end(), false);


	//draws mouse buttons and their states for debugging
	//printf("Pos: %f, %f ", Input::mousePos.x, Input::mousePos.y);
	//for (int i = 0; i < Input::mouseStates.size(); i++)
	//{
	//	if (Input::mouseStates[i] != 0)
	//	{
	//		printf("Button %i: %i\n", i, Input::mouseStates[i]);
	//	}
	//}

	//draws mouse wheel and their states for debugging
	//for (int i = 0; i < Input::wheelStates.size(); i++)
	//{
	//	if (Input::wheelStates[i] != 0)
	//	{
	//		printf("Direction %i: %i\n", i, Input::wheelStates[i]);
	//	}
	//}
}

static void readKeyboard()
{
	//loop through each key on the keyboard
	for (int i = 0; i < Input::keyStates.size(); i++)
	{
		//if the key is pressed
		if (SDLKeyStates[i])
		{
			//if it wasnt held down set it to just pressed
			if (Input::keyStates[i] == 0 || Input::keyStates[i] == 3)
				Input::keyStates[i] = 1;
			//if it was already pressed mark as held down
			else if (Input::keyStates[i] == 1 || Input::keyStates[i] == 2)
				Input::keyStates[i] = 2;
		}
		//if not and the key state was non zero
		else if (Input::keyStates[i])
		{
			//if it was held down mark as just released
			if (Input::keyStates[i] == 1 || Input::keyStates[i] == 2)
				Input::keyStates[i] = 3;
			//if already released mark as clear
			else
				Input::keyStates[i] = 0;
		}
	}

	//draws keys and their states for debugging
	//for (int i = 0; i < Input::keyStates.size(); i++)
	//{
	//	if (Input::keyStates[i] != 0)
	//	{
	//		printf("%i: %i \n", i, Input::keyStates[i]);
	//	}
	//}
}

bool Input::initController()
{
	//get a handle on the SDL key states array
	SDLKeyStates = SDL_GetKeyboardState(NULL);
	//resize our vector to be the same amount
	Input::keyStates.resize(SDL_SCANCODE_COUNT);
	//set each key to 0
	std::fill(Input::keyStates.begin(), Input::keyStates.end(), 0);

	//resize our mouseStates vector to hold all out mouse buttons
	Input::mouseStates.resize(5);
	//set each key to 0
	std::fill(Input::mouseStates.begin(), Input::mouseStates.end(), 0);

	//resize/set default mouseWheel/wheelStates for each direction
	mouseWheel.resize(4);
	std::fill(mouseWheel.begin(), mouseWheel.end(), false);
	Input::wheelStates.resize(4);
	std::fill(Input::wheelStates.begin(), Input::wheelStates.end(), 0);


	return true;
}

void Input::controller()
{
	while (SDL_PollEvent(&event)) {
		//printf("input detected: %i\n", event.type);
		switch (event.type) {
		case SDL_EVENT_QUIT:
			Engine::quit = true;
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			if (event.wheel.y > 0)
				mouseWheel[0] = true;
			else
				mouseWheel[1] = true;
			break;
		}
	}

	readMouse();
	readKeyboard();
}
