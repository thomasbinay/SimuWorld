#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <fstream>
#include "Map.hpp"
#include "speed.hpp"
#include "FileReader.hpp"

Map::Map(SDL_Renderer *renderer)
{
    m_mapsize = {0, 0};
    m_last_update = SDL_GetTicks();

    std::string name;
    FileReader fr;
    fr.read("settings//map.txt");
    m_biomes.resize(fr.getCount() - 2, NULL);
    for(int i = 0; i < m_biomes.size(); i++)
    {
        name = "map//biome_" + std::to_string(i + 1) + ".png";
        m_biomes[i] = IMG_LoadTexture(renderer, name.c_str());
    }

    fr.read("settings//resources_init.txt");
    m_resources.resize(fr.getCount(), NULL);
    for(int i = 0; i < m_resources.size(); i++)
    {
        name = "map//resource_" + std::to_string(i + 1) + ".png";
        m_resources[i] = IMG_LoadTexture(renderer, name.c_str());
    }

    for(int i = 0; i < m_resources.size(); i++)
    {
        fr.read("settings//resource_" + std::to_string(i + 1) + ".txt");
        m_location.push_back(fr.getVectorInt("location"));
        m_adaptive.push_back(fr.getInt("adaptive"));
        m_target.push_back(fr.getInt("target"));
        m_add.push_back(fr.getInt("add"));
    }
}

Map::~Map()
{
    for(int i = 0; i < m_biomes.size(); i++)
        SDL_DestroyTexture(m_biomes[i]);
    for(int i = 0; i < m_resources.size(); i++)
        SDL_DestroyTexture(m_resources[i]);

}

bool Map::get_error() const
{
    for(int i = 0; i < m_biomes.size(); i++)
    {
        if(!m_biomes[i])
        {
            std::string error = "map//biome_" + std::to_string(i + 1) + ".png n'a pas pu etre ouvert";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erreur de texture", error.c_str(), NULL);
            return true;
        }
    }
    for(int i = 0; i < m_resources.size(); i++)
    {
        if(!m_resources[i])
        {
            std::string error = "map//resource_" + std::to_string(i + 1) + ".png n'a pas pu etre ouvert";
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erreur de texture", error.c_str(), NULL);
            return true;
        }
    }
    return false;
}

void Map::generate()
{
    FileReader fr;
    fr.read("settings//map.txt");

    m_mapsize.x = fr.getInt("width");
    m_mapsize.y = fr.getInt("height");
    m_map.resize(m_mapsize.x, std::vector <int>(m_mapsize.y, 100)); //de base, le biome de la map est le biome_1 (au cas ou il n'y en a qu'un)

    std::vector <int> biome_percent;
    std::vector <int> resource_number;

    for(int i = 0; i < m_biomes.size(); i++)
        biome_percent.push_back(fr.getInt("biome_" + std::to_string(i + 1)));

    fr.read("settings//resources_init.txt");
    for(int i = 0; i < m_resources.size(); i++)
        resource_number.push_back(fr.getInt("resource_" + std::to_string(i + 1)));

    if(m_biomes.size() >= 2) //g�n�ration des biomes (s'il n'y en a qu'un, le travail est d�j� fait)
    {
        for(int biome = 1; biome < biome_percent.size(); biome++)
        {
            int previous_percents = 0;
            for(int i = 0; i < biome; i++)
                previous_percents += biome_percent[i];

            for(int x = m_mapsize.x * previous_percents / 100; x < m_mapsize.x * (previous_percents + biome_percent[biome]) / 100; x++)
                for(int y = 0; y < m_mapsize.y; y++)
                    set_biome(x, y, biome + 1);
        }
    }

    int x = 0, y = 0;
    for(int resource = 0; resource < m_resources.size(); resource++) //g�n�ration des ressources positionn�es correctement sur les biomes compatibles
    {
        for(int added = 0; added < resource_number[resource]; added++)
        {
            int attempts = 0;
            do
            {
                attempts += 1;
                x = rand() % m_mapsize.x;
                y = rand() % m_mapsize.y;
            }
            while((!is_free(x, y) || !resource_compatible_with_biome(resource + 1, get_biome(x, y))) && attempts < 100);

            if(attempts >= 100)
                break;
            set_resource(x, y, resource + 1);
        }
    }
}

void Map::render(SDL_Renderer *renderer, SDL_Point const& winsize, Camera const& camera)
{
    int camera_zoom = camera.get_zoom();
    SDL_Point camera_pos = camera.get_position();

    SDL_Rect position = {0, 0, camera_zoom, camera_zoom};
    for(int x = std::max(0, camera_pos.x/camera_zoom); x < std::min(m_mapsize.x, (camera_pos.x + winsize.x)/camera_zoom + 1); x++)
    {
        position.x = x * camera_zoom - camera_pos.x;
        for(int y = std::max(0, camera_pos.y/camera_zoom); y < std::min(m_mapsize.y, (camera_pos.y + winsize.y)/camera_zoom + 1); y++)
        {
            position.y = y * camera_zoom - camera_pos.y;
            SDL_RenderCopy(renderer, m_biomes[get_biome(x, y) - 1], NULL, &position);
            if(get_resource(x, y)) //there is a ressource on this block
                SDL_RenderCopy(renderer, m_resources[get_resource(x, y) - 1], NULL, &position);
        }
    }
}

void Map::update()
{
    if(SDL_GetTicks() - m_last_update >= 1000 / simulation_speed())
    {
        int x = 0, y = 0;
        for(int resource = 0; resource < m_resources.size(); resource++) //augmentation des ressources positionn�es correctement sur les biomes compatibles
        {
            int to_add = 0;
            if(m_adaptive[resource])
                to_add = m_target[resource] - get_number(resource + 1);
            else
                to_add = m_add[resource];

            for(int added = 0; added < to_add; added++)
            {
                int attempts = 0;
                do
                {
                    attempts += 1;
                    x = rand() % m_mapsize.x;
                    y = rand() % m_mapsize.y;
                }
                while((!is_free(x, y) || !resource_compatible_with_biome(resource + 1, get_biome(x, y))) && attempts < 100);

                if(attempts >= 100)
                    break;
                set_resource(x, y, resource + 1);
            }
        }
        m_last_update = SDL_GetTicks();
    }
}

SDL_Point Map::get_size() const
{
    return m_mapsize;
}

int Map::get_number(int resource) const
{
    int number = 0;
    for(int i = 0; i < m_mapsize.x; i++)
        for(int j = 0; j < m_mapsize.y; j++)
            if(get_resource(i, j) == resource)
                number += 1;
    return number;
}

int Map::get_biome(int x, int y) const
{
    return m_map[x][y] / 100;
}

int Map::get_resource(int x, int y) const
{
    return m_map[x][y] % 100;
}

void Map::remove_resource(int x, int y)
{
    m_map[x][y] = (m_map[x][y] / 100) * 100;
}

void Map::set_resource(int x, int y, int resource)
{
    m_map[x][y] = (m_map[x][y] / 100) * 100 + resource;
}

void Map::set_biome(int x, int y, int biome)
{
    m_map[x][y] = m_map[x][y] % 100 + 100 * biome;
}

bool Map::resource_compatible_with_biome(int resource, int biome) const
{
    for(int i = 0; i < m_location[resource-1].size(); i++)
        if(m_location[resource-1][i] == biome)
            return true;
    return false;
}

bool Map::is_free(int x, int y) const
{
    if(get_resource(x, y) != 0)
        return false;
    return true;
}
