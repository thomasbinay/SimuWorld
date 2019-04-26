#ifndef DEF_BUBBLE
#define DEF_BUBBLE

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include "Animal.hpp"

class Bubble
{
public:
    Bubble();
    ~Bubble();

    void render(SDL_Renderer *renderer, SDL_Point const& pos, Animal *target, int camera_zoom);

private:
    void render_text(SDL_Renderer *renderer, SDL_Point const& pos, std::string text, int line, int camera_zoom);

    TTF_Font *m_font;
    SDL_Color m_color;
    int m_spacing;
};

#endif
