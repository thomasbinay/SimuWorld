#include <SDL2/SDL.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cmath>
#include "Animal.hpp"
#include "speed.hpp"
#include "Population.hpp"
#include "FileReader.hpp"

Animal::Animal(Vector2d const& pos, int specie, int color, Animal *father, Animal *mother)
{
    m_pos = pos;
    m_dest = pos;

    m_specie = specie;
    m_male = rand() % 2;

    m_parents[0] = father;
    m_parents[1] = mother;

    m_direction = rand() % 4;
    m_animation = 0;

    m_last_move = SDL_GetTicks();
    m_last_animation = SDL_GetTicks();
    m_last_attack = SDL_GetTicks();
    m_last_update = SDL_GetTicks();

    m_alive = true;
    m_hungry = true;

    FileReader fr;
    fr.read("settings//specie_" + std::to_string(specie) + ".txt");

    m_damage = fr.getInt("damage");
    m_speed = fr.getDouble("speed");
    m_life_expectancy = fr.getInt("life_expectancy");
    m_maximum_health = fr.getInt("maximum_health");
    m_hunger_threshold = fr.getInt("hunger_threshold");
    m_satiated_threshold = fr.getInt("satiated_threshold");
    m_agressive = fr.getInt("agressive");
    m_plant_range_detection = fr.getInt("plant_range_detection");
    m_prey_range_detection = fr.getInt("prey_range_detection");
    m_partner_range_detection = fr.getInt("partner_range_detection");
    m_agressivity_range = fr.getInt("agressivity_range");
    m_search_distance = fr.getInt("search_distance");
    m_reproduction_time = fr.getInt("reproduction_time");
    m_decomposition_time = fr.getInt("decomposition_time");
    m_plants = fr.getVectorInt("plants");
    m_preys = fr.getVectorInt("preys");

    std::vector <int> colors = fr.getVectorInt("colors");
    if(color >= 0)
        m_color = color;
    else if(colors.size())
        m_color = colors[rand() % colors.size()] - 1;
    else
        m_color = rand() % 8;

    m_time_before_reproduction = m_reproduction_time;
    m_time_before_decomposition = m_decomposition_time;

    m_health = m_hunger_threshold; //starts at his hunger threshold so he immediatly starts looking for food
}

Animal::~Animal()
{}

int Animal::get_direction() const
{
    return m_direction;
}

int Animal::get_specie() const
{
    return m_specie;
}

double Animal::get_speed() const
{
    return m_speed;
}

int Animal::get_color() const
{
    return m_color;
}

int Animal::get_animation() const
{
    return m_animation;
}

int Animal::get_health() const
{
    return m_health;
}

bool Animal::is_alive() const
{
    return m_alive;
}

int Animal::get_diet() const
{
    if(m_preys.size() && m_plants.size())
        return OMNIVORE;
    if(m_preys.size())
        return CARNIVORE;
    return HERBIVORE;
}

Vector2d Animal::get_position() const
{
    return m_pos;
}

Vector2d Animal::get_destination() const
{
    return m_dest;
}

void Animal::set_destination(Vector2d const& dest)
{
    m_dest = dest;
}

bool Animal::move()
{
    if(SDL_GetTicks() - m_last_move >= 1) //moves at most every ms (and requires at least to move every 0.5s)
    {
        double diff_x = m_dest.x - m_pos.x;
        double diff_y = m_dest.y - m_pos.y;

        double distance = sqrt(pow(diff_x, 2) + pow(diff_y, 2));
        if(distance <= 0.25) //has already arrived to his destination
            return true;

        double angle = atan2(diff_y, diff_x);

        double actual_speed = std::min(distance, m_speed * simulation_speed() * std::min(0.5, (SDL_GetTicks() - m_last_move)/1000.0)); //cannot exceed its destination (Could exit the map !)
        m_pos.x += actual_speed * cos(angle);
        m_pos.y += actual_speed * sin(angle);

        if(2 * abs(diff_x) > abs(diff_y)) //gives the main direction for the animation
        {
            if(diff_x > 0)
                m_direction = RIGHT;
            else
                m_direction = LEFT;
        }
        else
        {
            if(diff_y > 0)
                m_direction = BOT;
            else
                m_direction = TOP;
        }

        if(SDL_GetTicks() - m_last_animation >= 100/simulation_speed()) //update the animation cycle
        {
            if(m_animation < 2)
                m_animation += 1;
            else
                m_animation = 0;
            m_last_animation = SDL_GetTicks();
        }

        m_last_move = SDL_GetTicks();
    }
    return false;
}

void Animal::regenerate(int health)
{
    m_health += health;
    if(m_health > m_maximum_health)
        m_health = m_maximum_health;
}

void Animal::update_stats() //update the vital stats (even if he is dead to update the time before his decomposition)
{
    if(SDL_GetTicks() - m_last_update >= 1000 / simulation_speed())
    {
        if(m_alive)
        {
            m_life_expectancy -= 1;
            m_health -= 1;
            m_time_before_reproduction -= 1;

            if(m_health < m_hunger_threshold) //starts hunting again
                m_hungry = true;
            else if(m_health > m_satiated_threshold) //stops hunting
                m_hungry = false;

            if(m_life_expectancy <= 0 || m_health <= 0) //death statement
            {
                m_health = 0;
                m_alive = false;
            }
        }
        else
            m_time_before_decomposition -= 1;

        m_last_update = SDL_GetTicks();
    }
}

void Animal::take_damage(int damage)
{
    m_health -= damage;
    if(m_health <= 0)
        m_alive = false;
}

bool Animal::attack(Animal &target) //returnes if the animal was able to attack (used to add blood)
{
    if(SDL_GetTicks() - m_last_attack >= 1000 / simulation_speed() && m_damage > 0) //animal ready to attack (and can actually do it)
    {
        target.take_damage(m_damage);
        m_last_attack = SDL_GetTicks();
        return true;
    }
    return false;
}

bool Animal::is_my_prey(Animal const& target) const
{
    int target_specie = target.get_specie();
    for(int n = 0; n < m_preys.size(); n++)
        if(m_preys[n] == target_specie)
            return true;
    return false;
}

bool Animal::is_my_plant(int plant) const
{
    for(int n = 0; n < m_plants.size(); n++)
        if(plant == m_plants[n])
            return true;
    return false;
}

bool Animal::is_male() const
{
    return m_male;
}

bool Animal::is_ready_to_reproduce() const
{
    return !is_hungry() && (m_time_before_reproduction <= 0 || m_male);
}

void Animal::reproduce()
{
    m_time_before_reproduction = m_reproduction_time;
}

bool Animal::is_decomposed() const
{
    return m_time_before_decomposition <= 0;
}

bool Animal::is_hungry() const
{
    return m_hungry;
}

bool Animal::is_agressive() const
{
    return m_agressive && m_male && m_health > m_hunger_threshold;
}

bool Animal::is_agressive_with(Animal const* target) const
{
    return is_agressive() && target->is_alive() && (m_specie == target->get_specie()) && !is_my_parent(target) && !target->is_my_parent(this) && target->is_agressive();
}

bool Animal::is_my_type(Animal const& target) const
{
    return is_ready_to_reproduce() && target.is_ready_to_reproduce() && target.is_alive() && (m_specie == target.get_specie()) && (m_male != target.is_male());
}

int Animal::get_plant_range_detection() const
{
    return m_plant_range_detection;
}

int Animal::get_prey_range_detection() const
{
    return m_prey_range_detection;
}

int Animal::get_partner_range_dection() const
{
    return m_partner_range_detection;
}

int Animal::get_agressivity_range() const
{
    return m_agressivity_range;
}

bool Animal::is_my_parent(Animal const* target) const
{
    return (m_parents[0] == target) || (m_parents[1] == target);
}

int Animal::get_search_distance() const
{
    return m_search_distance;
}
