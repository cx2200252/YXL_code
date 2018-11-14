#pragma once
#include "../../YXLHelper.h"

namespace YXL_TEST
{
	inline std::string GetTestDir()
	{
		std::string ret = "tmp/";
		if (false == YXL::File::FolderExist(ret))
			YXL::File::MkDir(ret);
		return ret;
	}


	void TestJson();
	void TestZip();
	void TestNaCL();
	void TestCrypto();

}