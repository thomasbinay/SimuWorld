#include "Simulation.hpp"
#include <map>
#include <iostream>

Settings getSettings(int argc, char *argv[])
{
    Settings settings;
    if(argc == 1) //default settings when lauching from the programm itself
    {
        settings.id = 0;
        settings.duration = 0;
        settings.speed = 1;
        settings.stopOnFailure = false;
        settings.saveOnFailure = false;
        settings.minFailureSave = 0;
        settings.hide = false;
        settings.directory = "results";

    }
    else //reading settings from argv when launching from the launcher
    {
        settings.id = std::stoi(argv[0]);
        settings.duration = std::stoi(argv[1]);
        settings.speed = std::stoi(argv[2]);
        settings.stopOnFailure = std::stoi(argv[3]);
        settings.saveOnFailure = std::stoi(argv[4]);
        settings.minFailureSave = std::stoi(argv[5]);
        settings.hide = std::stoi(argv[6]);
        settings.directory = argv[7];
    }
    return settings;
}

int main(int argc, char *argv[])
{
    Simulation *simulation = new Simulation(getSettings(argc, argv));

    if(simulation->get_error()) //checks if the simulation has been created successfully
        return 0;

    simulation->execute();

    delete simulation;
}
