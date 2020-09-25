#ifndef GRAPHPROCESSOR_H_
#define GRAPHPROCESSOR_H_

#include <opencv2/core/mat.hpp>

#include <functional>
#include <memory>


class GraphProcessor {
public:
	GraphProcessor(const int weight = 320, const int height = 240, const std::string imageName = "image") noexcept;
	~GraphProcessor() noexcept;
	static void smMouseCallback(int event, int x, int y, int flags, void* param) noexcept;
	int lunch() noexcept;
private:
	std::unique_ptr<cv::Mat> mpImage;
	std::string mImgWindowName;
	const int mImgRows;
	const int mImgCols;
};


#endif