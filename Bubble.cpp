#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <fstream>
#include "Bubble.hpp"
#include "FileReader.hpp"

Bubble::Bubble()
{
    FileReader fr;
    fr.read("settings//font.txt");
    m_font = TTF_OpenFont("font.ttf", fr.getInt("size"));
    m_color = {fr.getInt("red"), fr.getInt("green"), fr.getInt("blue")};
    m_spacing = fr.getInt("spacing");
}

Bubble::~Bubble()
{
    TTF_CloseFont(m_font);
}

void Bubble::render(SDL_Renderer *renderer, SDL_Point const& pos, Animal *target, int camera_zoom)
{
    std::string text;
    int health = target->get_health();
    if(health > 0)
        text = std::to_string(health) + " PV";
    else if(target->is_male())
        text = "mort";
    else
        text = "morte";
    render_text(renderer, pos, text, 0, camera_zoom);

    if(target->is_male())
        text = "male";
    else
        text = "femelle";
    render_text(renderer, pos, text, 1, camera_zoom);
}

void Bubble::render_text(SDL_Renderer *renderer, SDL_Point const& pos, std::string text, int line, int camera_zoom)
{
    SDL_Surface *surface = TTF_RenderText_Blended(m_font, text.c_str(), m_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {pos.x, pos.y - surface->h/2 - line * m_spacing * std::max(0.5, (camera_zoom/64.0)), surface->w * std::max(0.5, (camera_zoom/64.0)), surface->h * std::max(0.5, (camera_zoom/64.0))};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
