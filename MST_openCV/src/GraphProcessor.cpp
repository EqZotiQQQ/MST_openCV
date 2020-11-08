#include "../headers/GraphProcessor.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <utility>
#include <thread>
#include <random>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


using distance_t = double;
using node_t = std::pair<int, int>;
using pnode_t = node_t*;
using dots_pair_t = std::pair<node_t, node_t>;
using total_distances_t = std::unordered_map<dots_pair_t, distance_t, KeyHasherPair<dots_pair_t>>;


GraphProcessor::GraphProcessor() :
    m_img_rows(800),
    m_img_columns(1200),
    m_window_name("Graph"),
    m_cnt_connections(1),
    m_floating_node(FLOATING_MOUSE_NODE::OFF),
    m_run_type(RUN_TYPE::LATENCY_FLOW),
    m_image(cv::Mat(800, 1200, CV_8UC3, cv::Scalar(0, 0, 0)))
{

}

GraphProcessor::GraphProcessor(const int rows,
    const int columns,
    const std::string& image_name,
    const FLOATING_MOUSE_NODE mouse,
    const RUN_TYPE run_type) noexcept :
    m_img_rows(rows),
    m_img_columns(columns),
    m_window_name(image_name),
    m_cnt_connections(1),
    m_floating_node(mouse),
    m_run_type(run_type),
    m_image(cv::Mat(rows, columns, CV_8UC3, cv::Scalar(0, 0, 0)))
{

    printf("Image size: [%d %d]\n", m_image.rows, m_image.cols);
    printf("Press esc button to exit.\n");
    printf("scroll up/down to increase/reduce number of connections\n");
}

GraphProcessor::~GraphProcessor() noexcept {
}

int GraphProcessor::launch() noexcept {
    std::thread worker;
    auto lunch_status{ true };
    cv::namedWindow(m_window_name, cv::WINDOW_AUTOSIZE);
    cv::imshow(m_window_name, m_image);
    while (lunch_status) {
        if (m_run_type == RUN_TYPE::STATIC_DATA) {
            static_process();
        }
        else if (m_run_type == RUN_TYPE::REAL_TIME) {
            cv::setMouseCallback(m_window_name, s_mouse_callback, this);
        }
        else if (m_run_type == RUN_TYPE::LATENCY_FLOW) {
            worker = std::thread([this] {this->latency_flow(); });
        }
        auto c = cv::waitKey(0);
        if (c == 27) {
            lunch_status = false;
            if (worker.joinable()) {
                worker.detach();
            }
        }
    }
    return 0;
}

void GraphProcessor::s_mouse_callback(int event, int x, int y, int flags, void* param) noexcept {    /*param - image*/
    auto graph_processor = static_cast<GraphProcessor*>(param);
    if (event == cv::EVENT_LBUTTONDOWN) {
        graph_processor->process_realtime(x, y, false);
    }
    if (event == cv::EVENT_MOUSEMOVE) {
        if (graph_processor->m_floating_node == FLOATING_MOUSE_NODE::ON) {
            graph_processor->process_realtime(x, y, true);
        }
        else if (graph_processor->m_floating_node == FLOATING_MOUSE_NODE::NEAREST_NODE) {
            graph_processor->connect_nearest(x, y);
        }
    }
    if (graph_processor->m_floating_node == FLOATING_MOUSE_NODE::NEAREST_NODE) {
#ifdef __linux__ 
        if (event == cv::EVENT_MOUSEHWHEEL) {
            if (cv::getMouseWheelDelta(flags) > 0) {
                graph_processor->change_connectivity(0);
            }
            else {
                graph_processor->change_connectivity(1);
            }
        }
#elif _WIN32
        if (event == cv::EVENT_MOUSEWHEEL) {
            if (cv::getMouseWheelDelta(flags) > 0) {
                graph_processor->change_connectivity(0);
            }
            else {
                graph_processor->change_connectivity(1);
            }
        }
#endif
    }
}

[[noreturn]] void GraphProcessor::latency_flow() noexcept {
    std::random_device rand_dev;
    std::mt19937 gen(rand_dev());
    std::uniform_int_distribution<> rand_rows(0, m_img_columns);
    std::uniform_int_distribution<> rand_cols(0, m_img_rows);
    int i = 0;
    double total_time = 0;
    while (true) {
        auto pair = std::make_pair(rand_rows(gen), rand_cols(gen));
        if (contains(pair.first, pair.second)) {
            return;
        }
        calculate_graph(pair);
    }
}

void GraphProcessor::process_realtime(const int x, const int y, const bool mouse_call) noexcept {
    calculate_graph(std::make_pair(x, y));
    if (mouse_call) {
        for (int i = 0; i < matrix.size() - 1; i++) {
            matrix[i].pop_back();
        }
        matrix.pop_back();
        nodes.pop_back();
    }
}

void GraphProcessor::calculate_graph(std::pair<int, int> pair) noexcept {
    nodes.push_back(pair);
    calculate_distances();
    connect_MST();
}

void GraphProcessor::create_circles() noexcept {
    for (const auto& [x, y] : nodes) {
        cv::circle(m_image, cv::Point(x, y), 3, cv::Scalar(120, 250, 120), -1, cv::LINE_AA);
    }
    cv::circle(m_image, cv::Point(nodes.back().first, nodes.back().second), 3, cv::Scalar(0, 0, 250), -1, cv::LINE_AA);
    cv::circle(m_image, cv::Point(nodes.front().first, nodes.front().second), 3, cv::Scalar(250, 0, 250), -1, cv::LINE_AA);
}

int GraphProcessor::min_key(const std::vector<int>& key, const std::vector<bool>& mst_set) noexcept {
    int min = INT_MAX;
    int min_index = 0;
    for (int v = 0; v < nodes.size(); v++) {
        if (!mst_set[v] && key[v] < min) {
            min = key[v];
            min_index = v;
        }
    }
    return min_index;
}

void GraphProcessor::connect_MST() noexcept {
    refresh_img();
    std::vector<int> parent(nodes.size());
    std::vector<int> key(nodes.size(), INT_MAX);
    std::vector<bool> mst_set(nodes.size(), false);
    key[0] = 0;
    parent[0] = -1;
    for (int count = 0; count < nodes.size() - 1; count++) {
        int u = min_key(key, mst_set);
        mst_set[u] = true;
        for (int v = 0; v < nodes.size(); v++) {
            if (matrix[u][v] && !mst_set[v] && matrix[u][v] < key[v]) {
                parent[v] = u;
                key[v] = matrix[u][v];
            }
        }
    }
    draw_graph(parent);
}

void GraphProcessor::calculate_distances() noexcept {
    if (nodes.size() == 1) {
        matrix.push_back({ 0 });
        return;
    }
    std::vector<double> row;
    row.reserve(std::sqrt(nodes.size()));
    for (int i = 0; i < nodes.size() - 1; ++i) {
        auto distance = std::sqrt(std::pow(nodes[nodes.size() - 1].first - nodes[i].first, 2) + std::pow(nodes[nodes.size() - 1].second - nodes[i].second, 2));
        matrix[i].push_back(distance);
    }
    for (int i = 0; i < nodes.size(); ++i) {
        auto distance = std::sqrt(std::pow(nodes[nodes.size() - 1].first - nodes[i].first, 2) + std::pow(nodes[nodes.size() - 1].second - nodes[i].second, 2));
        row.push_back(distance);
    }
    matrix.push_back(row);
}

void GraphProcessor::connect_nearest(const int x, const int y) noexcept {
    if (nodes.size() == 0) {
        return;
    }
    cv::Mat image = m_image.clone();
    if (m_cnt_connections == 1) {
        std::vector<node_t>::const_iterator element = nodes.cbegin();
        double min_distance = INT_MAX;
        for (auto node = nodes.cbegin(); node != nodes.cend(); ++node) {
            double distance_to_node = std::sqrt(std::pow(node->first - x, 2) + std::pow(node->second - y, 2));
            if (min_distance > distance_to_node) {
                min_distance = distance_to_node;
                element = node;
            }
        }
        create_line(image, cv::Point(element->first, element->second), cv::Point(x, y));
    }
    else {
        std::unordered_map<std::pair< std::pair<int, int>, std::pair<int, int>>, distance_t, KeyHasherPair<dots_pair_t>> nearest_dots;
        for (const auto& [cx, cy] : nodes) {
            double min_distance = INT_MAX;
            double distance = std::sqrt(std::pow(cx - x, 2) + std::pow(cy - y, 2));
            if (min_distance > distance) {
                min_distance = distance;
            }
            if (m_cnt_connections > nearest_dots.size()) {
                auto p1 = std::make_pair(cx, cy);
                auto p2 = std::make_pair(x, y);
                nearest_dots.emplace(std::make_pair(p1, p2), distance);
            }
            else {
                auto max_remouted_pair = find_max_distance(nearest_dots);
                if (max_remouted_pair->second > min_distance) {
                    auto p1 = std::make_pair(cx, cy);
                    auto p2 = std::make_pair(x, y);
                    nearest_dots.erase(max_remouted_pair);
                    nearest_dots.emplace(std::make_pair(p1, p2), distance);
                }
            }
        }
        for (const auto& dot : nearest_dots) {
            create_line(image, cv::Point(dot.first.first.first, dot.first.first.second), cv::Point(x, y));
        }
    }
    cv::imshow(m_window_name, image);
}

void GraphProcessor::static_process() noexcept {
    nodes = { std::make_pair(50,20),
            std::make_pair(60,30),
            std::make_pair(50,30),
            std::make_pair(70,40),
            std::make_pair(80,10)
    };
    calculate_distances();
    connect_MST();
}

bool GraphProcessor::contains(const int x, const int y) noexcept {
    for (const auto& [cx, cy] : nodes) {
        if (cx == x && cy == y || cx == y && cy == x) {
            return true;
        }
    }
    return false;
}

void GraphProcessor::change_connectivity(const bool distination) noexcept {
    if (distination) {
        if (nodes.size() > m_cnt_connections) {
            m_cnt_connections++;
        }
    }
    else {
        if (m_cnt_connections > 1) {
            m_cnt_connections--;
        }
    }
}

void GraphProcessor::draw_graph(const std::vector<int>& parent) noexcept {
    for (int i = 1; i < nodes.size(); i++) {
        create_line(m_image, cv::Point(nodes[i].first, nodes[i].second), cv::Point(nodes[parent[i]].first, nodes[parent[i]].second));
    }
    cv::imshow(m_window_name, m_image);
}

void GraphProcessor::create_line(const cv::Mat& image, const cv::Point&& start, const cv::Point&& end) noexcept {
    cv::line(image, start, end, cv::Scalar(80, 80, 80), 2, cv::LINE_4);
    create_circles();
}

void GraphProcessor::refresh_img() noexcept {
    m_image = cv::Mat(m_img_rows, m_img_columns, CV_8UC3, cv::Scalar(0, 0, 0));
}

void GraphProcessor::print_statistics() noexcept {
    printf("==== Statistics ====\n");
    printf("Number of nodes: %d\n\
            Matrix size: %d %d\n",
        nodes.size(),
        matrix.size(), matrix[0].size());
}

total_distances_t::const_iterator GraphProcessor::find_max_distance(const total_distances_t& container) noexcept {
    double max_dist = 0;
    total_distances_t::const_iterator ret_iterator = container.begin();
    for (auto i = container.begin(); i != container.end(); ++i) {
        if (i->second > max_dist) {
            max_dist = i->second;
            ret_iterator = i;
        }
    }
    return ret_iterator;
}