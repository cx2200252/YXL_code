#pragma once
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#ifdef CV_VERSION
#include <opencv2/opencv.hpp>
#endif

#ifndef _NO_WINDOWS_
#include <Windows.h>

namespace YXL
{
	inline std::string GetCurTime()
	{
		SYSTEMTIME time;

		GetLocalTime(&time);

		char tmp[21] = {};

		sprintf_s(tmp, sizeof(tmp), "%04d%02d%02d_%02d%02d%02d",
			time.wYear,
			time.wMonth,
			time.wDay,
			time.wHour,
			time.wMinute,
			time.wSecond);

		return std::string(tmp);
	}

	inline std::string ToUnixPath(const std::string& str)
	{
		std::string path = str;
		for (auto iter = path.begin(); iter != path.end(); ++iter)
		{
			if ('\\' == *iter)
			{
				*iter = '/';
			}
		}
		return path;
	}

	inline std::string ToWindowsPath(const std::string& str)
	{
		std::string path = str;
		for (auto iter = path.begin(); iter != path.end(); ++iter)
		{
			if ('/' == *iter)
			{
				*iter = '\\';
			}
		}
		return path;
	}

	inline std::string CheckPath(const std::string& str)
	{
		if ('/' == *str.rbegin() || '\\' == *str.rbegin())
			return str;
		else
			return str + "/";
	}

	inline std::string ReplaceStrings(const std::string& str, std::map<std::string, std::string>& replace_strs)
	{
		std::string res = str;
		for (auto iter = replace_strs.begin(); iter != replace_strs.end(); ++iter)
		{
			size_t pos;
			while (std::string::npos != (pos = res.find(iter->first)))
			{
				res.replace(pos, iter->first.length(), iter->second);
			}
		}
		return res;
	}

	inline long long MakeLongLong(const int a, const int b)
	{
		return ((long long)a) << 32 | (long long)b;
	}

	inline std::pair<int, int> SplitLongLong(const long long l)
	{
		return std::make_pair((int)(l >> 32), (int)(l & 0xffffffff));
	}

	inline void LoadFileContent(const std::string& path, std::string& content) 
	{
		content = "";
		std::ifstream fin(path);
		std::string line;
		while (getline(fin, line))
		{
			content += line + "\n";
		}
		fin.close();
	}

	typedef bool (*CmdLineParserCallback)(const std::string& name, const std::string& val);
	//argv: -name=val
	/*
	std::multimap<std::string, std::string> params;
	bool ParserCallback(const std::string& name, const std::string& val)
	{
		params.insert(std::make_pair(name, val));
		return true;
	}
	CmdLineParser(argc, argv, ParserCallback);
	*/
	inline void CmdLineParser(int argc, char** argv, CmdLineParserCallback callback)
	{
		for (int i(1); i < argc; ++i)
		{
			std::string param = argv[i];
			auto pos = param.find('=');
			std::string name = "";
			std::string val = "";
			if (pos != std::string::npos)
			{
				name = param.substr(0, pos);
				val = param.substr(pos + 1, param.length() - pos - 1);
			}
			else
			{
				name = param;
			}
			callback(name, val);
		}
	}

	enum FileInfo {
		FileInfo_CreateTime,
		FileInfo_LastAccessTime,
		FileInfo_LastWriteTime,
		FileInfo_FileSize
	};
	//return high-low
	inline std::pair<DWORD, DWORD> GetFileInfo(const std::string& file_path, FileInfo fi)
	{
		WIN32_FIND_DATAA ffd;
		HANDLE hFind = FindFirstFileA(file_path.c_str(), &ffd);
		if (INVALID_HANDLE_VALUE == hFind)
			return std::pair<DWORD, DWORD>(-1, -1);

		std::pair<DWORD, DWORD> ret;
		switch (fi)
		{
		case FileInfo_CreateTime:
			ret = std::make_pair(ffd.ftCreationTime.dwHighDateTime, ffd.ftCreationTime.dwLowDateTime);
			break;
		case FileInfo_LastAccessTime:
			ret = std::make_pair(ffd.ftLastAccessTime.dwHighDateTime, ffd.ftLastAccessTime.dwLowDateTime);
			break;
		case FileInfo_LastWriteTime:
			ret = std::make_pair(ffd.ftLastWriteTime.dwHighDateTime, ffd.ftLastWriteTime.dwLowDateTime);
			break;
		case FileInfo_FileSize:
			ret = std::make_pair(ffd.nFileSizeHigh, ffd.nFileSizeLow);
			break;
		}
		return ret;
	}

	template<typename _Elem, typename _Traits>
	class YXLOutStream
	{
	public:
		typedef std::basic_ostream<_Elem, _Traits> _Myt;
		typedef std::basic_ios<_Elem, _Traits> _Myios;
		typedef std::basic_streambuf<_Elem, _Traits> _Mysb;
		typedef std::ostreambuf_iterator<_Elem, _Traits> _Iter;
		typedef std::num_put<_Elem, _Iter> _Nput;


		YXLOutStream()
		{
			InitializeCriticalSection(&cs);
		}
		~YXLOutStream()
		{
			DeleteCriticalSection(&cs);
		}

		void Lock()
		{
			EnterCriticalSection(&cs);
		}
		void Unlock()
		{
			LeaveCriticalSection(&cs);
		}

		template<typename type>
		YXLOutStream& operator<<(type val)
		{
			std::cout << val;
			std::ofstream fout("YXLOut.txt", std::ios::app);
			fout << val;
			fout.close();
			return *this;
		}
		YXLOutStream& operator<<(_Myt& (__cdecl *_Pfn)(_Myt&))
		{
			std::cout << _Pfn;
			std::ofstream fout("YXLOut.txt", std::ios::app);
			fout << _Pfn;
			fout.close();
			return *this;
		}
		YXLOutStream& operator<<(_Myios& (__cdecl *_Pfn)(_Myios&))
		{
			std::cout << _Pfn;
			std::ofstream fout("YXLOut.txt", std::ios::app);
			fout << _Pfn;
			fout.close();
			return *this;
		}
		YXLOutStream& operator<<(std::ios_base& (__cdecl *_Pfn)(std::ios_base&))
		{
			std::cout << _Pfn;
			std::ofstream fout("YXLOut.txt", std::ios::app);
			fout << _Pfn;
			fout.close();
			return *this;
		}

	private:
		CRITICAL_SECTION cs;
	};

	extern YXLOutStream<char, std::char_traits<char> > yxlout;

#define YXL_LOG_PREFIX GetCurrentThreadId()<<"["<<__FUNCTION__<<"] "



	template<typename key, typename val> void PrintMapAsRows(std::map<key, val>& m, const std::string& padding="")
	{
		for (auto iter = m.begin(); iter != m.end(); ++iter)
		{
			YXL::yxlout << padding<< iter->first << '\t' << iter->second << std::endl;
		}
	}

	template<typename val> void PrintVectorAsRows(std::vector<val>& v)
	{
		for (int i(0); i != v.size(); ++i)
		{
			YXL::yxlout << i << ':\t' << v[i] << std::endl;
		}
	}

	template<typename val> void PrintVector(std::vector<val>& v)
	{
		YXL::yxlout << "[";
		for (int i(0); i != v.size(); ++i)
		{
			if (i)
				YXL::yxlout << ",";
			YXL::yxlout << v[i];
		}
		YXL::yxlout << "]" << std::endl;
	}

	template<typename VertexType, typename IndexType> void SavePlainObjFile(const std::string& save_path, const VertexType* vertices, const int vertex_cnt, const IndexType* face, const int face_cnt)
	{
		std::ofstream fout(save_path);
		for (int i(0); i != vertex_cnt; ++i)
		{
			fout << "v " << vertices[0] << " " << vertices[1] << " " << vertices[2] << std::endl;
			vertices += 3;
		}
		for (int i(0); i != face_cnt; ++i)
		{
			fout << "f ";
			for (int j(0); j != 3; ++j)
			{
				fout << face[2-j]+1 << " ";
			}
			fout << std::endl;
			face += 3;
		}

		fout.close();
	}

	namespace SHA1
	{
		std::string SHA1Digest(std::string str);
	}

	class UnionFind
	{
	public:
		UnionFind(int cnt):_group_cnt(cnt), _id(std::vector<int>(cnt))
		{
			for (int i(0); i != cnt; ++i)
			{
				_id[i] = i;
				_group_size[i] = 1;
			}
		}

		int GroupCount() const;
		bool IsConnected(const int a, const int b) const;
		int Find(int a) const;
		void Union(int a, int b);
		void Update();

		int GroupID(const int a) const;
		const std::vector<int>* Group(const int group_id);
		
	private:
		int _group_cnt;
		std::vector<int> _id;
		std::map<int, int> _group_size;
		
		std::vector<int> _group_id;
		std::map<int, std::vector<int> > _groups;
	};
}

#else

#endif

#ifdef CV_VERSION
namespace YXL
{
	inline cv::Mat ComputeMeshNormal(cv::Mat vertices, cv::Mat tris, bool is_reverse_normal)
	{
		using namespace std;
		using namespace cv;
		cv::Mat normals = Mat(vertices.size(), vertices.type(), cv::Scalar(0, 0, 0, 0));
		for (int i(0); i != tris.rows; ++i)
		{
			Vec3i tri = tris.at<Vec3i>(i);

			int idx[3] = { tri[is_reverse_normal ? 2 : 0], tri[1], tri[is_reverse_normal ? 0 : 2] };

			Vec3f p0 = vertices.at<Vec3f>(idx[0]);
			Vec3f p1 = vertices.at<Vec3f>(idx[1]);
			Vec3f p2 = vertices.at<Vec3f>(idx[2]);

			Vec3f a = p0 - p1, b = p1 - p2, c = p2 - p0;
			float l2a = a.dot(a), l2b = b.dot(b), l2c = c.dot(c);
			Vec3f facenormal = a.cross(b);
			normals.at<Vec3f>(idx[0]) += facenormal*(1.0f / (l2a*l2c));
			normals.at<Vec3f>(idx[1]) += facenormal*(1.0f / (l2b*l2a));
			normals.at<Vec3f>(idx[2]) += facenormal*(1.0f / (l2c*l2b));
		}
		for (int i(0); i != normals.rows; ++i)
		{
			normals.at<Vec3f>(i) = cv::normalize(normals.at<Vec3f>(i));
		}
		return normals;
	}
}
#endif

#ifdef HAS_CMFILE

namespace YXL
{
	inline std::string ToAbsolutePath(const std::string& path)
	{
		if (path.length() > 2 && path[1] == ':')
			return path;
		return CmFile::GetWkDir()+"/" + path;
	}
}

#endif