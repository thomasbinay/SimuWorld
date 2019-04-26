#include <SDL2/SDL.h>
#include "Camera.hpp"

Camera::Camera()
{
    m_pos = {0, 0};
    m_zoom = 32;
    m_focus = NULL;
}

Camera::~Camera()
{}

void Camera::move(int x, int y)
{
    m_pos.x -= x;
    m_pos.y -= y;

    m_focus = NULL;
}

void Camera::zoom(SDL_Point const& mouse_pos, bool in)
{
    if(in)
    {
        if(m_zoom < 64) //zoom max
        {
            m_zoom += 2;
            //centers the camera on the mouse
            m_pos.x += (m_pos.x + mouse_pos.x)/(m_zoom-2)*2;
            m_pos.y += (m_pos.y + mouse_pos.y)/(m_zoom-2)*2;
        }
    }
    else
    {
        if(m_zoom > 6) //zoom min
        {
            m_zoom -= 2;
            //centers the camera on the mouse
            m_pos.x -= (m_pos.x + mouse_pos.x)/(m_zoom+2)*2;
            m_pos.y -= (m_pos.y + mouse_pos.y)/(m_zoom+2)*2;
        }
    }
}

SDL_Point Camera::get_position() const
{
    return m_pos;
}

int Camera::get_zoom() const
{
    return m_zoom;
}

void Camera::update(Population const& population, SDL_Point const& winsize) //keeps the camera following the animal focused (if there is one)
{
    if(population.is_inside(m_focus)) //the animal still exists
    {
        m_pos.x = (m_focus->get_position().x * m_zoom) - winsize.x/2;
        m_pos.y = (m_focus->get_position().y * m_zoom) - winsize.y/2;
    }
}

void Camera::focus_on_animal(Population const& population, Stats const& stats, SDL_Point const& mouse_pos)
{
    if(mouse_pos.x < 100) //search an animal of the specified specie (if there is one)
    {
        int specie = stats.get_mouse_focus(mouse_pos);
        if(specie > 0) //the mouse is focused on a specie
            m_focus = population.get_animal(specie);
    }
    else //focus on the animal targeted by the mouse (if there is one)
    {
        Vector2d pos = {1.0*(mouse_pos.x + m_pos.x)/m_zoom, 1.0*(mouse_pos.y + m_pos.y)/m_zoom};
        m_focus = population.get_animal(pos);
    }
}
