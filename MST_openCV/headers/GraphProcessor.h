#ifndef GRAPHPROCESSOR_H_
#define GRAPHPROCESSOR_H_

#include <opencv2/core/mat.hpp>

#include <unordered_map>
#include <utility>

enum class RUN_TYPE {
    REAL_TIME = 0,
    LATENCY_FLOW = 1,
    STATIC_DATA = 2
};

enum class FLOATING_MOUSE_NODE {
    OFF = 0,
    ON = 1
};

template<typename T>
struct KeyHasherPair {
    using dot_t                  = std::pair<int, int>;
    using dots_pair_t            = std::pair<dot_t, dot_t>;
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
public:
    using distance_t            = double;
    using dot_t                 = std::pair<int, int>;
    using dots_pair_t           = std::pair<dot_t, dot_t>;
    using total_distances_t     = std::unordered_map<dots_pair_t, distance_t, KeyHasherPair<dots_pair_t>>;
    using nodes_t               = std::vector<dot_t>;

    GraphProcessor(RUN_TYPE rt, FLOATING_MOUSE_NODE fmn, const int rows = 5, const int columns = 10, const std::string image_name = "image") noexcept;
    ~GraphProcessor() noexcept;
    int launch() noexcept;
private:
    void change_connectivity(bool distination) noexcept;
    void process_realtime(const int x, const int y, const bool mouse_call = false) noexcept;
    void connect_nearest(const int x, const int y) noexcept;
    void print_data() noexcept;
    void connect_MST() noexcept;
    void static_process() noexcept;

    [[noreturn]] void latency_flow() noexcept;
    void calculate_distances() noexcept;
    void create_line(const cv::Mat& image, const cv::Point&& start, const cv::Point&& end) noexcept;
    void create_circles() noexcept;
    static void s_mouse_callback(int event, int x, int y, int flags, void* param) noexcept;
    static total_distances_t::const_iterator find_max_distance(const total_distances_t& container) noexcept;
    void clean_entries() noexcept;
    void print_connected() noexcept;
    void print_not_connected() noexcept;
    void print_all_nodes() noexcept;
    void print_distances() noexcept;
    bool contains(const int x, const int y) noexcept;

    cv::Mat m_image;
    std::string m_window_name;
    nodes_t m_all_nodes;
    nodes_t m_connected_nodes;
    nodes_t m_not_connected_nodes;
    total_distances_t m_distances;
    RUN_TYPE m_run_type;
    FLOATING_MOUSE_NODE m_floating_node;
    const int m_img_rows;
    const int m_img_columns;
    int m_cnt_connections;
};

#endif