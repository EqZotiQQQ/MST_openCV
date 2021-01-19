#if defined(__linux__) or defined(__APPLE__)
#include <getopt.h>
#elif _WIN32
#include <string>
#endif
#include <iostream>
#include "headers/GraphProcessor.h"

void print_help();

int main(int argc, char* argv[]) {
    enum RUN_TYPE rt = RUN_TYPE::LATENCY_FLOW;
    enum FLOATING_MOUSE_NODE mouse_mod = FLOATING_MOUSE_NODE::OFF;
    int h = 150;
    int w = 350;
#if defined(__linux__) or defined(__APPLE__)
    int rez = 0;
    while ((rez = getopt(argc, argv, "r:f:h:w:")) != -1) {
        switch (rez) {
        case 'r': {
            if (std::string(optarg) == "0") {
                rt = RUN_TYPE::REAL_TIME;
            }
            else if (std::string(optarg) == "1") { 
                rt = RUN_TYPE::LATENCY_FLOW;
            }
            else if (std::string(optarg) == "2") {
                rt = RUN_TYPE::STATIC_DATA;
            }
            else {
                print_help();
            }
            break;
        }
        case 'f': {
            if (std::string(optarg) == "0") {
                mouse_mod = FLOATING_MOUSE_NODE::OFF;
            }
            else if (std::string(optarg) == "1") {
                mouse_mod = FLOATING_MOUSE_NODE::ON;
            }
            else {
                print_help();
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
            print_help();
            break;
        }
        }
    }
#elif _WIN32
    std::vector<std::string> arguments;
    if (argc > 1) {
        arguments.reserve(argc - 1);
        for (int i = 1; i < argc; ++i) {
            arguments.push_back(std::string(argv[i]));
        }
        for (auto i = 0; i < arguments.size(); ++i) {
            if (*arguments[i].substr(0, 1).c_str() != '-') {
                continue;
            }
            const char key = *arguments[i].substr(1, 2).c_str();
            switch (key) {
            case 'r': {
                std::string key_value_s = arguments[i].substr(2, 3);
                if (key_value_s == "0") {
                    rt = RUN_TYPE::REAL_TIME;
                }
                else if (key_value_s == "1") {
                    rt = RUN_TYPE::LATENCY_FLOW;
                }
                else if (key_value_s == "2") {
                    rt = RUN_TYPE::STATIC_DATA;
                }
                else {
                    print_help();
                }
                break;
            }
            case 'f': {
                std::string key_value_s = arguments[i].substr(2, 3);
                if (key_value_s == "0") {
                    mouse_mod = FLOATING_MOUSE_NODE::OFF;
                }
                else if (key_value_s == "1") {
                    mouse_mod = FLOATING_MOUSE_NODE::ON;
                }
                else if (key_value_s == "2") {
                    mouse_mod = FLOATING_MOUSE_NODE::NEAREST_NODE;
                }
                else {
                    print_help();
                }
                break;
            }
            case 'h': {
                std::string key_value_s = arguments[i].substr(2, arguments[i].size() - 1);
                h = stoi(key_value_s);
                break;
            }
            case 'w': {
                std::string key_value_s = arguments[i].substr(2, arguments[i].size() - 1);
                w = stoi(key_value_s);
                break;
            }
            default: {
                print_help();
                break;
            }
            }
        }
    }
#endif
    int ret = 0;
    //GraphProcessor gp;
    GraphProcessor gp(h,w, "image", mouse_mod, rt);
    //GraphProcessor gp;
    //gp.set_options(mouse_mod, rt);

    ret = gp.launch();
    return ret;
}

void print_help() {
    printf("You have entered wrong options.\n");
    printf("-r(1|2|3).\n");
    printf("-f(1|2|3).\n");
    printf("-h(N).\n");
    printf("-w(N).\n");
    std::exit(1);
}