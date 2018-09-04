#include "YXLWebpWarpper.h"
#include <iostream>

namespace YXL
{
	namespace WebP
	{
		static const char* const kErrorMessages[VP8_ENC_ERROR_LAST] = {
			"OK",
			"OUT_OF_MEMORY: Out of memory allocating objects",
			"BITSTREAM_OUT_OF_MEMORY: Out of memory re-allocating byte buffer",
			"NULL_PARAMETER: NULL parameter passed to function",
			"INVALID_CONFIGURATION: configuration is invalid",
			"BAD_DIMENSION: Bad picture dimension. Maximum width and height "
			"allowed is 16383 pixels.",
			"PARTITION0_OVERFLOW: Partition #0 is too big to fit 512k.\n"
			"To reduce the size of this partition, try using less segments "
			"with the -segments option, and eventually reduce the number of "
			"header bits using -partition_limit. More details are available "
			"in the manual (`man cwebp`)",
			"PARTITION_OVERFLOW: Partition is too big to fit 16M",
			"BAD_WRITE: Picture writer returned an I/O error",
			"FILE_TOO_BIG: File would be too big to fit in 4G",
			"USER_ABORT: encoding abort requested by user"
		};
	}
}

#ifdef WITH_OPENCV

bool YXL::WebP::Convert2Webp(const std::string & src_path, const std::string & dest_path)
{
	cv::Mat img = cv::imread(src_path, -1);
	return Convert2Webp(img, dest_path);
}

bool YXL::WebP::Convert2Webp(cv::Mat img, const std::string & dest_path)
{
	if (img.empty())
		return false;
	if (img.channels() == 3)
		cv::cvtColor(img, img, CV_BGR2RGBA);
	else
		cv::cvtColor(img, img, CV_BGRA2RGBA);

	std::string out;
	bool ret = ToWebp(img.data(), img.cols, img.rows, out);
	
	FILE* file = fopen(dest_path.c_str(), "wb");
	auto ret = fwrite(out, out.length(), 1, file);
	fclose(file);
	return ret == 1;
}

#endif

bool YXL::WebP::ToWebp(char * in, int w, int h, std::string & out)
{
	uint8_t* ptr = nullptr;
	//auto size = WebPEncodeBGRA(reinterpret_cast<uint8_t*>(in), w, h, w, 100.f, &ptr);
	auto size = WebPEncodeLosslessRGBA(reinterpret_cast<uint8_t*>(in), w, h, w, &ptr);

	if (!size)
	{
		std::cout << "Error! Cannot encode picture as WebP" << std::endl;
		return false;
	}
	out = std::string((char*)ptr, size);

	WebPFree(ptr);

	return true;
}

void YXL::WebP::FromWebp(char * in, const int in_size, std::string & out, int & w, int & h)
{
	auto ret = WebPDecodeRGBA(reinterpret_cast<uint8_t*>(in), in_size, &w, &h);
	out = std::string((char*)ret, w*h * 4);
}
