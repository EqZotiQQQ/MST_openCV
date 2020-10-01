#include "../headers/GraphProcessor.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <utility>
#include <thread>
#include <random>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


using distance_t            = double;
using dot_t                 = std::pair<int, int>;
using dots_pair_t           = std::pair<dot_t, dot_t>;
using total_distances_t     = std::unordered_map<dots_pair_t, distance_t, KeyHasherPair<dots_pair_t>>;
using nodes_t               = std::vector<dot_t>;


GraphProcessor::GraphProcessor(const int rows, const int columns, const std::string& image_name) noexcept :
    m_img_rows(rows),
    m_img_columns(columns),
    m_window_name(image_name),
    m_cnt_connections(1)
{
    m_image = cv::Mat(m_img_rows, m_img_columns, CV_8UC3, cv::Scalar(0, 0, 0));
    printf("Image size: [%d %d]\n", m_image.rows, m_image.cols);
    printf("Press esc button to exit.\n");
    printf("scroll up/down to increase/reduce number of connections\n");


}

GraphProcessor::~GraphProcessor() noexcept {
}

void GraphProcessor::set_options(const FLOATING_MOUSE_NODE mouse, const RUN_TYPE run_type) noexcept {
    m_floating_node = mouse;
    m_run_type = run_type;
    printf("set with opt\n");
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
            if (cv::getMouseWheelDelta(flags) > 0) {        /*contains bug in ubuntu*/
                graph_processor->change_connectivity(0);
            }
            else {
                graph_processor->change_connectivity(1);
            }
        }
#endif
    }
}

void GraphProcessor::connect_nearest(const int x, const int y) noexcept {
    if (m_all_nodes.size() == 0) {
        return;
    }
    cv::Mat image = m_image.clone();
    if (m_cnt_connections == 1) {
        nodes_t::const_iterator element = m_all_nodes.cbegin();
        double min_distance = -1;
        for (auto node = m_all_nodes.cbegin(); node != m_all_nodes.cend(); ++node) {
            double distance_to_node = std::sqrt(std::pow(node->first - x, 2) + std::pow(node->second - y, 2));
            if (min_distance > distance_to_node || min_distance) {
                min_distance = distance_to_node;
                element = node;
            }
        }
        create_line(image, cv::Point(element->first, element->second), cv::Point(x, y));
    }
    else {
        total_distances_t nearest_dots;
        for (const auto& node : m_all_nodes) {
            double min_distance = 0;
            double distance = std::sqrt(std::pow(node.first - x, 2) + std::pow(node.second - y, 2));
            if (min_distance == 0 || min_distance > distance) {
                min_distance = distance;
            }
            if (m_cnt_connections > nearest_dots.size()) {
                nearest_dots.emplace(std::make_pair(std::make_pair(node.first, node.second), std::make_pair(x, y)), distance);
            }
            else {
                auto max_remouted_pair = find_max_distance(nearest_dots);
                if (max_remouted_pair->second > min_distance) {
                    nearest_dots.erase(max_remouted_pair);
                    nearest_dots.emplace(std::make_pair(std::make_pair(node.first, node.second), std::make_pair(x, y)), distance);
                }
            }
        }
        for (const auto& dot : nearest_dots) {
            create_line(image, cv::Point(dot.first.first.first, dot.first.first.second), cv::Point(x, y));
        }
    }
    cv::imshow(m_window_name, image);
}

[[noreturn]] void GraphProcessor::latency_flow() noexcept {

    //using clock_t = std::chrono::high_resolution_clock;
    //using second_t = std::chrono::duration<double, std::ratio<1> >;

    std::random_device rand_dev;
    std::mt19937 gen(rand_dev());
    std::uniform_int_distribution<> rand_rows(0, m_img_columns);
    std::uniform_int_distribution<> rand_cols(0, m_img_rows);
    //int i = 0;
    //double total_time = 0;
    while (true) {
        //std::chrono::time_point<clock_t> beg = clock_t::now();
        clean_entries();
        auto pair = std::make_pair(rand_rows(gen), rand_cols(gen));
        if (contains(pair.first, pair.second)) {
            continue;
        }
        m_all_nodes.push_back(pair);
        calculate_distances();
        connect_MST();
        std::this_thread::sleep_for(std::chrono::milliseconds(0)); //strange bug. doens't connect part of nodes.
        //printf("debug? %d\n", i++);
        //printf("time = %f; total time = %f\n", std::chrono::duration_cast<second_t>(clock_t::now() - beg).count(),
        //        total_time += std::chrono::duration_cast<second_t>(clock_t::now() - beg).count());
    }
}

void GraphProcessor::clean_entries() noexcept {
    m_image = cv::Mat(m_img_rows, m_img_columns, CV_8UC3, cv::Scalar(0, 0, 0)); //prepare image for next tree.
    m_connected_nodes.clear();
    m_connected_nodes.reserve(m_all_nodes.size());
    m_not_connected_nodes.clear();
    m_not_connected_nodes.reserve(m_all_nodes.size());
}

void GraphProcessor::calculate_distances() noexcept {
    create_circles();
    //for (const auto& node : m_all_nodes) {
    for (auto node = m_all_nodes.cbegin(); node != m_all_nodes.cend(); ++node) {
        m_not_connected_nodes.emplace_back(*node);
    }
    if (m_all_nodes.size() == 1) {
        return;
    }
    auto last_node = m_all_nodes.cend() - 1;
    for (auto node = m_all_nodes.cbegin(); node != m_all_nodes.cend() - 1; ++node) {
        auto distance = std::sqrt(std::pow(last_node->first - node->first, 2) + std::pow(last_node->second - node->second, 2));
        m_distances.emplace(std::make_pair(std::make_pair(last_node->first, last_node->second), std::make_pair(node->first, node->second)), distance);
    }
}

void GraphProcessor::process_realtime(const int x, const int y, const bool mouse_call) noexcept {
    clean_entries();
    m_all_nodes.emplace_back(std::make_pair(x, y));
    calculate_distances();
    connect_MST();
    if (mouse_call) {
        m_all_nodes.pop_back();
    }
}

bool GraphProcessor::contains(const int x, const int y) noexcept {
    for (const auto& item : m_all_nodes) {
        if (item.first == x and item.second == y or
            item.first == y and item.second == x) {
            return true;
        }
    }
    return false;
}

void GraphProcessor::connect_MST() noexcept {
    if (m_all_nodes.size() < 2) {
        m_connected_nodes.push_back(*(m_not_connected_nodes.begin()));
        m_not_connected_nodes.erase(m_not_connected_nodes.begin());
        return;
    }
    double max_dist = find_max_distance(m_distances)->second;
    while (m_not_connected_nodes.size() > 0) {
        if (m_connected_nodes.size() == 0) {
            m_connected_nodes.push_back(*(m_not_connected_nodes.begin()));
            m_not_connected_nodes.erase(m_not_connected_nodes.begin());
        }
        else {
            nodes_t::const_iterator connected_node;
            nodes_t::const_iterator not_connected_node;
            double min_distance = max_dist;
            for (auto connected = m_connected_nodes.cbegin(); connected != m_connected_nodes.cend(); ++connected) {
                for (auto not_connected = m_not_connected_nodes.cbegin(); not_connected != m_not_connected_nodes.cend(); ++not_connected) {
                    total_distances_t::const_iterator current_pair_distance;
                    if (m_distances.find(std::make_pair(*connected, *not_connected)) != m_distances.end()) {
                        current_pair_distance = m_distances.find(std::make_pair(*connected, *not_connected));
                    }
                    else {
                        current_pair_distance = m_distances.find(std::make_pair(*not_connected, *connected));
                    }
                    if (min_distance >= current_pair_distance->second) {
                        min_distance = current_pair_distance->second;
                        connected_node = connected;
                        not_connected_node = not_connected;
                    }
                }
            }
            create_line(m_image,
                cv::Point(connected_node->first, connected_node->second),
                cv::Point(not_connected_node->first, not_connected_node->second));
            m_connected_nodes.push_back(*not_connected_node);
            m_not_connected_nodes.erase(not_connected_node);
        }
    }
    cv::imshow(m_window_name, m_image);
}

void GraphProcessor::print_connected() noexcept {
    printf("contain %d m_connected_nodes", m_connected_nodes.size());
    for (const auto& item : m_connected_nodes) {
        printf("Node: [%d %d]\n", item.second, item.first);
    }
}
void GraphProcessor::print_not_connected() noexcept {
    printf("contain %d m_not_connected_nodes", m_not_connected_nodes.size());
    for (const auto& item : m_not_connected_nodes) {
        printf("Node: [%d %d]\n", item.second, item.first);
    }
}
void GraphProcessor::print_all_nodes() noexcept {
    printf("contain %d m_all_nodes", m_distances.size());
    for (const auto& item : m_all_nodes) {
        printf("Node: [%d %d]\n", item.second, item.first);
    }
}
void GraphProcessor::print_distances() noexcept {
    printf("contain %d distances", m_distances.size());
    for (const auto& item : m_distances) {
        printf("Pair: [%d %d] and [%d %d] distance = %f\n", item.first.first.first, item.first.first.second, item.first.second.first, item.first.second.second, item.second);
    }
}

void GraphProcessor::create_line(const cv::Mat & image, const cv::Point && start, const cv::Point && end) noexcept {
    cv::line(image, start, end, cv::Scalar(80, 80, 80), 2, cv::LINE_4);
}

void GraphProcessor::create_circles() noexcept {
    for (const auto& i : m_all_nodes) {
        cv::circle(m_image, cv::Point(i.first, i.second), 3, cv::Scalar(120, 250, 120), -1, cv::LINE_AA);
    }
    cv::circle(m_image, cv::Point(m_all_nodes.back().first, m_all_nodes.back().second), 3, cv::Scalar(0, 0, 250), -1, cv::LINE_AA);

}

total_distances_t::const_iterator GraphProcessor::find_max_distance(const total_distances_t & container) noexcept {
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

void GraphProcessor::print_data() noexcept {
    printf("Statistics\n");
    printf("Count of nodes: %I64lu\n", m_all_nodes.size());
    printf("List of nodes:\n");
    for (const auto& i : m_all_nodes) {
        printf("[%d %d]\t", i.first, i.second);
    }
    std::cout << "\n====\nNodes and distances between nodes:" << std::endl;
    if (m_distances.size() < 1) {
        printf("NaN\n");
    }
    int j = 0;
    for (const auto& i : m_distances) {
        printf("%d [%d %d\t %d %d\t\t %f]\n",
            j++,
            i.first.first.first,
            i.first.first.second,
            i.first.second.first,
            i.first.second.second,
            i.second);
    }
}


void GraphProcessor::static_process() noexcept {
    m_all_nodes = { std::make_pair(50,20),
            std::make_pair(60,30),
            std::make_pair(50,30),
            std::make_pair(70,40),
            std::make_pair(80,10)
    };
    m_connected_nodes.clear();
    m_connected_nodes.reserve(m_all_nodes.size());
    m_not_connected_nodes.clear();
    m_not_connected_nodes.reserve(m_all_nodes.size());
    calculate_distances();
    connect_MST();
}

void GraphProcessor::change_connectivity(const bool distination) noexcept {
    if (distination) {
        if (m_all_nodes.size() > m_cnt_connections) {
            m_cnt_connections++;
        }
    }
    else {
        if (m_cnt_connections > 1) {
            m_cnt_connections--;
        }
    }
}