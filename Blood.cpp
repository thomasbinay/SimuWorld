#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <fstream>
#include "Blood.hpp"
#include "Camera.hpp"
#include "FileReader.hpp"

Blood::Blood(SDL_Renderer *renderer)
{
    m_texture = IMG_LoadTexture(renderer, "map//blood.png");

    FileReader fr;
    fr.read("settings//blood.txt");
    m_disappearance_time = fr.getInt("disappearance_time");
}

Blood::~Blood()
{
    SDL_DestroyTexture(m_texture);
}

bool Blood::get_error() const
{
    if(!m_texture)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erreur de texture", "map//blood.png n'a pas pu etre ouvert", NULL);
        return true;
    }
    return false;
}

void Blood::update()
{
    for(int i = 0; i < m_blood.size(); i++)
        if(SDL_GetTicks() - m_blood[i]->t >= m_disappearance_time)
            m_blood.erase(m_blood.begin() + i);
}

void Blood::render(SDL_Renderer *renderer, SDL_Point const& winsize, Camera const& camera)
{
    SDL_Point camera_pos = camera.get_position();
    int camera_zoom = camera.get_zoom();

    SDL_Rect position = {0, 0, camera_zoom, camera_zoom};
    for(int i = 0; i < m_blood.size(); i++)
    {
        SDL_SetTextureAlphaMod(m_texture, 1.0 * (m_disappearance_time - (SDL_GetTicks() - m_blood[i]->t)) / m_disappearance_time * 255); //smooth disappearance
        position.x = m_blood[i]->pos.x * camera_zoom - camera_pos.x - camera_zoom/2;
        position.y = m_blood[i]->pos.y * camera_zoom - camera_pos.y - camera_zoom/2;
        if(position.x + camera_zoom > 0 && position.x < winsize.x && position.y + camera_zoom > 0 && position.y < winsize.y)
            SDL_RenderCopy(renderer, m_texture, NULL, &position);
    }
}

void Blood::add_stain(Vector2d const& pos)
{
    m_blood.push_back(new Stain);
    m_blood[m_blood.size()-1]->pos = pos;
    m_blood[m_blood.size()-1]->t = SDL_GetTicks();
}
