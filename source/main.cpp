#include "sound_engine.h"
#include <iostream>

int num_loops = 2000;
int main()
{
    sound_engine* eng = new sound_engine(44100, 1, 64, 64);

    //std::thread t1(&sound_engine::update, &eng);
    eng->play_sound(0);
    // for(int i = 0 ; i < 10; i++)
    // {
    //     eng->play_sound(i);
    // }

    // for(int i = 0 ; i < 10; i++)
    // {
    //     eng.stop_sound(i);
    // }

    int quit = 0;
    while(quit < num_loops)
    {
        eng->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        quit++;
    }

    printf("quit\n");
    eng->quit();
    delete eng;
    //t1.join();

    return 0;
}