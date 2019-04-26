#ifndef DEF_CAMERA
#define DEF_CAMERA

#include <SDL2/SDL.h>
#include "Population.hpp"
#include "Animal.hpp"
#include "Stats.hpp"

class Camera
{
public:
    Camera();
    ~Camera();

    void set_focus(Animal *focus);

    void focus_on_animal(class Population const& population, class Stats const& stats, SDL_Point const& mouse_pos);

    int get_zoom() const;
    SDL_Point get_position() const;

    void move(int x, int y);
    void zoom(SDL_Point const& winsize, bool in);
    void update(class Population const& population, SDL_Point const& winsize);

private:
    SDL_Point m_pos;
    int m_zoom;
    Animal *m_focus;
};

#endif
