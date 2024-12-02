//add includes

//int main
// start SDL and make window
//		load other files and such
//		like controller or w/e
// 
//		main loop for running shit
//			read input
//			update modlues
//			render everything
//

#include "engine.h"


int main(int argc, char* argv[])
{
    //Initialize Engine 
    Engine::initEngine();

    while (!Engine::quit) {

        
        SDL_Event event;
        SDL_PollEvent(&event);
        switch (event.type) {
        case SDL_EVENT_QUIT:
            Engine::quit = true;
            break;
        }

    }

    
    return 0;
}