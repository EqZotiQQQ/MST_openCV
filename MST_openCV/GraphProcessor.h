#ifndef GRAPHPROCESSOR_H_
#define GRAPHPROCESSOR_H_

#include <opencv2/core/mat.hpp>

#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>

#include <utility>

class Coords {
    int x;
    int y;
};

class GraphProcessor {
public:
    using distance = double;
    GraphProcessor(const int weight = 320, const int height = 240, const std::string imageName = "image") noexcept;
    ~GraphProcessor() noexcept;
    static void smMouseCallback(int event, int x, int y, int flags, void* param) noexcept;
    void connectMST() noexcept;
    void process(const int x, const int y) noexcept;
    void printNodes() noexcept;
    int lunch() noexcept;
private:
    cv::Mat mpImage;
    std::string mImgWindowName;
    std::vector<std::pair<int,int>> nodes;
    std::vector<std::pair<int,int>> tree;
    std::vector<std::pair<int,int>> freeNodes;
    //std::unordered_map<std::pair<int, int>, distance> mDistances;
    const int mImgRows;
    const int mImgCols;
};


#endif