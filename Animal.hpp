#ifndef DEF_ANIMAL
#define DEF_ANIMAL

#include <SDL2/SDL.h>
#include <vector>

enum direction
{
    BOT, LEFT, RIGHT, TOP
};

enum diet
{
    CARNIVORE, HERBIVORE, OMNIVORE
};

typedef struct Vector2d Vector2d;
struct Vector2d
{
    double x, y;
};

class Animal
{
public:
    Animal(Vector2d const& pos, int specie, int color = -1, Animal *father = NULL, Animal *mother = NULL);
    ~Animal();

    int get_direction() const;
    int get_specie() const;
    double get_speed() const;
    int get_color() const;
    int get_animation() const;
    int get_health() const;
    int get_diet() const;
    Vector2d get_position() const;
    Vector2d get_destination() const;
    int get_plant_range_detection() const;
    int get_prey_range_detection() const;
    int get_partner_range_dection() const;
    int get_agressivity_range() const;
    int get_search_distance() const;

    void set_destination(Vector2d const& des);

    void regenerate(int health);

    bool move();
    void update_stats();

    void take_damage(int damage);
    bool attack(Animal &target);
    void reproduce();

    bool is_alive() const;
    bool is_male() const;
    bool is_hungry() const;
    bool is_my_prey(Animal const& target) const;
    bool is_my_plant(int plant) const;
    bool is_my_type(Animal const& target) const;
    bool is_my_parent(Animal const* target) const;
    bool is_agressive() const;
    bool is_agressive_with(Animal const* target) const;
    bool is_ready_to_reproduce() const;
    bool is_decomposed() const;

private:
    int m_specie;
    int m_color;
    int m_direction;
    int m_animation;
    int m_last_move;
    int m_last_animation;

    Vector2d m_pos;
    Vector2d m_dest;

    int m_last_update;
    int m_health;
    int m_maximum_health;
    double m_speed;
    int m_life_expectancy;

    int m_last_attack;
    int m_damage;

    bool m_male;
    int m_time_before_reproduction;
    int m_time_before_decomposition;

    bool m_hungry;
    int m_hunger_threshold;
    int m_satiated_threshold;

    bool m_agressive;
    bool m_alive;

    int m_plant_range_detection;
    int m_prey_range_detection;
    int m_partner_range_detection;
    int m_agressivity_range;
    int m_search_distance;

    int m_reproduction_time;
    int m_decomposition_time;

    std::vector <int> m_preys;
    std::vector <int> m_plants;
    Animal* m_parents[2];
};


#endif
