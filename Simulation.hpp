#ifndef DEF_SIMULATION
#define DEF_SIMULATION

#include <SDL2/SDL.h>
#include <map>
#include "Map.hpp"
#include "Population.hpp"
#include "Camera.hpp"
#include "Stats.hpp"

struct Settings
{
    int id, duration, speed, minFailureSave;
    bool stopOnFailure, saveOnFailure, hide;
    std::string directory;
};

class Simulation
{
public:
    Simulation(Settings const& settings);
    ~Simulation();

    void execute();
    bool get_error() const;
    void update();
    void render();
    bool process_events();

    bool confirm_exit();
    bool check_status();
    void save_status(int percent, bool has_failed);

private:
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;
    SDL_Point m_winsize;

    int m_leftclick;
    SDL_Point m_mousepos;
    bool m_render;
    bool m_paused;
    
    const Settings m_settings;

    Map *m_map;
    Camera *m_camera;
    Population *m_population;
    Stats *m_stats;
};

#endif
