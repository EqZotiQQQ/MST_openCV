#include "GraphProcessor.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <memory>
#include <utility>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>


/*
TODO:
remove camel case
create more awesome names
remove or create single place with printing info about tree
add drawing
*/
using distance_t            = double;
using dot_t                 = std::pair<int, int>;
using dotsPair_t            = std::pair<dot_t, dot_t>;
using totalDistances_t      = std::unordered_map<dotsPair_t, distance_t, KeyHasherPair<dotsPair_t>>;
using nodes_t               = std::vector<std::pair<int, int>>;

GraphProcessor::GraphProcessor(const int weight, const int height, const std::string imageName) noexcept:
        mImgRows(weight),
        mImgCols(height),
        mImgWindowName(imageName)
        //mDistances(std::make_shared<std::unordered_map<NodeCoords, distance>>())
        //mpImage(std::move(std::make_unique<cv::Mat>(mImgRows, mImgCols, CV_8UC3, cv::Scalar(0, 0, 0))))//idk why but it doesn't work
{
    mpImage = cv::Mat(mImgRows, mImgCols, CV_8UC3, cv::Scalar(0, 0, 0));
    printf("Picture size: %d X %d\n", mpImage.rows, mpImage.cols);
}

GraphProcessor::~GraphProcessor() noexcept {
    std::cout << "destroyed" << std::endl;
}

void GraphProcessor::smMouseCallback(int event, int x, int y, int flags, void* param) noexcept {    /*param - image*/
    //printf("event! x: %d; y: %d\n", x, y);
    auto graphProcesser = static_cast<GraphProcessor*>(param);
    if (event == cv::EVENT_LBUTTONDOWN) {
        graphProcesser->process(x, y);
    }
}


//TODO: 1. calculate distances between nodes; 2. connect it.
void GraphProcessor::process(const int x, const int y) noexcept {
    tree.clear();
    tree.reserve(nodes.size());
    freeNodes.clear();
    freeNodes.reserve(nodes.size());
    nodes.emplace_back(std::make_pair(x, y));
    calculateDistances();
    connectMST();
    printNodes();
    std::cout << "-------------------------------" << std::endl;
}

void GraphProcessor::staticProcess() noexcept {
    nodes = {std::make_pair(50,20),
            std::make_pair(60,30),
            std::make_pair(50,30),
            std::make_pair(70,40),
            std::make_pair(80,10)
    };
    tree.clear();
    tree.reserve(nodes.size());
    freeNodes.clear();
    freeNodes.reserve(nodes.size());
    calculateDistances();
    connectMST();
    printNodes();
}

void GraphProcessor::connectMST() noexcept {
    auto maxDist = findBiggestDistance();
    while (freeNodes.size() > 0) {
        if (tree.size() == 0) {
            tree.push_back(*(freeNodes.begin()));
            freeNodes.erase(freeNodes.begin());
        } else {
            nodes_t::iterator iter = freeNodes.begin();
            //matchClosestPair(iter, maxDist->second);
            nodes_t::const_iterator connected_node;
            nodes_t::const_iterator not_connected_node;
            double min_distance = maxDist->second;
            for (auto connected = tree.cbegin(); connected != tree.cend(); ++connected) {
                for (auto not_connected = freeNodes.cbegin(); not_connected != freeNodes.cend(); ++not_connected) {
                    double current_pair_distance = mDistances.find(std::make_pair(*connected, *not_connected))->second;
                    if (min_distance >= current_pair_distance) {
                        min_distance = current_pair_distance;
                        connected_node = connected;
                        not_connected_node = not_connected;
                    }
                }
            }
            printf("connected on step: [%d %d\t%d %d\t%f]", connected_node->first, connected_node->second, not_connected_node->first, not_connected_node->second, min_distance);
            tree.push_back(*not_connected_node);
            freeNodes.erase(not_connected_node);
        }

        printf("\n===\tTree contains:\t");
        for (const auto& i : tree) {
            printf("[%d %d]\t", i.first, i.second);
        }
        printf("\n===\tTree doesnt contain:\t");
        if (freeNodes.size() == 0) {
            printf("NaN");
        }
        for (const auto& i : freeNodes) {
            printf("[%d %d]\t", i.first, i.second);
        }
        printf("\n===check===\n");
    }
}

void GraphProcessor::matchClosestPair(nodes_t::iterator& iter, int maxDist) noexcept {

}

totalDistances_t::iterator GraphProcessor::findBiggestDistance() noexcept {
    return std::max_element
    (
        std::begin(mDistances), std::end(mDistances),
        [](const auto & lhs, const auto & rhs) {
            return lhs.second < rhs.second;
        }
    );
}

int GraphProcessor::lunch() noexcept {
    auto lunch_status{ true };
    cv::namedWindow(mImgWindowName, cv::WINDOW_AUTOSIZE);
    while (lunch_status) {
        cv::imshow(mImgWindowName, mpImage);
        //this->staticProcess();
        cv::setMouseCallback(mImgWindowName, smMouseCallback, this);    // 3rd param passes to callback
        auto c = cv::waitKey(0);
        if (c == 27) {
            lunch_status = false;
        }
    }
    return 0;
}

void GraphProcessor::printNodes() noexcept {
    printf("Statistics\n");
    printf("Count of nodes: %I64u\n", nodes.size());
    printf("List of nodes:\n");
    for (const auto& i : nodes) {
        printf("[%d %d]\t", i.first, i.second);
    }
    std::cout << "\n====\nNodes and distances between nodes:" << std::endl;
    if (mDistances.size() < 1) {
        printf("NaN\n");
    }
    int j = 0;
    for (const auto& i : mDistances) {
        printf("%d [%d %d\t %d %d\t\t %f]\n",
            j++,
            i.first.first.first,
            i.first.first.second,
            i.first.second.first,
            i.first.second.second,
            i.second);
    }
    std::cout << "====" << std::endl;
}


void GraphProcessor::calculateDistances() noexcept {
    for (const auto& nodeA : nodes) {
        freeNodes.push_back(nodeA);
    }
    /*for (auto first_iter = freeNodes.cbegin(); first_iter != freeNodes.cend(); ++first_iter) {
        for (auto second_iter = freeNodes.cbegin(); second_iter != freeNodes.cend(); ++second_iter) {
            auto f = mDistances.find(std::make_pair(std::make_pair(second_iter->first, second_iter->second), std::make_pair(first_iter->first, first_iter->second)));
            if (first_iter == second_iter ||
                    first_iter->first  == second_iter->second && 
                    first_iter->second == second_iter->first) {
                continue;
            }
            auto distanceAB = std::sqrt(std::pow(second_iter->first - first_iter->first, 2) + std::pow(second_iter->second - first_iter->second, 2));
            mDistances.emplace(
                std::make_pair(
                    std::make_pair(first_iter->first, first_iter->second), 
                    std::make_pair(second_iter->first, second_iter->second)
                ), distanceAB);
        }
    }*/
    for (const auto& nodeA : freeNodes) {
        for (const auto& nodeB : freeNodes) {
            auto f = mDistances.find(std::make_pair(std::make_pair(nodeB.first, nodeB.second), std::make_pair(nodeA.first, nodeA.second)));//.contains in c++20
            if (nodeA.first == nodeB.second && nodeA.second == nodeB.first || f != mDistances.end() || nodeA.first == nodeB.first && nodeA.second == nodeB.second) {
                continue;
            }
            auto distanceAB = std::sqrt(std::pow(nodeB.first - nodeA.first, 2) + std::pow(nodeB.second - nodeA.second, 2));
            mDistances.emplace(std::make_pair(std::make_pair(nodeA.first, nodeA.second), std::make_pair(nodeB.first, nodeB.second)), distanceAB);
        }
    }
    printNodes();
}