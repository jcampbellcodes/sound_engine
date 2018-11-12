#include "AudioEngine.h"
#include <iostream>

int num_loops = 100;
int main()
{
    AudioEngine* eng = new AudioEngine(44100, 1, 64, 64);
    eng->playSound(0);

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

    return 0;
}