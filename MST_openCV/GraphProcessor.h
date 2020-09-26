#ifndef GRAPHPROCESSOR_H_
#define GRAPHPROCESSOR_H_

#include <opencv2/core/mat.hpp>

#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>
#include <utility>

struct NodeCoords {
    NodeCoords() = delete;
    NodeCoords(int x, int y):
            x(x),
            y(y) {}
    NodeCoords(const std::pair<int,int>& coords):
            x(coords.first),
            y(coords.second) {}
    bool operator=(const NodeCoords& lhs) {
        return this->x==lhs.x && this->y==lhs.y;
    }
    int x;
    int y;
};

struct KeyHasherNode {
    std::size_t operator()(const NodeCoords& k) const {
        using std::size_t;
        using std::hash;
        using std::string;
        auto h1 = hash<int>()(k.x);
        auto h2 = hash<int>()(k.y) << 1;
        return ((h1 ^ h2) >> 1);
    }
};
/* TODO: create hash function for mDistances
struct KeyHasherPair {
    std::size_t operator()(const std::pair<NodeCoords, NodeCoords>& k) const {
        using std::size_t;
        using std::hash;
        using std::string;
        auto h1 = hash<NodeCoords>()(k.first);
        auto h2 = hash<NodeCoords>()(k.second) << 1;
        return ((h1 ^ h2) >> 1);
    }
};
*/
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
    //std::unordered_map<std::pair<NodeCoords, NodeCoords>, distance, TODO> mDistances;
    const int mImgRows;
    const int mImgCols;
};


#endif