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

    GraphProcessor(const int rows = 800, const int columns = 800, const std::string image_name = "image") noexcept;
    ~GraphProcessor() noexcept;
    static void s_mouse_callback(int event, int x, int y, int flags, void* param) noexcept;
    void connect_MST() noexcept;
    void process_realtime(const int x, const int y) noexcept;
    void static_process() noexcept;
    void print_data() noexcept;
    int launch() noexcept;
private:
    void calculate_distances() noexcept;
    void print_distances() noexcept;
    void create_line(const cv::Point&& start, const cv::Point&& end) noexcept;
    void create_circles() noexcept;
    //totalDistances_t::iterator findBiggestDistance() noexcept;
    double find_max_distance() noexcept;
    void clean_entries() noexcept;
    cv::Mat m_image;
    std::string m_window_name;
    nodes_t m_all_nodes;
    nodes_t m_connected_nodes;
    nodes_t m_not_connected_nodes;
    totalDistances_t m_distances;
    const int m_img_rows;
    const int m_img_columns;
};


#endif