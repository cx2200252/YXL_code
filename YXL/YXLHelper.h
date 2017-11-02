#pragma once
#include <string>
#include <map>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#ifdef CV_VERSION
#include <opencv2\opencv.hpp>
#endif

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

	template<typename key, typename val> void PrintMapAsRows(std::map<key, val>& m)
	{
		for (auto iter = m.begin(); iter != m.end(); ++iter)
		{
			YXL::yxlout << iter->first << '\t' << iter->second << std::endl;
		}
	}

	template<typename val> void PrintVectorAsRows(std::vector<val>& v)
	{
		for (int i(0); i != v.size(); ++i)
		{
			YXL::yxlout << i << '\t' << v[i] << std::endl;
		}
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

			Vec3f p0 = vertices.at<Vec3f>(tri[is_reverse_normal ? 2 : 0]);
			Vec3f p1 = vertices.at<Vec3f>(tri[1]);
			Vec3f p2 = vertices.at<Vec3f>(tri[is_reverse_normal ? 0 : 2]);

			Vec3f a = p0 - p1, b = p1 - p2, c = p2 - p0;
			float l2a = a.dot(a), l2b = b.dot(b), l2c = c.dot(c);
			Vec3f facenormal = a.cross(b);
			normals.at<Vec3f>(tri[0]) += facenormal*(1.0f / (l2a*l2c));
			normals.at<Vec3f>(tri[1]) += facenormal*(1.0f / (l2b*l2a));
			normals.at<Vec3f>(tri[2]) += facenormal*(1.0f / (l2c*l2b));
		}
		for (int i(0); i != normals.rows; ++i)
		{
			normals.at<Vec3f>(i) = cv::normalize(normals.at<Vec3f>(i));
		}
		return normals;
	}
}
#endif