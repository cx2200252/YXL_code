#include "YXLWebpWarpper.h"
#include "CmFile.h"

#ifdef CV_MAJOR_VERSION

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
		cv::cvtColor(img, img, CV_BGR2BGRA);

	WebPConfig config;
	WebPPicture pic;
	if (!WebPPictureInit(&pic) || !WebPConfigInit(&config))
		return false;
	//check within WebPEncode
	/*if (!WebPValidateConfig(&config))
	return false;*/

	pic.use_argb = 1;
	pic.argb = reinterpret_cast<uint32_t*>(img.data);
	pic.argb_stride = img.cols;
	pic.width = img.cols;
	pic.height = img.rows;

	WebPMemoryWriter memory_writer;
	WebPMemoryWriterInit(&memory_writer);

	pic.writer = WebPMemoryWrite;
	pic.custom_ptr = (void*)&memory_writer;

	if (!WebPEncode(&config, &pic))
	{
		std::cout << "Error! Cannot encode picture as WebP" << std::endl;
		std::cout << "Error code: " << pic.error_code << " (" << kErrorMessages[pic.error_code] << ")" << std::endl;
		return false;
	}

	FILE* file = fopen(dest_path.c_str(), "wb");
	auto ret = fwrite(memory_writer.mem, memory_writer.size, 1, file);
	fclose(file);


#if WEBP_ENCODER_ABI_VERSION > 0x0203
	WebPMemoryWriterClear(&memory_writer);
#else
	free(memory_writer.mem);
#endif

	return ret == 1;
}

#endif
