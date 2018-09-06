#ifndef _BS_TENSOR_H_
#define _BS_TENSOR_H_

#ifdef _YXL_DLL_
#ifndef YXL_EXPORT
#ifdef _YXL_EXPORT_IMPL
#define YXL_EXPORT __declspec(dllexport)
#else
#define YXL_EXPORT __declspec(dllimport)
#endif
#endif
#else
#define YXL_EXPORT
#endif

#include "TriMesh.h"

namespace YXL
{
	class YXL_EXPORT BSTensor
	{
	public:
		BSTensor(const int bs_cnt);
		~BSTensor();

		bool IsOK()
		{
			return _is_ok;
		}

		void LoadFromBS(vecS& bs_list);
		void LoadFromBS(std::vector<std::shared_ptr<TriMesh> >& bs);
		TriMesh RetrieveMesh(std::vector<float>& exp);
		std::shared_ptr<TriMesh> RetrieveMeshPtr(std::vector<float>& exp);
		std::shared_ptr<TriMesh> GetBS0() const
		{
			return std::shared_ptr<TriMesh>(new TriMesh(*b0));
		}

	private:
		const int _bs_cnt;
		bool _is_ok = false;

		std::shared_ptr<TriMesh> b0 = nullptr;
		cv::Mat tensor;
	};

}

#endif