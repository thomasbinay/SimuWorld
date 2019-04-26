#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <fstream>

#include <dirent.h>
#ifndef WIN32
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "Animal.hpp"
#include "Stats.hpp"
#include "speed.hpp"
#include "FileReader.hpp"
#include <sstream>

Stats::Stats()
{
    m_show_graph = false;
    m_show_data = true;

    m_last_update = -1000; //needs to update the stats immediatly at the beginning
    m_elapsed_time = -1; //important too
    m_FPS = 0;

    FileReader fr;
    fr.read("settings//font.txt");
    m_font = TTF_OpenFont("font.ttf", fr.getInt("size"));
    m_color = {fr.getInt("red"), fr.getInt("green"), fr.getInt("blue")};
    m_spacing = fr.getInt("spacing");

    fr.read("settings//species_name.txt");
    m_species_number = fr.getCount();
    for(int i = 0; i < m_species_number; i++)
        m_names.push_back(fr.getString("specie_" + std::to_string(i + 1)));

    fr.read("settings//resources_name.txt");

    m_resources_number = fr.getCount();
    for(int i = 0; i < m_resources_number; i++)
        m_names.push_back(fr.getString("resource_" + std::to_string(i + 1)));

    m_selected_graph.resize(m_names.size(), false);
    m_maximum_value.resize(m_names.size(), 0);

    m_color_warning = {255, 0, 0};
    m_last_frame = SDL_GetTicks();
}

Stats::~Stats()
{
    TTF_CloseFont(m_font);
}

bool Stats::get_error() const
{
    if(!m_font)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erreur de police", "font.ttf n'a pas pu etre ouvert", NULL);
        return true;
    }
    return false;
}

void Stats::update(Population const& population, Map const& map)
{
    
    m_render_time.insert(m_render_time.begin(), SDL_GetTicks() - m_last_frame);
    m_last_frame = SDL_GetTicks();
    if(m_render_time.size() > 10)
        m_render_time.pop_back();
    int total_time = 0;
    for(int i = 0; i < m_render_time.size(); i++)
        total_time += m_render_time[i];
    m_FPS = 1000.0/total_time*m_render_time.size();
    regulate_simulation_speed(m_FPS);

    if(SDL_GetTicks() - m_last_update >= 1000 / simulation_speed())
    {
        m_elapsed_time += 1;

        //update the stats of all the differents species
        m_values.push_back(std::vector <int>(m_resources_number + m_species_number));
        for(int specie = 0; specie < m_species_number; specie++)
            m_values[m_elapsed_time][specie] = population.get_number(specie + 1);

        for(int resource = 0; resource < m_resources_number; resource++)
            m_values[m_elapsed_time][resource + m_species_number] = map.get_number(resource + 1);

        //update the maximum values
        for(int i = 0; i < m_resources_number + m_species_number; i++)
            if(m_values[m_elapsed_time][i] > m_maximum_value[i])
                m_maximum_value[i] = m_values[m_elapsed_time][i];

        m_last_update = SDL_GetTicks();
    }
}

void Stats::render(SDL_Renderer *renderer, SDL_Point const& winsize)
{
    if(m_show_data)
    {
        int line = 0;
        std::string text;

        if(simulation_speed() == simulation_speed_target())
            text = "vitesse: " + std::to_string(simulation_speed()) + "x";
        else
            text = "vitesse: " + std::to_string(simulation_speed()) + "x (cible: " + std::to_string(simulation_speed_target()) + "x)";
        render_text(renderer, text, line);

        line += 1;
        text = std::to_string(m_elapsed_time / 3600) + "h " + std::to_string(m_elapsed_time % 3600 / 60) + "m " + std::to_string(m_elapsed_time % 60) + "s";
        render_text(renderer, text, line);

        line += 1;
        text = std::to_string(m_FPS) + " FPS";
        if(m_FPS < 2*simulation_speed()) //the simulation is running to slow
            render_text(renderer, text, line, m_color_warning);
        else
            render_text(renderer, text, line);

        line += 1;
        for(int i = 0; i < m_resources_number + m_species_number; i++)
        {
            if(m_maximum_value[i]) //only displays if there has been one at least since the beginning
            {
                if(i == m_species_number)
                    line += 2;
                else
                    line += 1;
                text = m_names[i] + ": " + std::to_string(m_values[m_elapsed_time][i]);
                render_text(renderer, text, line);
            }
        }
    }
    if(m_show_graph)
    {
        SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 255);

        //define the maximum value for the scale of the graph
        int maximum = 0;
        for(int i = 0; i < m_maximum_value.size(); i++)
            if(m_selected_graph[i] && m_maximum_value[i] > maximum)
                maximum = m_maximum_value[i];

        for(int t = 1; t <= m_elapsed_time; t++)
        {
            for(int i = 0; i < m_resources_number + m_species_number; i++)
                if(m_selected_graph[i])
                    SDL_RenderDrawLine(renderer, (1.0*winsize.x/m_elapsed_time)*(t-1), winsize.y - (1.0*winsize.y/maximum) * m_values[t-1][i], (1.0*winsize.x/m_elapsed_time)*t, winsize.y - (1.0*winsize.y/maximum) * m_values[t][i]);
        }

        //draw the axes of reference values
        for(int i = 0; i < 10; i++)
        {
            SDL_RenderDrawLine(renderer, 0, winsize.y/10*i, winsize.x, winsize.y/10*i);
            render_text(renderer, winsize, maximum, i);
        }
    }
}

void Stats::show_hide_data()
{
    m_show_data = !m_show_data;
}

void Stats::show_hide_graph()
{
    m_show_graph = !m_show_graph;
}

void Stats::reset()
{
    m_last_update = SDL_GetTicks() - 1000;
    m_elapsed_time = -1;
    m_maximum_value.assign(m_maximum_value.size(), 0);
    m_values.clear();
}

void Stats::render_text(SDL_Renderer *renderer, std::string text, int line)
{
    SDL_Surface *surface = TTF_RenderText_Blended(m_font, text.c_str(), m_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {0, line * m_spacing, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Stats::render_text(SDL_Renderer *renderer, std::string text, int line, SDL_Color color)
{
    SDL_Surface *surface = TTF_RenderText_Blended(m_font, text.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {0, line * m_spacing, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Stats::render_text(SDL_Renderer *renderer, SDL_Point const& winsize, int maximum, int line)
{
    SDL_Surface *surface = TTF_RenderText_Blended(m_font, std::to_string(maximum - maximum/10*line).c_str(), m_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {winsize.x - surface->w, line * winsize.y/10, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool Stats::save(std::string root, int id) //save the stats of the current simulation in a file
{
    DIR *directory = 0;
    std::string folder_name;

    //id specified -> overwrite previous folder if needed
    if(id)
    {
        folder_name = root + "//simulation_" + std::to_string(id);
        directory = opendir(folder_name.c_str());
        if(directory)
        {
            closedir(directory);
            rmdir(folder_name.c_str()); 
        }
    }
    else //id not specified (launching on its own) -> find empty folder in default root "settings"
    { 
        
        for(int i = 1; i < 10000; i++)
        {
            folder_name = "results//simulation " + std::to_string(i);
            directory = opendir(folder_name.c_str());
            if(directory)
                closedir(directory);
            else
                break;
        }
    }
    #if defined(_WIN32) //creates the folder for the simulation
        mkdir(folder_name.c_str());
    #else 
        mkdir(folder_name.c_str(), 0777); 
    #endif 

    std::string name = folder_name + "//settings"; //creates the subfolder for the settings
    

    #if defined(_WIN32)
        mkdir(name.c_str());
    #else 
        mkdir(name.c_str(), 0777); 
    #endif



    //copy the settings in the settings folder of the simulation
    std::ifstream buffer;
    std::ofstream file;

    struct dirent *content = NULL;
    directory = opendir("settings");
    seekdir(directory, 2);
    while(content = readdir(directory))
    {
        //converts to string
        std::ostringstream cv;
        cv << content->d_name;

        name = "settings//" + cv.str();
        buffer.open(name.c_str());
        name = folder_name + "//settings//" + cv.str();
        file.open(name.c_str());
        file << buffer.rdbuf();
        file.close();
        buffer.close();
    }

    //saves the stats of the simulation
    name = folder_name + "//stats.txt";
    file.open(name.c_str());
    if(file)
    {
        //prints the name of differents values
        file << "TIME ";
        for(int i = 0; i < m_names.size(); i++)
        {
            if(m_maximum_value[i])
                file << m_names[i] << " ";
        }
        file << std::endl;

        //prints the values
        for(int t = 0; t < m_elapsed_time; t++)
        {
            file << t << " ";
            for(int i = 0; i < m_resources_number + m_species_number; i++)
                if(m_maximum_value[i])
                    file << m_values[t][i] << " ";
            file << std::endl;
        }
        return true;
    }
    return false;
}

int Stats::get_mouse_focus(SDL_Point const& mouse_pos) const //returnes which specie is targeted by the mouse
{
    if(mouse_pos.x < 100)
    {
        int line = 3;
        for(int specie = 0; specie < m_species_number; specie++)
        {
            if(m_maximum_value[specie])
            {
                line += 1;
                if(mouse_pos.y/m_spacing == line)
                    return specie + 1;
            }
        }
    }
    return 0;
}

void Stats::select_graph(SDL_Point const& mouse_pos) //select which graph of population should be displayed in real time
{
    int line = 3;
    for(int graph = 0; graph < m_resources_number + m_species_number; graph++)
    {
        if(m_maximum_value[graph])
        {
            if(graph == m_species_number)
                line += 2;
            else
                line += 1;
            if(mouse_pos.y/m_spacing == line)
                m_selected_graph[graph] = !m_selected_graph[graph];
        }
    }
}

void Stats::hide_all_graph()
{
    for(int i = 0; i < m_selected_graph.size(); i++)
        m_selected_graph[i] = false;
}

bool Stats::has_failed()
{
    if(m_elapsed_time < 1) //simulation has just started
        return false;
    for(int i = 0; i < m_species_number; i++)
        if(m_maximum_value[i])
            if(m_values[m_elapsed_time - 1][i] == 0) //one specie is extinguished -> failure
                return true;
    return false;
}

bool Stats::has_finished(int duration)
{
    if(duration == 0) //no duration limit 
        return false;
    return m_elapsed_time >= duration;
}

int Stats::get_percent(int duration)
{
    if(duration == 0)
        return 0;
    return 100*m_elapsed_time/duration;
}