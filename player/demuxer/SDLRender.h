#ifndef SDL_RENDER_H
#define SDL_RENDER_H
#include <SDL2/SDL.h>
class SDL_Render
{
public:
    SDL_Render(){}
    virtual ~SDL_Render(){
        SDL_Quit();//在所有成员对象析构后释放资源
    }
};
#endif