#include "../headers/GraphProcessor.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <utility>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>


using distance_t            = double;
using dot_t                 = std::pair<int, int>;
using dots_pair_t           = std::pair<dot_t, dot_t>;
using total_distances_t     = std::unordered_map<dots_pair_t, distance_t, KeyHasherPair<dots_pair_t>>;
using nodes_t               = std::vector<dot_t>;


GraphProcessor::GraphProcessor(const int rows, const int columns, const std::string image_name) noexcept:
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

int GraphProcessor::launch() noexcept {
    auto lunch_status{ true };
    cv::namedWindow(m_window_name, cv::WINDOW_AUTOSIZE);
    while (lunch_status) {
        cv::imshow(m_window_name, m_image);
        if (DEBUG == true) {
            this->static_process();
        }
        cv::setMouseCallback(m_window_name, s_mouse_callback, this);

        auto c = cv::waitKey(0);
        if (c == 27) {
            lunch_status = false;
        }
    }
    return 0;
}

void callback_button(int state, void* userdata) {
    std::cout << "okay" << std::endl;
}

void GraphProcessor::s_mouse_callback(int event, int x, int y, int flags, void* param) noexcept {    /*param - image*/
    auto graph_processor = static_cast<GraphProcessor*>(param);
    if (event == cv::EVENT_LBUTTONDOWN) {
        graph_processor->process_realtime(x, y);
    }
    if (event == cv::EVENT_MOUSEMOVE) {
        graph_processor->connect_nearest(x, y);
    }

    /* For some reason it works for ubuntu 20.04 and doesn't work for windows.
     * Works like: leave cursor on position and just scroll mousewheel up/down.
     * For windows works EVENT_MOUSEWHEEL*/
    if (event == cv::EVENT_MOUSEHWHEEL) {
        if (cv::getMouseWheelDelta(flags) > 0) {
            graph_processor->change_connectivity(0);
        } else {
            graph_processor->change_connectivity(1);
        }
    }
    if (event == cv::EVENT_MOUSEWHEEL) {
        printf("getMouseWheelDelta: %d\n", cv::getMouseWheelDelta(flags));
        if (cv::getMouseWheelDelta(flags) > 0) {        /*contains bug in ubuntu*/
            graph_processor->change_connectivity(0);
        } else {
            graph_processor->change_connectivity(1);
        }
    }
}

void GraphProcessor::process_mouse_moving(const int x, const int y) noexcept {

}



void GraphProcessor::connect_nearest(const int x, const int y) noexcept {
    if(m_all_nodes.size() == 0) {
        return;
    }
    cv::Mat image = m_image.clone();
    total_distances_t nearest_dots;
    for (const auto& node : m_all_nodes) {
        double min_distance = 0;
        auto distance = std::sqrt(std::pow(node.first - x, 2) + std::pow(node.second - y, 2));
        if (min_distance == 0 || min_distance > distance) {
            min_distance = distance;
        }
        if (m_cnt_connections > nearest_dots.size()) {
            nearest_dots.emplace(std::make_pair(std::make_pair(node.first, node.second), std::make_pair(x, y)), distance);
        } else {
            auto max_remouted_pair = find_max_distance(nearest_dots);
            if(max_remouted_pair->second > min_distance) {
                nearest_dots.erase(max_remouted_pair);
                nearest_dots.emplace(std::make_pair(std::make_pair(node.first, node.second), std::make_pair(x, y)), distance);
            }
        }
    }
    for (const auto& dot : nearest_dots) {
        create_line(image, cv::Point(dot.first.first.first, dot.first.first.second), cv::Point(x, y));
    }
    cv::imshow(m_window_name, image);
}


void GraphProcessor::process_realtime(const int x, const int y) noexcept {
    clean_entries();
    m_all_nodes.emplace_back(std::make_pair(x, y));
    calculate_distances();
    connect_MST();
}

void GraphProcessor::clean_entries() noexcept {
    m_connected_nodes.clear();
    m_connected_nodes.reserve(m_all_nodes.size());
    m_not_connected_nodes.clear();
    m_not_connected_nodes.reserve(m_all_nodes.size());
}

void GraphProcessor::calculate_distances() noexcept {
    for (const auto& node : m_all_nodes) {
        m_not_connected_nodes.push_back(node);
    }
    if (m_all_nodes.size() > 1) {
        for (const auto& node_a : m_all_nodes) {
            for (const auto& node_b : m_all_nodes) {
                auto f = m_distances.find(std::make_pair(std::make_pair(node_b.first, node_b.second), std::make_pair(node_a.first, node_a.second)));//.contains in c++20
                if (node_a.first == node_b.second && node_a.second == node_b.first
                    || f != m_distances.end()
                    || node_a.first == node_b.first && node_a.second == node_b.second) {
                    continue;
                }
                auto distance = std::sqrt(std::pow(node_b.first - node_a.first, 2) + std::pow(node_b.second - node_a.second, 2));
                m_distances.emplace(std::make_pair(std::make_pair(node_a.first, node_a.second), std::make_pair(node_b.first, node_b.second)), distance);
            }
        }
    }
}

void GraphProcessor::connect_MST() noexcept {
    m_image = cv::Mat(m_img_rows, m_img_columns, CV_8UC3, cv::Scalar(0, 0, 0));
    create_circles();
    if(m_all_nodes.size() < 2) {
        cv::imshow(m_window_name, m_image);
        return;
    }
    double max_dist = find_max_distance(m_distances)->second;
    while (m_not_connected_nodes.size() > 0) {
        if (m_connected_nodes.size() == 0) {
            m_connected_nodes.push_back(*(m_not_connected_nodes.begin()));
            m_not_connected_nodes.erase(m_not_connected_nodes.begin());
        } else {
            nodes_t::const_iterator connected_node;
            nodes_t::const_iterator not_connected_node;
            double min_distance = max_dist;
            for (auto connected = m_connected_nodes.cbegin(); connected != m_connected_nodes.cend(); ++connected) {
                for (auto not_connected = m_not_connected_nodes.cbegin(); not_connected != m_not_connected_nodes.cend(); ++not_connected) {
                    total_distances_t::const_iterator current_pair_distance;
                    if (m_distances.find(std::make_pair(*connected, *not_connected)) != m_distances.end()) {
                        current_pair_distance = m_distances.find(std::make_pair(*connected, *not_connected));
                    } else {
                        current_pair_distance = m_distances.find(std::make_pair(*not_connected, *connected));
                    }
                    if (min_distance >= current_pair_distance->second) {
                        min_distance = current_pair_distance->second;
                        connected_node = connected;
                        not_connected_node = not_connected;
                    }
                }
            }
            create_line(m_image, cv::Point(connected_node->first, connected_node->second), cv::Point(not_connected_node->first, not_connected_node->second));
            m_connected_nodes.push_back(*not_connected_node);
            m_not_connected_nodes.erase(not_connected_node);
        }
        cv::imshow(m_window_name, m_image);
    }
}

void GraphProcessor::create_line(const cv::Mat& image, const cv::Point&& start, const cv::Point&& end) noexcept {
    cv::line(image, start, end, cv::Scalar(80, 80, 80), 2, cv::LINE_4);
}

void GraphProcessor::create_circles() noexcept {
    for (const auto& i : m_all_nodes) {
        cv::circle(m_image, cv::Point(i.first, i.second), 3, cv::Scalar(120, 250, 120), -1, cv::LINE_AA);
    }
}

total_distances_t::const_iterator GraphProcessor::find_max_distance(const total_distances_t& container) noexcept {
    return std::max_element
    (
        std::begin(container), std::end(container),
        [](const auto & lhs, const auto & rhs) {
            return lhs.second < rhs.second;
        }
    );
}



void GraphProcessor::print_data() noexcept {
    printf("Statistics\n");
    printf("Count of nodes: %I64u\n", m_all_nodes.size());
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