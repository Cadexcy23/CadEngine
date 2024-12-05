#include "engine.h"


int main(int argc, char* argv[])
{
    //Initialize Engine 
    Engine::initEngine();

    while (!Engine::quit) {
        //Process input
        Engine::controller();
        
        //Add modules here

        //Render scene
        Engine::draw();
    }

    return 0;
}