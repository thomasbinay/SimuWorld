#ifndef DEF_BLOOD
#define DEF_BLOOD

#include <SDL2/SDL.h>
#include <vector>
#include "Camera.hpp"

struct Stain
{
    Vector2d pos;
    int t;
};

class Blood
{
public:
    Blood(SDL_Renderer *renderer);
    ~Blood();

    bool get_error() const;

    void update();
    void render(SDL_Renderer *renderer, SDL_Point const& winsize, class Camera const& camera);

    void add_stain(Vector2d const& pos);

private:
    std::vector <Stain*> m_blood;
    SDL_Texture *m_texture;

    int m_disappearance_time;
};

#endif
