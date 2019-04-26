#ifndef DEF_MAP
#define DEF_MAP

#include <SDL2/SDL.h>
#include <vector>
#include "Camera.hpp"

class Map
{
public:
    Map(SDL_Renderer *renderer);
    ~Map();

    bool get_error() const;
    SDL_Point get_size() const;
    int get_number(int resource) const;

    void generate();

    void render(SDL_Renderer *renderer, SDL_Point const& winsize, class Camera const& camera);
    void update();

    int get_resource(int x, int y) const;
    int get_biome(int x, int y) const;
    void set_resource(int x, int y, int resource);
    void set_biome(int x, int y, int biome);
    void remove_resource(int x, int y);

    bool is_free(int x, int y) const;

private:
    bool resource_compatible_with_biome(int resource, int biome) const;

    std::vector <SDL_Texture*> m_resources;
    std::vector <SDL_Texture*> m_biomes;

    std::vector < std::vector <int> > m_map;
    SDL_Point m_mapsize;

    int m_last_update;

    std::vector <bool> m_adaptive;
    std::vector <int> m_target;
    std::vector <int> m_add;
    std::vector < std::vector <int> > m_location;
};

#endif
