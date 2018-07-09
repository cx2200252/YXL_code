#pragma once
#include <string>
#include "webp/webp/encode.h"
#include <opencv2/opencv.hpp>

namespace YXL
{
	namespace WebP
	{
		bool Convert2Webp(const std::string& src_path, const std::string& dest_path);
		bool Convert2Webp(cv::Mat img, const std::string& dest_path);
	}
}