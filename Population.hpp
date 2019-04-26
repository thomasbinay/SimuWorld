#ifndef DEF_POPULATION
#define DEF_POPULATION

#include <SDL2/SDL.h>
#include <array>
#include "Animal.hpp"
#include "Map.hpp"
#include "Bubble.hpp"
#include "Blood.hpp"

class Population
{
public:
    Population(SDL_Renderer *renderer);
    ~Population();

    void generate();

    bool get_error() const;
    int get_number(int specie) const;
    Animal* get_animal(int specie) const;
    Animal* get_animal(Vector2d const& pos) const;

    bool is_inside(Animal *target) const;

    void show_hide_bubble();
    void show_hide_blood();

    void render(SDL_Renderer *renderer, SDL_Point const& winsize, class Camera const& camera);
    void update(class Map &map);

private:
    void AI_main(class Map &map);
    void AI_check_current_location(int index, class Map &map);
    void AI_check_territory(int index);

    bool AI_find_plant(int index, class Map &map);
    bool AI_find_prey(int index);
    bool AI_find_partner(int index);

    Vector2d AI_midway(Vector2d const& pos, Vector2d const& dest);
    void AI_simulate_search(int index);

    int get_nutritional_value(Animal const& target) const;
    int get_nutritional_value(int plant) const;


    std::vector <Animal*> m_animals;

    std::vector < std::vector <std::vector <Animal*> > > m_animal_map;
    SDL_Point m_mapsize;

    std::vector <SDL_Texture*> m_texture;

    std::vector <int> m_nutritional_value;

    class Bubble *m_bubble;
    bool m_show_bubble;

    class Blood *m_blood;
    bool m_show_blood;
};

#endif
