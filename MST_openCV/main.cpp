#ifdef __linux__ 
#include <getopt.h>
#endif
#include <iostream>
#include "headers/GraphProcessor.h"

int main(int argc, char* argv[]) {
    enum RUN_TYPE rt = RUN_TYPE::LATENCY_FLOW;
    enum FLOATING_MOUSE_NODE mouse_mod = FLOATING_MOUSE_NODE::OFF;
    int h = 320;
    int w = 480;
#ifdef __linux__
    int rez=0;
    while((rez = getopt(argc,argv,"r:f:h:w:")) != -1) {
        switch (rez) {
            case 'r': {
                if (std::string(optarg) == "0") {
                    rt = RUN_TYPE::REAL_TIME;
                } else if (std::string(optarg) == "2") {
                    rt = RUN_TYPE::STATIC_DATA;
                } else if (std::string(optarg) == "1") {
                    rt = RUN_TYPE::LATENCY_FLOW;
                } else {
                    printf("wrong option r\nTry next: -r(0|1|2) and/or -f(0|1) and/or -hN and/or -wN (where N - 0 ... N number");
                }
                break;
            }
            case 'f': {
                if (std::string(optarg) == "1") {
                    mouse_mod = FLOATING_MOUSE_NODE::ON;
                } else if (std::string(optarg) == "0") {
                    mouse_mod = FLOATING_MOUSE_NODE::OFF;
                } else {
                    printf("wrong option w\nTry next: -r(0|1|2) and/or -f(0|1) and/or -hN and/or -wN (where N - 0 ... N number");
                }
                break;
            }
            case 'h': {
                h = stoi(std::string(optarg));
                break;
            }
            case 'w': {
                w = stoi(std::string(optarg));
                break;
            }
            default: {
                printf("wrong option\nTry next: -r(0|1|2) and/or -f(0|1) and/or -hN and/or -wN (where N - 0 ... N number");
                break;
            }
        }
    }
#endif
    int ret = 0;
    GraphProcessor gp { rt, mouse_mod, h, w };
    ret = gp.launch();
    return ret;
}