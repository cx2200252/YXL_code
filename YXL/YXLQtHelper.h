#ifndef _YXL_QT_HELPER_H_
#define _YXL_QT_HELPER_H_

#if defined(_WITH_QT_) && defined(_WITH_OPENCV_)
#include "CmFile.h"
#include <QLabel>

#define ToQStr(str) QString::fromLocal8Bit((str).c_str())
#define FromQStr(str) ((std::string)(str).toLocal8Bit())

namespace YXL
{
	inline cv::Mat ImresizeAspectFix(cv::Mat img, const cv::Size size, const cv::Scalar bg_color = cv::Scalar::all(128))
	{
		const float aspect_dest = static_cast<float>(size.width) / size.height;
		const float aspect_src = static_cast<float>(img.cols) / img.rows;
		cv::Size s;
		if (aspect_src > aspect_dest)
		{
			s.width = size.width;
			s.height = s.width / aspect_src;
		}
		else
		{
			s.height = size.height;
			s.width = s.height*aspect_src;
		}
		cv::resize(img, img, s);

		cv::Mat ret(size, img.type(), bg_color);
		const int sx = (size.width - s.width) / 2;
		const int sy = (size.height - s.height) / 2;
		img.copyTo(ret.rowRange(sy, sy + s.height).colRange(sx, sx + s.width));
		return ret;
	}
	inline bool QLabelSetImage(QLabel* label, const cv::Mat img, const cv::Scalar bg_color = cv::Scalar::all(128))
	{
		if (img.empty())
			return false;
		auto size = label->size();
		auto data = ImresizeAspectFix(img, cv::Size(size.width(), size.height()));
		if (data.channels() == 4)
			cv::cvtColor(data, data, CV_BGRA2RGBA);
		else
			cv::cvtColor(data, data, CV_BGR2RGBA);

		auto img2 = new QImage(data.data, data.cols, data.rows, QImage::Format::Format_RGBA8888);
		label->setPixmap(QPixmap::fromImage(*img2));
		return true;
	}
	inline bool QLabelSetImage(QLabel* label, const std::string path, const cv::Scalar bg_color=cv::Scalar::all(128))
	{
		if (false == CmFile::FileExist(path))
			return false;
		cv::Mat img = cv::imread(path);
		return QLabelSetImage(label, img, bg_color);
	}
}
#endif

#endif
