#ifndef GRAPHPROCESSOR_H_
#define GRAPHPROCESSOR_H_

#include <opencv2/core/mat.hpp>

#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>
#include <utility>



template<typename T>
struct KeyHasherPair {
    using dot_t                 = std::pair<int, int>;
    using dotsPair_t            = std::pair<dot_t, dot_t>;
    std::size_t operator()(const dotsPair_t& k) const {
        using std::hash;
        auto h11 = hash<int>()(k.first.first);
        auto h12 = hash<int>()(k.first.second);
        auto h21 = hash<int>()(k.second.first);
        auto h22 = hash<int>()(k.second.second);
        return ((h11 ^ h12 ^ h21 ^ h22));
    }
};

class GraphProcessor {
public:

    using distance_t            = double;
    using dot_t                 = std::pair<int, int>;
    using dotsPair_t            = std::pair<dot_t, dot_t>;
    using totalDistances_t      = std::unordered_map<dotsPair_t, distance_t, KeyHasherPair<dotsPair_t>>;
    using nodes_t               = std::vector<std::pair<int, int>>;

    GraphProcessor(const int weight = 320, const int height = 240, const std::string imageName = "image") noexcept;
    ~GraphProcessor() noexcept;
    static void smMouseCallback(int event, int x, int y, int flags, void* param) noexcept;
    void connectMST() noexcept;
    void process(const int x, const int y) noexcept;
    void staticProcess() noexcept;
    void printNodes() noexcept;
    int lunch() noexcept;
private:
    void calculateDistances() noexcept;
    totalDistances_t::iterator findBiggestDistance() noexcept;
    void matchClosestPair(nodes_t::iterator& iter, int maxDist) noexcept;
    
    cv::Mat mpImage;
    std::string mImgWindowName;
    nodes_t nodes;
    nodes_t tree;
    nodes_t freeNodes;
    totalDistances_t mDistances;
    const int mImgRows;
    const int mImgCols;
};


#endif