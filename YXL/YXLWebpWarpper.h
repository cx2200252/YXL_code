#ifndef _YXL_WEBP_WARPPER_H_
#define _YXL_WEBP_WARPPER_H_
#include <string>
#include "webp/webp/encode.h"
#include "webp/webp/decode.h"
#ifdef WITH_OPENCV
#include <opencv2/opencv.hpp>
#endif

namespace YXL
{
	namespace WebP
	{
#ifdef WITH_OPENCV
		bool Convert2Webp(const std::string& src_path, const std::string& dest_path);
		bool Convert2Webp(cv::Mat img, std::string& dest_path);
#endif
		bool ToWebp(char* in, int w, int h, std::string& out);
		void FromWebp(char* in, const int in_size, std::string& out, int& w, int& h);
	}
}
#endif