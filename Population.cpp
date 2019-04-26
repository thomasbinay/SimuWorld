#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <algorithm>
#include <cmath>
#include "Population.hpp"
#include "speed.hpp"
#include "FileReader.hpp"

Population::Population(SDL_Renderer *renderer)
{
    FileReader fr;
    fr.read("settings//species_init.txt");
    int species_number = fr.getCount();
    m_texture.resize(species_number, NULL);

    std::string name;
    for(int i = 0; i < species_number; i++)
    {
        name = "animals//specie_" + std::to_string(i + 1) + ".png";
        m_texture[i] = IMG_LoadTexture(renderer, name.c_str());
    }

    fr.read("settings//nutritional_value.txt");
    int all_number = fr.getCount();
    m_nutritional_value.resize(all_number);

    for(int i = 0; i < all_number; i++)
    {
        if(i < species_number)
            name = "specie_" + std::to_string(i + 1);
        else
            name = "resource_" + std::to_string(i + 1 - species_number);
        m_nutritional_value[i] = fr.getInt(name);
    }

    m_bubble = new Bubble();
    m_show_bubble = true;

    m_blood = new Blood(renderer);
    m_show_blood = true;
}

Population::~Population()
{
    for(int i = 0; i < m_texture.size(); i++)
        SDL_DestroyTexture(m_texture[i]);

    for(int i = 0; i < m_animals.size(); i++)
        delete m_animals[i];

    delete m_blood;
}

bool Population::get_error() const
{
    for(int i = 0; i < m_texture.size(); i++)
    {
        if(!m_texture[i])
        {
            std::string error = "animals//specie_" + std::to_string(i + 1) + ".png n'a pas pu etre ouvert";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erreur de texture", error.c_str(), NULL);
            return true;
        }
    }

    if(m_blood->get_error())
        return true;

    return false;
}

void Population::generate()
{
    for(int i = 0; i < m_animals.size(); i++)
        delete m_animals[i];
    m_animals.clear();


    FileReader fr;
    fr.read("settings//map.txt");

    m_mapsize.x = fr.getInt("width");
    m_mapsize.y = fr.getInt("height");
    m_animal_map.resize(m_mapsize.x, std::vector < std::vector <Animal*> >(m_mapsize.y));

    fr.read("settings//species_init.txt");
    std::vector <int> to_add;
    for(int i = 0; i < m_texture.size(); i++)
        to_add.push_back(fr.getInt("specie_" + std::to_string(i + 1)));

    for(int specie = 0; specie < to_add.size(); specie++)
    {
        for(int added = 0; added < to_add[specie]; added++)
        {
            Vector2d pos = {(rand() % m_mapsize.x) + 0.5, (rand() % m_mapsize.y) + 0.5};
            m_animals.push_back(new Animal(pos, specie + 1));
        }
    }
}

void Population::render(SDL_Renderer *renderer, SDL_Point const& winsize, Camera const& camera)
{
    int camera_zoom = camera.get_zoom();
    SDL_Point camera_pos = camera.get_position();

    if(m_show_blood)
        m_blood->render(renderer, winsize, camera);

    SDL_Rect position = {0, 0, camera_zoom, camera_zoom};
    SDL_Rect portion = {0, 0, 48, 48};
    for(int i = 0; i < m_animals.size(); i++)
    {
        position.x = m_animals[i]->get_position().x * camera_zoom - camera_pos.x - camera_zoom/2;
        position.y = m_animals[i]->get_position().y * camera_zoom - camera_pos.y - camera_zoom/2;
        if(position.x + camera_zoom > 0 && position.x < winsize.x && position.y + camera_zoom > 0 && position.y < winsize.y)
        {
            portion.x = (m_animals[i]->get_color() % 4) * 144 + m_animals[i]->get_animation() * 48;
            portion.y = m_animals[i]->get_direction() * 48 + (m_animals[i]->get_color() / 4) * 192;
            SDL_RenderCopy(renderer, m_texture[m_animals[i]->get_specie() - 1], &portion, &position);
        }
    }

    if(m_show_bubble)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Point pos;
        for(int n = 0; n < m_animals.size(); n++)
        {
            pos.x = m_animals[n]->get_position().x * camera_zoom - camera_pos.x - camera_zoom/2;
            pos.y = m_animals[n]->get_position().y * camera_zoom - camera_pos.y - camera_zoom/2;
            if(pos.x + camera_zoom > 0 && pos.x < winsize.x && pos.y + camera_zoom > 0 && pos.y < winsize.y)
                m_bubble->render(renderer, pos, m_animals[n], camera_zoom);
        }
    }
}

void Population::update(Map &map)
{
    //update the map contening all animals according to their current position (to optimize the search and interactions between animals)
    for(int i = 0; i < m_mapsize.x; i++)
        for(int j = 0; j < m_mapsize.y; j++)
            m_animal_map[i][j].clear();


    for(int i = 0; i < m_animals.size(); i++)
        m_animal_map[m_animals[i]->get_position().x][m_animals[i]->get_position().y].push_back(m_animals[i]);

    //AI of the animals
    AI_main(map);

    //the blood disappeares after some time
    m_blood->update();
}

void Population::AI_main(Map &map) //main function of the AI
{
    for(int index = 0; index < m_animals.size(); index++)
    {
        m_animals[index]->update_stats(); //update every sec the stats of the animal

        if(m_animals[index]->is_alive())
        {
            if(m_animals[index]->move()) //moves the animal, if arrived, find a new one
            {
                if(m_animals[index]->is_hungry()) //in priority, if hungry, find food according to its diet
                {
                    switch(m_animals[index]->get_diet())
                    {
                    case HERBIVORE:
                        if(!AI_find_plant(index, map))
                            AI_simulate_search(index);
                        break;
                    case CARNIVORE:
                        if(!AI_find_prey(index))
                            AI_simulate_search(index);
                        break;
                    case OMNIVORE:
                        if(!AI_find_plant(index, map) && !AI_find_prey(index))
                            AI_simulate_search(index);
                        break;
                    }
                }
                else if(m_animals[index]->is_ready_to_reproduce()) //if not hungry and able to reproduce, find a partner
                     if(!AI_find_partner(index))
                        AI_simulate_search(index);

            }

            AI_check_current_location(index, map); //deals with all the interactions of the animals once it has moved
            AI_check_territory(index); //deals with the agressivty of the animal
        }
        else if(m_animals[index]->is_decomposed()) //if the animal is dead for a certain period of time, it disappears of the map
        {
            m_animals.erase(m_animals.begin() + index);
            index -= 1; //necessary not to skip an animal
        }
    }
}

void Population::AI_check_current_location(int index, Map &map) //deals with all the interactions of the animals
{
    Vector2d pos = m_animals[index]->get_position();
    if(m_animals[index]->is_my_plant(map.get_resource(pos.x, pos.y)) && m_animals[index]->is_hungry()) //check if the nearest resource is comestible
    {
        m_animals[index]->regenerate(get_nutritional_value(map.get_resource(pos.x, pos.y)));
        map.remove_resource(pos.x, pos.y);
    }
    Vector2d other_pos;

    std::vector <Animal*> m_neighbors; //list of all neighboors
    for(int i = std::max(0, int(pos.x) - 1); i < std::min(m_mapsize.x, int(pos.x) + 2); i++)
        for(int j = std::max(0, int(pos.y) - 1); j < std::min(m_mapsize.y, int(pos.y) + 2); j++)
            m_neighbors.insert(m_neighbors.end(), m_animal_map[i][j].begin(), m_animal_map[i][j].end());

    for(int n = 0; n < m_neighbors.size(); n++) //go through this list
    {
        if(m_animals[index] != m_neighbors[n]) //not myself 
        {
            other_pos = m_neighbors[n]->get_position();
            if(sqrt(pow(pos.x - other_pos.x, 2) + pow(pos.y - other_pos.y, 2)) <= 0.5) //max distance of interaction
            {
                if(m_animals[index]->is_my_prey(*m_neighbors[n])) //I found a prey
                {
                    if(m_neighbors[n]->is_alive()) //let's attack it
                    {
                        if(m_animals[index]->attack(*m_neighbors[n]))
                            m_blood->add_stain(other_pos);
                    }
                    else if(m_animals[index]->is_hungry()) //let's eat it
                    {
                        m_animals[index]->regenerate(get_nutritional_value(*m_neighbors[n]));
                        m_animals.erase(std::remove(m_animals.begin(), m_animals.end(), m_neighbors[n]), m_animals.end());
                    }
                }
                else if(m_neighbors[n]->is_my_prey(*m_animals[index])) //I'm the prey
                {
                    if(m_animals[index]->attack(*m_neighbors[n])) //let's defend myself
                        m_blood->add_stain(other_pos);
                }
                else if(m_animals[index]->is_my_type(*m_neighbors[n])) //I found a compatible partner
                {
                    m_animals[index]->reproduce(); //let's reproduce
                    m_neighbors[n]->reproduce();
                    m_animals.push_back(new Animal(m_animals[index]->get_position(), m_animals[index]->get_specie(), m_animals[index]->get_color(), m_animals[index], m_neighbors[n]));
                }
                else if(m_animals[index]->is_agressive_with(m_neighbors[n])) //I am agressive with it
                {
                    if(m_animals[index]->attack(*m_neighbors[n])) //let's attack it
                        m_blood->add_stain(other_pos);
                }
            }
        }
    }
}

bool Population::AI_find_plant(int index, Map &map) //set as destination the nearest plant, otherwise, returns false
{
    Vector2d pos = m_animals[index]->get_position();
    Vector2d food_pos;
    bool found = false;
    int range = m_animals[index]->get_plant_range_detection();

    double current_distance = 0, shortest_distance = range;

    for(int x = std::max(0, int(pos.x - range)); x < std::min(m_mapsize.x, int(pos.x + range) + 1); x++)
    {
        for(int y = std::max(0, int(pos.y - range)); y < std::min(m_mapsize.y, int(pos.y + range) + 1); y++)
        {
            if(m_animals[index]->is_my_plant(map.get_resource(x, y)))
            {
                current_distance = sqrt(pow(x + 0.5 - pos.x, 2) + pow(y + 0.5 - pos.y, 2));
                if(current_distance <= shortest_distance)
                {
                    shortest_distance = current_distance;
                    food_pos = {x + 0.5, y + 0.5}; //coordinates of the center of the block
                    found = true;
                }
            }
        }
    }

    if(found)
    {
        m_animals[index]->set_destination(AI_midway(pos, food_pos));
        return true;
    }
    return false;
}

bool Population::AI_find_prey(int index) //set as destination the nearest prey, otherwise, returns false
{
    Vector2d pos = m_animals[index]->get_position();
    Vector2d prey_pos;
    bool found = false;
    double range = m_animals[index]->get_prey_range_detection();

    double current_distance = 0, shortest_distance = range; //makes sure he stays in his range

    std::vector <Animal*> m_neighbors;
        for(int i = std::max(0, int(pos.x - range)); i < std::min(m_mapsize.x, int(pos.x + range) + 1); i++)
            for(int j = std::max(0, int(pos.y - range)); j < std::min(m_mapsize.y, int(pos.y + range) + 1); j++)
                m_neighbors.insert(m_neighbors.end(), m_animal_map[i][j].begin(), m_animal_map[i][j].end());

    Vector2d other_pos;
    for(int n = 0; n < m_neighbors.size(); n++)
    {
        if(m_animals[index] != m_neighbors[n])
        {
            if(m_animals[index]->is_my_prey(*m_neighbors[n]))
            {
                other_pos = m_neighbors[n]->get_position();
                current_distance = sqrt(pow(pos.x - other_pos.x, 2) + pow(pos.y - other_pos.y, 2));
                if(current_distance <= shortest_distance)
                {
                    shortest_distance = current_distance;
                    prey_pos = other_pos;
                    found = true;
                }
            }
        }
    }

    if(found)
    {
        m_animals[index]->set_destination(AI_midway(pos, prey_pos));
        return true;
    }
    return false;
}

bool Population::AI_find_partner(int index) //set as destination the nearest partner, otherwise, returns false
{
    Vector2d pos = m_animals[index]->get_position();
    Vector2d partner_pos;
    bool found = false;
    int range = m_animals[index]->get_partner_range_dection();

    double current_distance = 0, shortest_distance = range; //makes sure he stays in his range

    std::vector <Animal*> m_neighbors;
        for(int i = std::max(0, int(pos.x - range)); i < std::min(m_mapsize.x, int(pos.x + range) + 1); i++)
            for(int j = std::max(0, int(pos.y - range)); j < std::min(m_mapsize.y, int(pos.y + range) + 1); j++)
                m_neighbors.insert(m_neighbors.end(), m_animal_map[i][j].begin(), m_animal_map[i][j].end());

    Vector2d other_pos;
    for(int n = 0; n < m_neighbors.size(); n++)
    {
        if(m_animals[index] != m_neighbors[n])
        {
            if(m_animals[index]->is_my_type(*m_neighbors[n]))
            {
                other_pos = m_neighbors[n]->get_position();
                current_distance = sqrt(pow(pos.x - other_pos.x, 2) + pow(pos.y - other_pos.y, 2));
                if(current_distance <= shortest_distance)
                {
                    shortest_distance = current_distance;
                    partner_pos = other_pos;
                    found = true;
                }
            }
        }
    }

    if(found)
    {
        m_animals[index]->set_destination(AI_midway(pos, partner_pos));
        return true;
    }
    return false;
}

void Population::AI_simulate_search(int index) //set a random location at the borders of its range to simulate the search
{
    Vector2d pos = m_animals[index]->get_position();
    Vector2d dest;
    int range = m_animals[index]->get_search_distance();

    double angle = (rand() % 628)/100.0;

    dest.x = pos.x + range * cos(angle);
    if(dest.x < 0.5)
        dest.x = 0.5;
    else if(dest.x > m_mapsize.x - 0.5)
        dest.x = m_mapsize.x - 0.5;

    dest.y = pos.y + range * sin(angle);
    if(dest.y < 0.5)
        dest.y = 0.5;
    else if(dest.y >= m_mapsize.y - 0.5)
        dest.y = m_mapsize.y - 0.5;

    m_animals[index]->set_destination(dest);
}

Vector2d Population::AI_midway(Vector2d const& pos, Vector2d const& dest) //returns the position between those two
{
    if(sqrt(pow(pos.x - dest.x, 2) + pow(pos.y - dest.y, 2)) <= 0.5) //very close to its destination -> no need to set midway destination
        return dest;
    Vector2d midway = {(pos.x + dest.x)/2, (pos.y + dest.y)/2};
    return midway;
}

void Population::AI_check_territory(int index) //looks if there is an ennemy on his territory (and set him as a destination if there is one)
{
    if(m_animals[index]->is_agressive())
    {
        Vector2d pos = m_animals[index]->get_position();
        Vector2d ennemy_pos;
        bool found = false;
        int range = m_animals[index]->get_agressivity_range();

        double current_distance = 0, shortest_distance = range;

        std::vector <Animal*> m_neighbors;
            for(int i = std::max(0, int(pos.x - range)); i < std::min(m_mapsize.x, int(pos.x + range) + 1); i++)
                for(int j = std::max(0, int(pos.y - range)); j < std::min(m_mapsize.y, int(pos.y + range) + 1); j++)
                    m_neighbors.insert(m_neighbors.end(), m_animal_map[i][j].begin(), m_animal_map[i][j].end());

        Vector2d other_pos;
        for(int n = 0; n < m_neighbors.size(); n++)
        {
            if(m_animals[index] != m_neighbors[n])
            {
                if(m_animals[index]->is_agressive_with(m_neighbors[n]))
                {
                    other_pos = m_neighbors[n]->get_position();
                    current_distance = sqrt(pow(pos.x - other_pos.x, 2) + pow(pos.y - other_pos.y, 2));
                    if(current_distance <= shortest_distance)
                    {
                        shortest_distance = current_distance;
                        ennemy_pos = other_pos;
                        found = true;
                    }
                }
            }
        }

        if(found)
            m_animals[index]->set_destination(AI_midway(pos, ennemy_pos));
    }
}

int Population::get_number(int specie) const
{
    int number = 0;
    for(int i = 0; i < m_animals.size(); i++)
        if(m_animals[i]->is_alive() && m_animals[i]->get_specie() == specie)
            number += 1;
    return number;
}

void Population::show_hide_bubble()
{
    m_show_bubble = !m_show_bubble;
}

bool Population::is_inside(Animal *target) const //make sure if the animal targeted still exits or has been deleted (security check for the camera)
{
    for(int i = 0; i < m_animals.size(); i++)
        if(m_animals[i] == target)
            return true;
    return false;
}

Animal* Population::get_animal(int specie) const //returnes a pointer to the latest animal of the specified specie found or NULL
{
    for(int i = m_animals.size() - 1; i >= 0; i--)
        if(m_animals[i]->get_specie() == specie && m_animals[i]->is_alive())
            return m_animals[i];
    return NULL;
}

Animal* Population::get_animal(Vector2d const& pos) const //returnes a pointer to the first animal located at this position or NULL
{
    for(int i = 0; i < m_animals.size(); i++)
        if(sqrt(pow(m_animals[i]->get_position().x - pos.x, 2) + pow(m_animals[i]->get_position().y - pos.y, 2)) <= 0.5)
            return m_animals[i];
    return NULL;
}

int Population::get_nutritional_value(Animal const& target) const
{
    return m_nutritional_value[target.get_specie() - 1];
}

int Population::get_nutritional_value(int plant) const
{
    return m_nutritional_value[m_texture.size() + plant - 1];
}

void Population::show_hide_blood()
{
    m_show_blood = !m_show_blood;
}
