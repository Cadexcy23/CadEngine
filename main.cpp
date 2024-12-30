#include "engine.h"

int main(int argc, char* argv[])
{
	//Initialize Engine 
	Engine::initEngine();

	//Initialize modules
	

	while (!Engine::quit) {
		//Process input
		Engine::controller();

		//Update modules here
		

		//Render scene
		Engine::draw();
	}

	return 0;
}