#pragma once
#include <string>
#include <map>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

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
		}
		~YXLOutStream()
		{
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

	namespace SHA1
	{
		std::string SHA1Digest(std::string str);
	}
}