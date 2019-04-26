#ifndef DEF_STATS
#define DEF_STATS

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Population.hpp"
#include "Map.hpp"

class Stats
{
public:
    Stats();
    ~Stats();

    void update(class Population const& population, class Map const& map);

    void render(SDL_Renderer *rederer, SDL_Point const& winsize);

    bool get_error() const;
    int get_mouse_focus(SDL_Point const& mouse_pos) const;

    void reset();

    bool save(std::string root, int id);

    void show_hide_data();
    void show_hide_graph();
    void select_graph(SDL_Point const& mouse_pos);
    void hide_all_graph();

    bool has_failed();
    bool has_finished(int duration);
    int get_percent(int duration);

private:
    void render_text(SDL_Renderer *renderer, std::string text, int line);
    void render_text(SDL_Renderer *renderer, std::string text, int line, SDL_Color color);
    void render_text(SDL_Renderer *renderer, SDL_Point const& winsize, int maximum, int line);

    TTF_Font *m_font;
    SDL_Color m_color;
    SDL_Color m_color_warning;
    int m_spacing;

    int m_elapsed_time;
    int m_last_update;
    int m_FPS;
    int m_last_frame;
    std::vector<int> m_render_time;

    bool m_show_graph;
    bool m_show_data;

    std::vector <int> m_maximum_value;
    std::vector <bool> m_selected_graph;

    std::vector < std::vector <int> > m_values;
    int m_resources_number;
    int m_species_number;

    std::vector <std::string> m_names;
};

#endif
