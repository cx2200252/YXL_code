#include "BSTensor.h"


YXL::BSTensor::BSTensor(const int bs_cnt)
	:_bs_cnt(bs_cnt)
{
}

YXL::BSTensor::~BSTensor()
{
}

void YXL::BSTensor::LoadFromBS(vecS & bs_list)
{
	using namespace std;
	using namespace cv;
	YXL::yxlout << YXL_LOG_PREFIX << "loading tensor..." << endl;
	if (bs_list.size() != _bs_cnt)
	{
		YXL::yxlout << YXL_LOG_PREFIX << "errors occur during loading tensor..." << endl;
		return;
	}

	for (int i(0); i != _bs_cnt; ++i)
	{
		if (false == YXL::File::FileExist(bs_list[i]))
		{
			YXL::yxlout << YXL_LOG_PREFIX << "errors occur during loading tensor..." << endl;
			return;
		}
	}

	std::vector<std::shared_ptr<TriMesh> > bs(_bs_cnt);
	int i = 0;
#pragma omp parallel for
	for (i = 0; i < _bs_cnt; ++i)
	{
		bs[i] = std::shared_ptr<TriMesh>(new TriMesh());

		bs[i]->LoadFromObjFile(bs_list[i]);
		{
			YXL::yxlout.Lock();
			YXL::yxlout << YXL_LOG_PREFIX << "load bs: " << bs_list[i] << std::endl;
			YXL::yxlout.Unlock();
		}
	}
	LoadFromBS(bs);

	YXL::yxlout << YXL_LOG_PREFIX << "tensor loaded..." << endl;
}

void YXL::BSTensor::LoadFromBS(std::vector<std::shared_ptr<TriMesh> >& bs)
{
	if (bs.size() != _bs_cnt)
	{
		YXL::yxlout << YXL_LOG_PREFIX << "#blendshapes != "<< _bs_cnt<<"..." << std::endl;
		return;
	}
	CStr id = bs[0]->GetIdentifier();
	for (int i(1); i != _bs_cnt; ++i)
	{
		if (id != bs[i]->GetIdentifier())
		{
			YXL::yxlout << YXL_LOG_PREFIX << "blendshapes' topology are not the same..." << std::endl;
			return;
		}
	}

	b0 = std::shared_ptr<TriMesh>(new TriMesh(*bs[0]));
	cv::Mat tmp;
	for (int i(1); i != _bs_cnt; ++i)
	{
		cv::Mat diff = bs[i]->_render_vertices - b0->_render_vertices;
		tmp.push_back(diff.t());
	}
	tensor = tmp.t();
	tensor = tensor.clone();
	_is_ok = true;
}

YXL::TriMesh YXL::BSTensor::RetrieveMesh(std::vector<float>& exp)
{
	if (_bs_cnt-1 != exp.size())
		return TriMesh();

	TriMesh mesh(*b0);
	for (int i(0); i != exp.size(); ++i)
		mesh._render_vertices += tensor.col(i)*exp[i];

	return mesh;
}

std::shared_ptr<YXL::TriMesh> YXL::BSTensor::RetrieveMeshPtr(std::vector<float>& exp)
{
	if (_bs_cnt-1 != exp.size())
		return nullptr;

	auto ret = std::shared_ptr<TriMesh>(new TriMesh(*b0));
	for (int i(0); i != exp.size(); ++i)
		ret->_render_vertices += tensor.col(i)*exp[i];

	return ret;
}
