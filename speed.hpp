#ifndef DEF_SPEED
#define DEF_SPEED

void regulate_simulation_speed(int FPS);
void change_simulation_speed(bool increase);
int simulation_speed();
int simulation_speed_target();
void set_simulation_speed(int target);

#endif
