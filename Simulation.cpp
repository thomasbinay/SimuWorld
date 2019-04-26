#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <fstream>
#include <iostream>
#include "Simulation.hpp"
#include "speed.hpp"

Simulation::Simulation(Settings const& settings): m_settings(settings)
{
    srand(time(0));
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    if(m_settings.hide)
        m_window = SDL_CreateWindow("SimuWorld", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_HIDDEN);
    else
        m_window = SDL_CreateWindow("SimuWorld", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    SDL_GetWindowSize(m_window, &m_winsize.x, &m_winsize.y);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG);
    SDL_SetWindowIcon(m_window, IMG_Load("icone.png"));
    TTF_Init();

    m_map = new Map(m_renderer);
    m_population = new Population(m_renderer);
    m_camera = new Camera();
    m_stats = new Stats();

    m_leftclick = false;
    m_render = true;
    m_paused = false;

    m_map->generate();
    m_population->generate();

    set_simulation_speed(m_settings.speed);
}

Simulation::~Simulation()
{
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    delete m_map;
    delete m_camera;
    delete m_population;
    delete m_stats;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool Simulation::get_error() const
{
    if(m_map->get_error() || m_population->get_error() || m_stats->get_error())
        return true;
    return false;
}

void Simulation::execute()
{
    while(process_events() && check_status())
    {
        update();
        if(!m_settings.hide)
            render();
    }
}

void Simulation::update()
{
    if(!m_paused)
    {
        m_map->update();
        m_population->update(*m_map);
        m_stats->update(*m_population, *m_map);
    }
}

void Simulation::render()
{
    SDL_SetRenderDrawColor(m_renderer, 128, 128, 128, 255);
    SDL_RenderClear(m_renderer);
    if(m_render)
    {
        m_camera->update(*m_population, m_winsize);
        m_map->render(m_renderer, m_winsize, *m_camera);
        m_population->render(m_renderer, m_winsize, *m_camera);
    }
    m_stats->render(m_renderer, m_winsize);
    SDL_RenderPresent(m_renderer);
}

bool Simulation::process_events()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                if(confirm_exit())
                    return false;
            }
            else if(event.window.event == SDL_WINDOWEVENT_RESIZED)
                m_winsize = {event.window.data1, event.window.data2};
            break;

        case SDL_MOUSEBUTTONDOWN:
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                m_camera->focus_on_animal(*m_population, *m_stats, m_mousepos);
                m_leftclick = true;
            }
            else if(event.button.button == SDL_BUTTON_RIGHT)
            {
                if(event.button.x < 100)
                    m_stats->select_graph(m_mousepos);
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if(event.button.button == SDL_BUTTON_LEFT)
                m_leftclick = false;
            break;

        case SDL_MOUSEWHEEL:
            if(event.wheel.y < 0)
                m_camera->zoom(m_mousepos, false);
            else if(event.wheel.y > 0)
                m_camera->zoom(m_mousepos, true);
            break;

        case SDL_MOUSEMOTION:
            if(m_leftclick)
                m_camera->move(event.motion.x - m_mousepos.x, event.motion.y - m_mousepos.y);
            m_mousepos = {event.motion.x, event.motion.y};
            break;

        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_i)
                m_stats->show_hide_data();
            else if(event.key.keysym.sym == SDLK_g)
                m_stats->show_hide_graph();
            else if(event.key.keysym.sym == SDLK_r)
                m_render = !m_render;
            else if(event.key.keysym.sym == SDLK_p)
                m_paused = !m_paused;
            else if(event.key.keysym.sym == SDLK_b)
                m_population->show_hide_bubble();
            else if(event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_UP)
                change_simulation_speed(true);
            else if(event.key.keysym.sym == SDLK_KP_MINUS || event.key.keysym.sym == SDLK_DOWN)
                change_simulation_speed(false);
            else if(event.key.keysym.sym == SDLK_DELETE)
                m_stats->hide_all_graph();
            else if(event.key.keysym.sym == SDLK_s)
                m_population->show_hide_blood();
            break;
        }
    }
    return true;
}

bool Simulation::confirm_exit()
{
    const SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "annuler" },
        {                                       0, 1, "non" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 2, "oui" },
    };

    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION,
        NULL,
        "Fermeture de la simulation",
        "Voulez-vous sauvegarder les resultats ?",
        SDL_arraysize(buttons),
        buttons,
        NULL
    };

    int selection;
    SDL_ShowMessageBox(&messageboxdata, &selection);

    switch(selection)
    {
    case -1: case 0:
        return false;
    case 2:
        if(!m_stats->save(m_settings.directory, m_settings.id))
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fermeture impossible", "Une erreur est survenue durant la sauvegarde", NULL);
            return false;
        }
    }
    return true;
}

bool Simulation::check_status()
{
    if(m_settings.id == 0) //the programm has been launched from simuworld and not the launcher
        return true;

    bool has_finished = m_stats->has_finished(m_settings.duration);
    bool has_failed = m_stats->has_failed();
    int percent = m_stats->get_percent(m_settings.duration);
    save_status(percent, has_failed);
    if((m_settings.stopOnFailure && has_failed) || has_finished) //simulation failure or completed
    {
        if((m_settings.saveOnFailure && has_failed && (percent > m_settings.minFailureSave)) || has_finished) //failure and save still or completed
        {
            while(!m_stats->save(m_settings.directory, m_settings.id))
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fermeture impossible", "Une erreur est survenue durant la sauvegarde", NULL);
        }
        return false;
    }
    return true;
}

void Simulation::save_status(int percent, bool has_failed)
{
    std::ofstream file("status_" + std::to_string(m_settings.id) + ".txt");
    if(file)
    {
        file << percent << ";" << has_failed << std::endl;
    }
    file.close();
}