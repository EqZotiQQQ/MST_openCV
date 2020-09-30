#ifdef __linux__ 
#include <getopt.h>
#endif
#include <stdio.h>
#include <iostream>
#include "headers/GraphProcessor.h"

int main(int argc, char* argv[]) {
#ifdef __linux__ 
    const struct options opt[] = {
        {"fm", 0, &set_floatng_mouse, 1},
        {"st", 0, &set_static_simulation, 1},
        {"l", 0, &set_latency_simulation, 1} };
#elif _WIN32
    //TODO
#endif
    int ret = 0;
    GraphProcessor gp { RUN_TYPE::REAL_TIME, FLOATING_MOUSE_NODE::OFF };
    ret = gp.launch();
    return ret;
}