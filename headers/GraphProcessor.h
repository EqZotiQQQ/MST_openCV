#ifndef GRAPHPROCESSOR_H_
#define GRAPHPROCESSOR_H_

#include <opencv2/core/mat.hpp>

#include <unordered_map>
#include <utility>

#include "ThreadPool/ThreadPool/headers/ThreadPool.h"
#include "ThreadPool/ThreadPool/headers/ThreadManager.h"
#include "ThreadPool/ThreadPool/headers/ThreadsafeQueue.h"

/*r(0|1|2) options*/
enum RUN_TYPE {
    REAL_TIME = 0,
    LATENCY_FLOW = 1,
    STATIC_DATA = 2
};

/*f(0|1|2) options*/
enum FLOATING_MOUSE_NODE {
    OFF = 0,
    ON = 1,
    NEAREST_NODE = 2
};

template<typename T>
struct KeyHasherPair {
    using dot_t = std::pair<int, int>;
    using dots_pair_t = std::pair<dot_t, dot_t>;
    std::size_t operator()(const dots_pair_t& k) const {
        using std::hash;
        auto h11 = hash<int>()(k.first.first);
        auto h12 = hash<int>()(k.first.second);
        auto h21 = hash<int>()(k.second.first);
        auto h22 = hash<int>()(k.second.second);
        return ((h11 ^ h12 ^ h21 ^ h22));
    }
};

class GraphProcessor {
    using distance_t = double;
    using node_t = std::pair<int, int>;
    using pnode_t = node_t*;
    using dots_pair_t = std::pair<node_t, node_t>;
    using tst = std::vector<node_t>::iterator;
    using total_distances_t = std::unordered_map<dots_pair_t, distance_t, KeyHasherPair<dots_pair_t>>;
    using pconnection_t = std::pair<pnode_t, pnode_t>;
private:
    std::vector<node_t> nodes;
    std::vector<std::vector<double>> matrix;
    cv::Mat m_image;
    std::string m_window_name;

    RUN_TYPE m_run_type;
    FLOATING_MOUSE_NODE m_floating_node;
    const int m_img_rows;
    const int m_img_columns;
    int m_cnt_connections;
    ThreadPool tp;

    void create_line(const cv::Mat& image, const cv::Point&& start, const cv::Point&& end) noexcept;
    void init() noexcept;
    void place_circles_on_image() noexcept;
    void place_circles_on_subimage(cv::Mat& res, std::vector<node_t>::iterator begin, std::vector<node_t>::iterator end) noexcept;
    void refresh_img() noexcept;
    void print_statistics() noexcept;
    void calculate_graph(std::pair<int, int> pair) noexcept;
    void static_process() noexcept;
    void calculate_distances() noexcept;
    void connect_nearest(const int x, const int y) noexcept;
    void process_realtime(const int x, const int y, const bool mouse_call = false) noexcept;
    bool contains(const int x, const int y) noexcept;
    void change_connectivity(bool distination) noexcept;
    void connect_MST() noexcept;
    void draw_graph(const std::vector<int>& parent) noexcept;
    int min_key(const std::vector<int>& key, const std::vector<bool>& mst_set) noexcept;
    static total_distances_t::const_iterator find_max_distance(const total_distances_t& container) noexcept;
    [[noreturn]] void latency_flow() noexcept;
    static void s_mouse_callback(int event, int x, int y, int flags, void* param) noexcept;

public:
    GraphProcessor();
    GraphProcessor(const int rows,
        const int columns,
        std::string image_name,
        const FLOATING_MOUSE_NODE mouse,
        const RUN_TYPE run_type) noexcept;
    ~GraphProcessor() noexcept = default;
    int launch() noexcept;
};

#endif
