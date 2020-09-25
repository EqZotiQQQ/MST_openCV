#include "GraphProcessor.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

GraphProcessor::GraphProcessor(const int weight, const int height, const std::string imageName) noexcept :
	mImgRows(weight),
	mImgCols(height),
	mImgWindowName(imageName)
	//mpImage(std::move(std::make_unique<cv::Mat>(mImgRows, mImgCols, CV_8UC3, cv::Scalar(0, 0, 0))))//idk why but it doesn't work
{
	mpImage = std::make_unique<cv::Mat>(mImgRows, mImgCols, CV_8UC3, cv::Scalar(0, 0, 0));
	std::cout << "created" << std::endl;
	printf("rows: %d x cols: %d\n", mpImage->rows, mpImage->cols);
}

GraphProcessor::~GraphProcessor() noexcept {
	std::cout << "destroyed" << std::endl;
}

void GraphProcessor::smMouseCallback(int event, int x, int y, int flags, void* param) noexcept {
	printf("event! x: %d; y: %d\n", x, y);
}

int GraphProcessor::lunch() noexcept {
	auto lunch_status{ true };
	cv::namedWindow(mImgWindowName, cv::WINDOW_AUTOSIZE);
	while (lunch_status) {
		cv::imshow(mImgWindowName, *mpImage);
		cv::setMouseCallback(mImgWindowName, smMouseCallback, &*mpImage);
		std::cout << "fuck" << std::endl;
		auto c = cv::waitKey(0);
		if (c == 27) {
			lunch_status = false;
		}
	}
	return 0;
}
