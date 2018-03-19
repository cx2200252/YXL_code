#pragma once
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include <opencv2/opencv.hpp>

//#define _NO_WINDOWS_

#ifndef _NO_WINDOWS_
#include <Windows.h>
#else
#include <qdatetime.h>
#include <qmutex.h>
#endif

namespace YXL
{
	inline std::string GetCurTime()
	{
#ifndef _NO_WINDOWS_
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
#else
		QDateTime time = QDateTime::currentDateTime();
		QString str = time.toString("yyyyMMdd_hhmmss");
		return str.toStdString();
#endif
}

	inline std::string ToUnixPath(const std::string& str)
	{
		std::string path = str;
		for (auto iter = path.begin(); iter != path.end(); ++iter)
			if ('\\' == *iter)
				*iter = '/';
		return path;
	}

	inline std::string ToWindowsPath(const std::string& str)
	{
		std::string path = str;
		for (auto iter = path.begin(); iter != path.end(); ++iter)
			if ('/' == *iter)
				*iter = '\\';
		return path;
	}

	inline std::string CheckDirPath(const std::string& str)
	{
		if ('/' == *str.rbegin() || '\\' == *str.rbegin())
			return str;
		else
			return str + "/";
	}

	inline std::string ReplaceStrings(const std::string& str, std::map<std::string, std::string>& replace_strs)
	{
		std::string res = str;
		size_t pos=0;
		for (auto& rep:replace_strs)
			while (std::string::npos != (pos = res.find(rep.first)))
				res.replace(pos, rep.first.length(), rep.second);
		return res;
	}

	inline long long MakeLongLong(const unsigned int a, const unsigned int b)
	{
		return ((unsigned long long)a) << 32 | (unsigned long long)b;
	}

	inline std::pair<unsigned int, unsigned int> SplitLongLong(const unsigned long long l)
	{
		return std::make_pair((unsigned int)(l >> 32), (unsigned int)(l & 0xffffffff));
	}

	inline size_t FileSize(std::ifstream& file)
	{
		std::streampos oldPos = file.tellg();

		file.seekg(0, std::ios::beg);
		std::streampos beg = file.tellg();
		file.seekg(0, std::ios::end);
		std::streampos end = file.tellg();

		file.seekg(oldPos, std::ios::beg);

		return static_cast<size_t>(end - beg);
	}
	inline size_t FileSize(const std::string& filename)
	{
		std::ifstream fin(filename);
		size_t ret = FileSize(fin);
		fin.close();
		return ret;
	}

	template<typename type> bool LoadFileContentBinary(const std::string& filepath, std::vector<type>& data)
	{
		std::ifstream fin(filepath, std::ios::binary);
		if (false == fin.good())
		{
			fin.close();
			return false;
		}
		size_t size = FileSize(fin);
		if (0 == size)
		{
			fin.close();
			return false;
		}
		data.resize(size / sizeof(type));
		fin.read(reinterpret_cast<char*>(&data[0]), size);

		fin.close();
		return true;
	}

	inline void LoadFileContent(const std::string& path, std::string& content) 
	{
		content = "";
		std::ifstream fin(path);
		std::string line;
		while (getline(fin, line))
			content += line + "\n";
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
				name = param;
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
#ifndef _NO_WINDOWS_
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
#endif

	template<typename _Elem, typename _Traits>
	class YXLOutStream
	{
	public:
		typedef std::basic_ostream<_Elem, _Traits> _Myt;
		typedef std::basic_ios<_Elem, _Traits> _Myios;
		typedef std::basic_streambuf<_Elem, _Traits> _Mysb;
		typedef std::ostreambuf_iterator<_Elem, _Traits> _Iter;
		typedef std::num_put<_Elem, _Iter> _Nput;

#ifndef _NO_WINDOWS_
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
#else
		void Lock()
		{
			cs.lock();
		}
		void Unlock()
		{
			cs.unlock();
		}
#endif

		template<typename type>
		YXLOutStream& operator<<(type val)
		{
			std::cout << val;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << val;
				fout.close();
			}
			return *this;
		}
		YXLOutStream& operator<<(_Myt& (__cdecl *_Pfn)(_Myt&))
		{
			std::cout << _Pfn;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << _Pfn;
				fout.close();
			}
			return *this;
		}
		YXLOutStream& operator<<(_Myios& (__cdecl *_Pfn)(_Myios&))
		{
			std::cout << _Pfn;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << _Pfn;
				fout.close();
			}
			return *this;
		}
		YXLOutStream& operator<<(std::ios_base& (__cdecl *_Pfn)(std::ios_base&))
		{
			std::cout << _Pfn;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << _Pfn;
				fout.close();
			}
			return *this;
		}

		void SetLogFile(const std::string& log_file)
		{
			_log_file = log_file;
		}

	private:
#ifndef _NO_WINDOWS_
		CRITICAL_SECTION cs;
#else
		QMutex cs;
#endif
		std::string _log_file = "YXLOut.txt";
	};

	extern YXLOutStream<char, std::char_traits<char> > yxlout;


#ifndef _NO_WINDOWS_
#define YXL_LOG_PREFIX GetCurrentThreadId()<<"["<<__FUNCTION__<<"] "
#else
#define YXL_LOG_PREFIX "["<<__FUNCTION__<<"] "
#endif


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


namespace YXL
{
	cv::Mat ComputeMeshNormal(cv::Mat vertices, cv::Mat tris, bool is_reverse_normal);

	cv::Mat FilterImage(cv::Mat img, cv::Mat kernel, cv::Mat mask=cv::Mat());
}

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