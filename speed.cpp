#include "speed.hpp"
#include <algorithm>

//global variables but that cannot be accessed directly from anywhere in the programm, only here
int m_simulation_speed_target = 1;
int m_simulation_speed = m_simulation_speed_target;

void regulate_simulation_speed(int FPS)
{
    //regulating the simulation speed according to the FPS
    if((m_simulation_speed > 1 && FPS < 2*m_simulation_speed) || m_simulation_speed > m_simulation_speed_target)
        m_simulation_speed -= 1;
    else if(m_simulation_speed < m_simulation_speed_target && FPS > 2*(m_simulation_speed + 1))
        m_simulation_speed += 1;
}

void change_simulation_speed(bool increase)
{
    if(increase && m_simulation_speed_target < 100)
        m_simulation_speed_target += 1;
    else if(!increase && m_simulation_speed_target > 1)
        m_simulation_speed_target -= 1;
}

void set_simulation_speed(int target)
{
    m_simulation_speed_target = target;
}

int simulation_speed()
{
    return m_simulation_speed;
}

int simulation_speed_target()
{
    return m_simulation_speed_target;
}
