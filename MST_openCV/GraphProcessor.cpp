#include "GraphProcessor.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <memory>
#include <utility>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

GraphProcessor::GraphProcessor(const int weight, const int height, const std::string imageName) noexcept:
        mImgRows(weight),
        mImgCols(height),
        mImgWindowName(imageName)
        //mDistances(std::make_shared<std::unordered_map<NodeCoords, distance>>())
        //mpImage(std::move(std::make_unique<cv::Mat>(mImgRows, mImgCols, CV_8UC3, cv::Scalar(0, 0, 0))))//idk why but it doesn't work
{
    mpImage =cv::Mat(mImgRows, mImgCols, CV_8UC3, cv::Scalar(0, 0, 0));
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
    if(nodes.size() > 1) {
        connectMST();
    }
    printNodes();
}

void GraphProcessor::connectMST() noexcept {
    for (const auto& nodeA : nodes) {
        for (const auto& nodeB : nodes) {
            if (nodeA.first == nodeB.second && nodeA.second == nodeB.first) {
                continue;
            }
            auto distanceAB = std::sqrt(std::pow(nodeB.first-nodeA.first, 2) + std::pow(nodeB.second - nodeA.second, 2));
            //mDistances[]
        }
    }
}

int GraphProcessor::lunch() noexcept {
    auto lunch_status{ true };
    cv::namedWindow(mImgWindowName, cv::WINDOW_AUTOSIZE);
    while (lunch_status) {
        cv::imshow(mImgWindowName, mpImage);
        cv::setMouseCallback(mImgWindowName, smMouseCallback, this);    // 3rd param passes to callback
        auto c = cv::waitKey(0);
        if (c == 27) {
            lunch_status = false;
        }
    }
    return 0;
}

void GraphProcessor::printNodes() noexcept {
    printf("count of nodes: %I64u\n", nodes.size());
    for (const auto& i : nodes) {
        printf("pair: %d %d\n", i.first, i.second);
    }
}