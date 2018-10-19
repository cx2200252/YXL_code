#ifndef _YXL_HELPER_H_
#define _YXL_HELPER_H_

//which modul to use
#define _YXL_OTHER_
#define _YXL_FILES_
#define _YXL_STRING_
#define _YXL_PARAM_PARSER_
#define _YXL_PRINT_
#define _YXL_OUT_STREAM_
#define _YXL_UNION_FIND_
#define _YXL_KD_TREE_
#define _YXL_TIME_
#define _YXL_CONSOLE_
#define _YXL_TRANSFORM_
#define _YXL_GRAPHIC_
#define _YXL_IMG_PROC_
//#define _YXL_IMG_CODEC_
//#define _YXL_MINI_Z_
//#define _YXL_HASH_
//#define _YXL_CIPHER_
#define _YXL_CRYPTO_
#define _YXL_ENCRYPT_DATA_
//#define _YXL_GLFW_

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>
#include <queue>
#include <algorithm>
#include <stack>

#pragma warning(disable:4819)
#ifdef YXL_HELPER_DYNAMIC
#ifndef LIB_YXL_HELPER
#ifdef _LIB_YXL_HELPER_IMPL
#define LIB_YXL_HELPER __declspec(dllexport)
#else
#define LIB_YXL_HELPER __declspec(dllimport)
#endif
#endif
#else
#define LIB_YXL_HELPER
#endif

namespace YXL
{
	typedef const std::string CStr;
	typedef std::vector<std::string> vecS;
	typedef std::vector<int> vecI;
	typedef std::vector<std::string> vecS;
	typedef std::vector<float> vecF;
	typedef std::vector<double> vecD;
}

//check marco
#ifndef _WITH_WINDOWS_
#undef _YXL_CONSOLE_
#endif
#ifndef _YXL_TRANSFORM_
#undef _YXL_GRAPHIC_
#endif
#ifndef _WITH_OPENCV_
#undef _YXL_IMG_PROC_
#endif

//
#ifdef _YXL_GLFW_
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#endif

#ifdef _WITH_OPENCV_

#define POINTER_64 __ptr64
#include <opencv2/opencv.hpp>

#ifndef CV_LIB
#define CV_VERSION_ID CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#ifdef _DEBUG
#define CV_LIB(name) "opencv_" name CV_VERSION_ID "d"
#else
#define CV_LIB(name) "opencv_" name CV_VERSION_ID
#endif
#endif

#define CV_Assert_(expr, args) \
{\
	if(!(expr)) {\
	std::string msg = cv::format args; \
	printf("%s in %s:%d\n", msg.c_str(), __FILE__, __LINE__); \
	cv::error(cv::Exception(CV_StsAssert, msg, __FUNCTION__, __FILE__, __LINE__) ); }\
}

#ifdef _WITH_OPENCV_WORLD_
#pragma comment( lib, CV_LIB("world"))
#else
#pragma comment( lib, CV_LIB("core"))
#pragma comment( lib, CV_LIB("imgproc"))
#pragma comment( lib, CV_LIB("highgui"))

#if (2 < CV_MAJOR_VERSION)
#pragma comment( lib, CV_LIB("imgcodecs"))
#endif

#endif
namespace YXL
{
	typedef const cv::Mat CMat;
	typedef std::vector<cv::Mat> vecM;
}

#endif

#ifdef _WITH_WINDOWS_
#include <Windows.h>
#else
#ifdef _WITH_QT_
#include <qdir.h>
#include <qprocess.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/QApplication.h>
#include <qdatetime.h>
#include <qmutex.h>

#ifdef _DEBUG
#pragma comment(lib, "Qt5Cored.lib")
#pragma comment(lib, "Qt5Guid.lib")
#pragma comment(lib, "Qt5Widgetsd.lib")
#else
#pragma comment(lib, "Qt5Core.lib")
#pragma comment(lib, "Qt5Gui.lib")
#pragma comment(lib, "Qt5Widgets.lib")
#endif
#endif
#endif

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#ifdef _YXL_OTHER_
namespace YXL
{
	inline long long MakeLongLong(const unsigned int a, const unsigned int b)
	{
		return ((unsigned long long)a) << 32 | (unsigned long long)b;
	}

	inline std::pair<unsigned int, unsigned int> SplitLongLong(const unsigned long long l)
	{
		return std::make_pair((unsigned int)(l >> 32), (unsigned int)(l & 0xffffffff));
	}

	inline int GetCurrentThreadID()
	{
#if defined(_WITH_WINDOWS_)
		return GetCurrentThreadId();
#elif defined(__linux__)
		return gettid();
#else
		return 0;
#endif
	}
}
#endif

#ifdef _YXL_STRING_
namespace YXL
{
	namespace Str
	{
		inline std::string Esacpe(const std::string& str, const std::string escape_ch="\t ")
		{
			auto beg = str.find_first_not_of(escape_ch);
			auto end = str.find_last_not_of(escape_ch);
			return str.substr(beg, end - beg+1);
		}

		inline std::string Replace(const std::string& str, const std::map<std::string, std::string>& replace_strs)
		{
			std::string res = str;
			size_t pos = 0;
			for (auto& rep : replace_strs)
				while (std::string::npos != (pos = res.find(rep.first)))
					res.replace(pos, rep.first.length(), rep.second);
			return res;
		}

		inline void Spilt(std::vector<std::string>& res, const std::string& src, const std::string& splitChars)
		{
			std::size_t prePos = src.find_first_not_of(splitChars);
			std::size_t pos = src.find_first_of(splitChars, prePos);
			while (std::string::npos != pos)
			{
				res.push_back(src.substr(prePos, pos - prePos));
				prePos = src.find_first_not_of(splitChars, pos + 1);
				pos = src.find_first_of(splitChars, prePos);
			}
			if (std::string::npos != prePos)
				res.push_back(src.substr(prePos, src.length() - prePos));
		}
	}
}
#endif

#ifdef _YXL_FILES_
namespace YXL
{
	class LIB_YXL_HELPER File
	{
	public:
		static std::string ToUnixPath(const std::string& str)
		{
			std::string path = str;
			for (auto iter = path.begin(); iter != path.end(); ++iter)
				if ('\\' == *iter)
					*iter = '/';
			return path;
		}
		static std::string ToWindowsPath(const std::string& str)
		{
			std::string path = str;
			for (auto iter = path.begin(); iter != path.end(); ++iter)
				if ('/' == *iter)
					*iter = '\\';
			return path;
		}
		static std::string CheckDirPath(const std::string& str)
		{
			return ('/' == *str.rbegin() || '\\' == *str.rbegin()) ? str : str + "/";
		}
		static std::string GetFolder(CStr& path)
		{
			return path.substr(0, path.find_last_of("\\/") + 1);
		}
		static std::string GetName(CStr& path)
		{
			size_t start = path.find_last_of("\\/") + 1;
			size_t end = path.find_last_not_of(' ') + 1;
			return path.substr(start, end - start);
		}
		static std::string GetNameNE(CStr& path)
		{
			size_t start = path.find_last_of("\\/") + 1;
			size_t end = path.find_last_of('.');
			if (end >= 0)
				return path.substr(start, end - start);
			else
				return path.substr(start, path.find_last_not_of(' ') + 1 - start);
		}
		static std::string GetPathNE(CStr& path)
		{
			size_t end = path.find_last_of('.');
			if (end >= 0)
				return path.substr(0, end);
			else
				return path.substr(0, path.find_last_not_of(' ') + 1);
		}
		static std::string GetExtention(CStr name)
		{
			return name.substr(name.find_last_of('.'));
		}
		static void WriteNullFile(CStr& fileName) 
		{
			FILE *f = fopen(fileName.c_str(), "w");
			fclose(f); 
		}
		static vecS loadStrList(CStr &fName);
		static bool writeStrList(CStr &fName, const vecS &strs);

		static size_t FileSize(std::ifstream& file)
		{
			std::streampos oldPos = file.tellg();

			file.seekg(0, std::ios::beg);
			std::streampos beg = file.tellg();
			file.seekg(0, std::ios::end);
			std::streampos end = file.tellg();

			file.seekg(oldPos, std::ios::beg);

			return static_cast<size_t>(end - beg);
		}
		static size_t FileSize(const std::string& filename)
		{
			std::ifstream fin(filename);
			size_t ret = FileSize(fin);
			fin.close();
			return ret;
		}

		static bool LoadFileContentBinary(const std::string& filepath, std::string& data)
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
			data.resize(size);
			fin.read(reinterpret_cast<char*>(&data[0]), size);
			fin.close();
			return true;
		}

		static bool WriteFileContentBinary(const std::string& filepath, const std::string& data)
		{
			std::ofstream fout(filepath, std::ios::binary);
			if (false == fout.good())
			{
				fout.close();
				return false;
			}
			fout.write(data.data(), data.length());
			fout.close();
			return true;
		}

		static void LoadFileContent(const std::string& path, std::string& content, const std::string ignore="")
		{
			content = "";
			std::ifstream fin(path);
			std::string line;
			if ("" == ignore)
			{
				while (getline(fin, line))
					content += line + "\n";
			}
			else
			{
				while (getline(fin, line))
					if (Str::Esacpe(line).substr(0, ignore.length()) != ignore)
						content += line + "\n";
			}
			
			fin.close();
		}

		template<typename VertexType, typename IndexType> static void SavePlainObjFile(const std::string& save_path, const VertexType* vertices, const int vertex_cnt, const IndexType* face, const int face_cnt)
		{
			std::ofstream fout(save_path);
			for (int i(0); i != vertex_cnt; ++i)
			{
				fout << "v " << vertices[0] << " " << vertices[1] << " " << vertices[2] << "\n";
				vertices += 3;
			}
			for (int i(0); i != face_cnt; ++i)
			{
				fout << "f ";
				for (int j(0); j != 3; ++j)
				{
					fout << face[2 - j] + 1 << " ";
				}
				fout << "\n";
				face += 3;
			}
			fout.close();
		}

#if defined(_WITH_WINDOWS_) || defined(_WITH_QT_)
		static std::string ToAbsolutePath(const std::string& path)
		{
			if (path.length() > 2 && path[1] == ':')
				return path;
			return GetWkDir() + "/" + path;
		}
#endif

#ifdef _WITH_WINDOWS_
		enum FileInfo {
			FileInfo_CreateTime,
			FileInfo_LastAccessTime,
			FileInfo_LastWriteTime,
			FileInfo_FileSize
		};
		//return high-low
		static std::pair<DWORD, DWORD> GetFileInfo(const std::string& file_path, FileInfo fi);

		//filters: pair of "text-filter", {"Images", "*.jpg;*.png"}
		static std::string BrowseFile(const std::vector<std::pair<std::string, std::string>>& filters, bool isOpen = true, const std::string& dir = "", CStr& title = "BrowseFile");
		static std::string BrowseFolder(CStr& title = "BrowseFolder");

		// Get file names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
		static int GetNames(CStr &nameW, vecS &names, std::string &dir = std::string());
		static int GetNames(CStr& rootFolder, CStr &fileW, vecS &names);
		static int GetNamesNE(CStr& nameWC, vecS &names, std::string &dir = std::string(), std::string &ext = std::string());
		static int GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names);

		static bool FileExist(CStr& filePath)
		{
			if (filePath.size() == 0)
				return false;
			return  GetFileAttributesA(filePath.c_str()) != INVALID_FILE_ATTRIBUTES;
		}
		//FilesExist("./*.jpg")
		static bool FilesExist(CStr& fileW)
		{
			vecS names;
			int fNum = GetNames(fileW, names);
			return fNum > 0;
		}
		static bool FolderExist(CStr& strPath)
		{
			if (strPath.size() == 0)
				return false;
			int i = (int)strPath.size() - 1;
			for (; i >= 0 && (strPath[i] == '\\' || strPath[i] == '/'); i--)
				;
			std::string str = strPath.substr(0, i + 1);

			WIN32_FIND_DATAA  wfd;
			HANDLE hFind = FindFirstFileA(str.c_str(), &wfd);
			bool rValue = (hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			FindClose(hFind);
			return rValue;
		}

		static std::string GetWkDir()
		{
			std::string wd;
			wd.resize(1024);
			DWORD len = GetCurrentDirectoryA(1024, &wd[0]);
			wd.resize(len);
			return wd;
		}
		static void SetWkDir(CStr& dir)
		{
			SetCurrentDirectoryA(dir.c_str());
		}

		static bool MkDir(CStr&  path);

		// Eg: RenameImages("D:/DogImages/*.jpg", "F:/Images", "dog", ".jpg");
		static int Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt);

		//RmFile("./*.jpg")
		static void RmFile(CStr& fileW);
		static void CleanFolder(CStr& dir, bool subFolder = false);
		static void RmFolder(CStr& dir);
		

		static int GetSubFolders(CStr& folder, vecS& subFolders);

		static bool Copy(CStr &src, CStr &dst, bool failIfExist = false)
		{
			return ::CopyFileA(src.c_str(), dst.c_str(), failIfExist)==TRUE;
		}
		static bool Move(CStr &src, CStr &dst, DWORD dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH)
		{
			return MoveFileExA(src.c_str(), dst.c_str(), dwFlags)==TRUE;
		}
		//Move2Dir("./*.jpg", "../")
		static bool Move2Dir(CStr &srcW, CStr dstDir);
		//Copy2Dir("./*.jpg", "../")
		static bool Copy2Dir(CStr &srcW, CStr dstDir);

		static void RunProgram(CStr &fileName, CStr &parameters = "", bool waiteF = false, bool showW = true);
#else
#ifdef _WITH_QT_
		static void InitQApplication(int argc, char** argv)
		{
			if (nullptr == _app)
				_app = std::shared_ptr<QApplication>(new QApplication(argc, argv));
		}
		//may need call YXL::File::InitQApplication first
		//filters: pair of "text-filter", {"Images", "*.jpg;*.png"}
		static std::string BrowseFile(const std::vector<std::pair<std::string, std::string>>& filters, bool isOpen = true, const std::string& dir = "", CStr& title = "BrowseFile");
		//may need call YXL::File::InitQApplication first
		//filters: pair of "text-filter", {"Images", "*.jpg;*.png"}
		static std::string BrowseFolder(CStr& title = "BrowseFolder");

		// Get file names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
		static int GetNames(CStr &nameW, vecS &names, std::string &dir);
		static int GetNames(CStr& rootFolder, CStr &fileW, vecS &names);
		static int GetNamesNE(CStr& nameWC, vecS &names, std::string &dir, std::string &ext);
		static int GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names);

		static inline bool FileExist(CStr& filePath)
		{
			if (filePath.size() == 0)
				return false;

			QFile file(QString::fromLocal8Bit(filePath.c_str()));
			return file.exists();
		}
		static inline bool FilesExist(CStr& fileW)
		{
			vecS names;
			std::string dir;
			int fNum = GetNames(fileW, names, dir);
			return fNum > 0;
		}
		static inline bool FolderExist(CStr& strPath)
		{
			if (strPath.size() == 0)
				return false;
			int i = (int)strPath.size() - 1;
			for (; i >= 0 && (strPath[i] == '\\' || strPath[i] == '/'); i--)
				;
			std::string str = strPath.substr(0, i + 1);

			QDir dir(QString::fromLocal8Bit(str.c_str()));
			return dir.exists();
		}

		static inline std::string GetWkDir()
		{
			QDir dir;
			return dir.currentPath().toLocal8Bit();
		}
		static inline void SetWkDir(CStr& dir)
		{
			QDir tmp;
			tmp.setCurrent(QString::fromLocal8Bit(dir.c_str()));
		}

		static bool MkDir(CStr&  path);

		// Eg: RenameImages("D:/DogImages/*.jpg", "F:/Images", "dog", ".jpg");
		static int Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt);

		static void RmFile(CStr& fileW);
		static void RmFolder(CStr& dir)
		{
			QDir tmp;
			tmp.rmdir(QString::fromLocal8Bit(dir.c_str()));
		}
		static void CleanFolder(CStr& dir, bool subFolder = false);

		static int GetSubFolders(CStr& folder, vecS& subFolders);

		static bool Copy(CStr &src, CStr &dst)
		{
			QFile f;
			return f.copy(QString::fromLocal8Bit(src.c_str()), QString::fromLocal8Bit(dst.c_str()));
		}
		static bool Move(CStr &src, CStr &dst)
		{
			QFile f;
			return f.copy(QString::fromLocal8Bit(src.c_str()), QString::fromLocal8Bit(dst.c_str()));
		}
		static bool Move2Dir(CStr &srcW, CStr dstDir);
		static bool Copy2Dir(CStr &srcW, CStr dstDir);

		static void RunProgram(CStr &fileName, CStr &parameters = "", bool waiteF = false, bool showW = true);
	private:
		static std::shared_ptr<QApplication> _app;
#endif
#endif
	};
}
#endif

#ifdef _YXL_PARAM_PARSER_
namespace YXL
{
	namespace CMD
	{
		typedef bool(*CmdLineParserCallback)(const std::string& name, const std::string& val);
		//argv: -name=val
		/*
		std::multimap<std::string, std::string> params;
		bool ParserCallback(const std::string& name, const std::string& val)
		{
		params.insert(std::make_pair(name, val));
		return true;
		}
		CmdLineParser(argc, argv, ParserCallback);

		or

		std::map < std::string, std::string > params;
		CmdLineParser(argc, argv, [&params](std::string& key, std::string& val) {params[key]=val;});

		*/
		template <typename Callback> 
		void CmdLineParser(int argc, char** argv, Callback callback)
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

		inline std::map<std::string, std::string> ParseCmdLine(int argc, char** argv)
		{
			std::map<std::string, std::string> ret;
			CmdLineParser(argc, argv, [&ret](std::string& key, std::string& val) {ret[key] = val; });
			return ret;
		}
		inline std::map<std::string, std::vector<std::string> > ParseCmdLineDuplicate(int argc, char** argv)
		{
			std::map<std::string, std::vector<std::string> > ret;
			CmdLineParser(argc, argv, [&ret](std::string& key, std::string& val) {ret[key].push_back(val); });
			return ret;
		}
	}
}
#endif

#ifdef _YXL_PRINT_
namespace YXL
{
	template<typename type> void _print(type t, const int i)
	{
		std::cout << '\t' << t[i];
	}
	template<typename type> void _print2(type t, const int i)
	{
		if (i)
			std::cout << ',';
		std::cout <<t[i];
	}

	template<typename... type> void PrintVector(const int vec_len, const type*... vecs)
	{
		const int arg_cnt = sizeof...(vecs);
		std::cout << "[";
		for (int i(0); i != vec_len; ++i)
		{
			if (i)
				std::cout << ",";
			if(arg_cnt>1)
				std::cout << "(";
			int tmp[] = { (_print2(vecs, i), 0)... };
			if(arg_cnt>1)
				std::cout << ")";
		}
		std::cout << "]" << std::endl;
	}
	
	template<typename... type> void PrintVectorAsRow(int vec_len, type*... vecs)
	{
		for (int i(0); i != vec_len; ++i)
		{
			std::cout << i << ":";
			int tmp[] = { (_print(vecs, i), 0)... };
			std::cout << "\n";
		}
		
	}

	template<typename type> void PrintVector(std::vector<type>& v)
	{
		PrintVector(v.size(), v.data());
	}
	template<typename type> void PrintVectorAsRow(std::vector<type>& v)
	{
		PrintVectorAsRow(v.size(), v.data());
	}

	template<typename key, typename val> void PrintMapAsRows(std::map<key, val>& m, const std::string& padding = "")
	{
		for (auto iter = m.begin(); iter != m.end(); ++iter)
			YXL::yxlout << padding << iter->first << '\t' << iter->second << "\n";
	}
}
#endif

#ifdef _YXL_OUT_STREAM_
namespace YXL
{
	template<typename _Elem, typename _Traits>
	class YXLOutStream
	{
	public:
		typedef std::basic_ostream<_Elem, _Traits> _Myt;
		typedef std::basic_ios<_Elem, _Traits> _Myios;
		typedef std::basic_streambuf<_Elem, _Traits> _Mysb;
		typedef std::ostreambuf_iterator<_Elem, _Traits> _Iter;
		typedef std::num_put<_Elem, _Iter> _Nput;

		void Lock()
		{
			_mu.lock();
		}
		void Unlock()
		{
			_mu.unlock();
		}

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
		std::mutex _mu;
		std::string _log_file = "";
	};

	typedef YXLOutStream<char, std::char_traits<char> > YXLOut;

#if defined(_WITH_WINDOWS_) || defined(__linux__)
#define YXL_LOG_PREFIX YXL::GetCurrentThreadID()<<"["<<__FUNCTION__<<"] "
#else
#define YXL_LOG_PREFIX "["<<__FUNCTION__<<"] "
#endif
	extern YXLOut yxlout;
}
#endif

#ifdef _YXL_UNION_FIND_
namespace YXL
{
	class LIB_YXL_HELPER UnionFind
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

		int GroupCount() const
		{
			return _group_cnt;
		}
		bool IsConnected(const int a, const int b) const
		{
			return Find(a) == Find(b);
		}
		int Find(int a) const
		{
			while (a != _id[a])
				a = _id[a];
			return a;
		}
		void Union(int a, int b);
		void Update();

		int GroupID(const int a) const
		{
			return (0 <= a&&a < _group_id.size()) ? _group_id[a] : -1;
		}

		const std::vector<int>* Group(const int group_id)
		{
			return (0 <= group_id && group_id < _group_cnt) ? &(_groups[group_id]) : nullptr;
		}
		
	private:
		int _group_cnt;
		std::vector<int> _id;
		std::map<int, int> _group_size;
		
		std::vector<int> _group_id;
		std::map<int, std::vector<int> > _groups;
	};
}
#endif

#ifdef _YXL_KD_TREE_
namespace YXL
{
	template<typename type, int dim=3>
	class KDTree
	{
		struct Element
		{
			int idx;
			type val[dim];
		};
		struct Node
		{
			~Node()
			{
				if (left)
					delete left;
				if (right)
					delete right;
			}
			Element val;
			int split_dim;
			Node *left = nullptr;
			Node *right = nullptr;
		};
	public:
		~KDTree()
		{
			if (_root)
				delete _root;
		}

		void Bulid(const type* data, const int ele_cnt)
		{
			std::vector<Element> ele(ele_cnt);
			for (int i(0); i != ele_cnt; ++i)
			{
				auto& e = ele[i];
				e.idx = i;
				memcpy(e.val, data + i*dim, sizeof(type)*dim);
			}

			_root = Build(&ele[0], ele_cnt, 0);
		}

		int FindNearest(type* ret, const type* ele)
		{
			Element _ele, _ret;
			memcpy(_ele.val, ele, sizeof(Element));
			double min_dist = (std::numeric_limits<double>::max)();
			FindNearest(_ret, _ele, _root, min_dist);
			memcpy(ret, _ret.val, sizeof(Element));
			return _ret.idx;
		}

	private:
		Node* Build(Element* _data, const int ele_cnt, const int split_dim)
		{
			std::sort(_data, _data + ele_cnt, 
				[split_dim](const Element& a, const Element& b) {return a.val[split_dim] < b.val[split_dim]; });

			int mid = ele_cnt / 2;
			while (mid + 2 < ele_cnt && _data[mid].val[split_dim] == _data[mid + 1].val[split_dim])
				++mid;

			Node* node = new Node;
			node->val = _data[mid];
			node->split_dim = split_dim;

			int next_split_dim = (split_dim + 1) % dim;
			if (mid > 0)
				node->left = Build(_data, mid, next_split_dim);
			if(mid<ele_cnt-1)
				node->right = Build(_data+mid+1, ele_cnt-mid-1, next_split_dim);
			return node;
		}
		void FindNearest(Element& ret, const Element& ele, Node* node, double& min_dist)
		{
			if (node == nullptr)
				return;
			Element diff;
			for (int i(dim); i--;)
				diff.val[i] = ele.val[i] - node->val.val[i];
			double cur_dist = YXL::Dot<dim>(diff.val, diff.val);
			if (cur_dist < min_dist)
			{
				min_dist = cur_dist;
				ret = node->val;
			}
			FindNearest(ret, ele,
				(ele.val[node->split_dim] <= node->val.val[node->split_dim]) ? node->left : node->right,
				min_dist);
			double range = ele.val[node->split_dim] - node->val.val[node->split_dim];
			if (abs(range) > min_dist)
				return;
			FindNearest(ret, ele,
				(range < 0 ? node->right : node->left),
				min_dist);
		}

	private:
		Node* _root=nullptr;
	};
}
#endif

#ifdef _YXL_TIME_
#ifdef __linux__
#include <system.h>
#endif
namespace YXL
{
	inline std::string GetCurTime(const char* format_ymd_hms)
	{
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm* ptm = localtime(&tt);
		char date[60] = { 0 };
		sprintf(date, format_ymd_hms,
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
		return std::string(date);
	}

	inline std::string GetCurTime()
	{
		return GetCurTime("%04d%02d%02d_%02d%02d%02d");
	}

	inline void Sleep(double ms)
	{
#if defined(_WITH_WINDOWS_)
		::Sleep(ms);
#elif defined(__linux__)
		usleep(ms*1000.);
#else
		std::cout << "no sleep function" << std::endl;
#endif
	}

	template<typename type> double TimeElapsedMS(type start, type end)
	{
		using namespace std::chrono;
		auto d = duration_cast<microseconds>(end - start);
		return double(d.count()*1000.)*microseconds::period::num / microseconds::period::den;
	}

	class LIB_YXL_HELPER Timer
	{
	public:
		void Reset()
		{
			_str_t_start.clear();
			_int_t_start.clear();
			_str_acc.clear();
			_int_acc.clear();
			_last = std::chrono::high_resolution_clock::now();
		}

		void Start(const std::string& stamp)
		{
			_str_t_start[stamp] = std::chrono::high_resolution_clock::now();
		}
		void Start(const int stamp)
		{
			_int_t_start[stamp] = std::chrono::high_resolution_clock::now();
		}
		double End(const std::string& stamp)
		{
			if (_str_t_start.find(stamp) == _str_t_start.end())
				return 0.;
			return TimeElapsedMS(_str_t_start[stamp], std::chrono::high_resolution_clock::now());
		}
		double EndAverage(const std::string& stamp)
		{
			auto ret = End(stamp);
			return TimeElapsedAverage(ret, _str_acc[stamp]);
			
		}
		double End(const int stamp)
		{
			if (_int_t_start.find(stamp) == _int_t_start.end())
				return 0.;
			return TimeElapsedMS(_int_t_start[stamp], std::chrono::high_resolution_clock::now());
		}
		double EndAverage(const int stamp)
		{
			auto ret = End(stamp);
			return TimeElapsedAverage(ret, _int_acc[stamp]);
		}

		//time escape from last call
		double TimeEscape()
		{
			auto tmp = std::chrono::high_resolution_clock::now();
			auto ret = TimeElapsedMS(_last, tmp);
			_last = tmp;
			return ret;
		}

	private:
		double TimeElapsedAverage(const double cur_escape, std::pair<int, double>& history)
		{
			double ret = (history.first*history.second + cur_escape) / (history.first + 1);
			++history.first;
			history.second = ret;
			return ret;
		}

	private:
		std::map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock> > _str_t_start;
		std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock> > _int_t_start;

		std::map<std::string, std::pair<int, double> > _str_acc;
		std::map<int, std::pair<int, double> > _int_acc;

		std::chrono::time_point<std::chrono::high_resolution_clock> _last;
	};

	extern Timer g_timer;

	class LIB_YXL_HELPER FPSCounter
	{
	public:
		FPSCounter(const int measure_frame=1)
		{
			_measure_frame = measure_frame;
			_last = std::chrono::high_resolution_clock::now();
		}

		//calculate current FPS
		double CalcFPS()
		{
			auto cur = std::chrono::high_resolution_clock::now();
			auto ms = TimeElapsedMS(_last, cur);
			_last = cur;
			_pre_frame_time_ms.push_back(ms);
			_total += ms;
			
			if (_pre_frame_time_ms.size() > _measure_frame)
			{
				_total -= _pre_frame_time_ms.front();
				_pre_frame_time_ms.pop_front();
			}
			
			_fps = 1000.*_pre_frame_time_ms.size() / _total;
			return _fps;
		}

		//not update, get last calculated FPS
		double GetFPS()
		{
			return _fps;
		}

	private:
		int _measure_frame;
		double _total = 0.;
		double _fps = 0.;
		std::deque<double> _pre_frame_time_ms;
		std::chrono::time_point<std::chrono::high_resolution_clock> _last;

	};

	class LIB_YXL_HELPER FPSLimiter
	{
	public:
		FPSLimiter() {}
		FPSLimiter(const double fps)
		{
			SetFPS(fps);
		}
		void SetFPS(const double fps)
		{
			_fps = fps;
			_ms_per_frame = 1000. / fps;
		}

		void Start()
		{
			_last = std::chrono::high_resolution_clock::now();
		}
		void End()
		{
			auto ms = TimeElapsedMS(_last, std::chrono::high_resolution_clock::now());
			if (ms < _ms_per_frame)
				Sleep(_ms_per_frame - ms);
		}

	private:
		double _fps = 30.;
		double _ms_per_frame=1000./30.;
		std::chrono::time_point<std::chrono::high_resolution_clock> _last;
	};
}
#endif

#ifdef _YXL_CONSOLE_
namespace YXL
{
	namespace Console
	{
		inline void ClearLine(const int idx)
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;

			COORD pos;
			pos.X = 0;
			pos.Y = idx;
			DWORD written(0);
			FillConsoleOutputCharacterA(hStdout, ' ', csbiInfo.dwSize.X, pos, &written);
		}
		inline void ClearCurrentLine()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;
			COORD pos = csbiInfo.dwCursorPosition;
			pos.X = 0;
			DWORD written(0);
			FillConsoleOutputCharacterA(hStdout, ' ', csbiInfo.dwSize.X, pos, &written);
		}
		inline void ClearAll()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;

			COORD line;
			line.X = 0;
			line.Y = 0;
			DWORD written(0);
			FillConsoleOutputCharacterA(hStdout, ' ', csbiInfo.dwSize.X*csbiInfo.dwSize.Y, line, &written);
		}
		inline void RedirectTo(const int x, const int y)
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;

			COORD pos;
			pos.Y = x;
			pos.X = y;
			if (!SetConsoleCursorPosition(hStdout, pos))
				return;
		}

		inline void RedirectToFirstLine(bool is_clear=true)
		{
			if(is_clear)
				ClearAll();
			RedirectTo(0, 0);
		}
		
		inline int CurrentLine()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return -1;
			return csbiInfo.dwCursorPosition.Y;
		}
	}
}
#endif

#ifdef _YXL_TRANSFORM_
namespace YXL
{
	namespace Vec
	{
		template<typename type> void Cross(type* v, const type* v0, const type* v1)
		{
			v[0] = v0[1] * v1[2] - v0[2] * v1[1];
			v[1] = v0[2] * v1[0] - v0[0] * v1[2];
			v[2] = v0[0] * v1[1] - v0[1] * v1[0];
		}
		template<int len, typename type> type Dot(const type* v0, const type* v1)
		{
			type ret = 0;
			for (int i(len); i--;)
				ret += v0[i] * v1[i];
			return ret;
		}
		template<int len, typename type> void Normalize(type* v, const type* v0)
		{
			type norm = static_cast<type>(1) / sqrt(Dot<len>(v0, v0));
			for (int i(len); i--; )
				v[i] = v0[i] * norm;
		}
	}

	namespace Quat
	{
		template<typename type> void Rotate(type* out, const type* v, const type* quat)
		{
			float a[3], b[3];
			Vec::Cross(a, quat, v);
			Vec::Cross(b, quat, a);
			out[0] = v[0] + static_cast<type>(2)*(a[0] * quat[3] + b[0]);
			out[1] = v[1] + static_cast<type>(2)*(a[1] * quat[3] + b[1]);
			out[2] = v[2] + static_cast<type>(2)*(a[2] * quat[3] + b[2]);
		}
		template<typename type> void ToQuaternion(type* out, const type* axis, const type angle_degree)
		{
			type angle = angle_degree*(3.1415926*0.5 / 180);
			type c = std::cos(angle);
			type s = std::sin(angle);

			type norm = static_cast<type>(1) / Vec::Dot<3>(axis, axis);

			out[0] = axis[0] * norm*s;
			out[1] = axis[1] * norm*s;
			out[2] = axis[2] * norm*s;
			out[3] = c;
		}
	}

	namespace Mat
	{
		template<typename type> void RotationPoint(type* pt_ret, type* pt, type* rot)
		{
			if (pt_ret == pt)
			{
				type x = pt[0], y = pt[1], z = pt[2];
				pt_ret[0] = x*rot[0] + y*rot[4] + z*rot[8];
				pt_ret[1] = x*rot[1] + y*rot[5] + z*rot[9];
				pt_ret[2] = x*rot[2] + y*rot[6] + z*rot[10];
			}
			else
			{
				pt_ret[0] = pt[0]*rot[0] + pt[1]*rot[4] + pt[2]*rot[8];
				pt_ret[1] = pt[0]*rot[1] + pt[1]*rot[5] + pt[2]*rot[9];
				pt_ret[2] = pt[0]*rot[2] + pt[1]*rot[6] + pt[2]*rot[10];
			}
		}

		template<typename type> void Perspective(type* ret, type aspect, type z_near, type z_far, type fov)
		{
			type tan_half_fov = tan(fov*0.5f / 180.0f*3.1415926f);

			memset(ret, 0, sizeof(type) * 16);
			ret[0] = 1.0f / (tan_half_fov*aspect);
			ret[5] = 1.0f / tan_half_fov;
			ret[10] = -(z_near + z_far) / (z_far - z_near);
			ret[11] = -1.0f;
			ret[14] = -2.0f*z_far*z_near / (z_far - z_near);

			/*
			{
			1.0f / (tan_half_fov*aspect), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / tan_half_fov, 0.0f, 0.0f,
			0.0f, 0.0f, -(z_near + z_far) / (z_far - z_near), -1.0f,
			0.0f, 0.0f, -2.0f*z_far*z_near / (z_far - z_near), 0.0f
			};
			*/
		}

		template<typename type> void LookAt(type* ret, type* eye, type* at, type* up)
		{
			type f[] = { at[0] - eye[0], at[1] - eye[1], at[2] - eye[2] };
			Vec::Normalize<3>(f, f);

			type s[3];
			Vec::Cross(s, f, up);
			Vec::Normalize<3>(s, s);

			type u[3];
			Vec::Cross(u, s, f);

			ret[0] = s[0];
			ret[1] = s[1];
			ret[2] = s[2];
			ret[3] = 0;
			ret[4] = u[0];
			ret[5] = u[1];
			ret[6] = u[2];
			ret[7] = 0;
			ret[8] = f[0];
			ret[9] = f[1];
			ret[10] = f[2];
			ret[11] = 0;
			ret[12] = -Vec::Dot<3>(s, eye);
			ret[13] = -Vec::Dot<3>(u, eye);
			ret[14] = Vec::Dot<3>(f, eye);
			ret[15] = 1.0f;

			/*float data[] = {
			s[0], s[1], s[2], 0.0f,
			u[0], u[1], u[2], 0.0f,
			f[0], f[1], f[2], 0.0f,
			-s.dot(eye), -u.dot(eye), f.dot(eye), 1.0f
			};*/
		}

		template<typename type> void RotationX(type* ret, type angle)
		{
			memset(ret, 0, sizeof(type) * 16);
			float theta = angle*3.1415926f / 180.0f;
			ret[0] = 1.f;
			ret[5] = cos(theta);
			ret[6] = -sin(theta);
			ret[9] = sin(theta);
			ret[10] = cos(theta);
			ret[15] = 1.f;
			/*
			{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cos(theta), -sin(theta), 0.0f,
			0.0f, sin(theta), cos(theta), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			};
			*/
		}
		template<typename type> void RotationY(type* ret, type angle)
		{
			memset(ret, 0, sizeof(type) * 16);
			float theta = angle*3.1415926f / 180.0f;
			ret[0] = cos(theta);
			ret[2] = -sin(theta);
			ret[5] = 1.f;
			ret[8] = sin(theta);
			ret[10] = cos(theta);
			ret[15] = 1.f;
			/*
			{
			cos(theta), 0.0f, -sin(theta), 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sin(theta), 0.0f, cos(theta), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			};
			*/
		}
		template<typename type> void RotationZ(type* ret, type angle)
		{
			memset(ret, 0, sizeof(type) * 16);
			float theta = angle*3.1415926f / 180.0f;
			ret[0] = cos(theta);
			ret[1] = -sin(theta);
			ret[4] = sin(theta);
			ret[5] = cos(theta);
			ret[10] = 1.f;
			ret[15] = 1.f;
			/*
			{
			cos(theta), -sin(theta), 0.0f, 0.0f,
			sin(theta), cos(theta), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			};
			*/
		}
		template<typename type> void RotationFromQuaternion(type* ret, type* quat)
		{
			type q[4];
			Vec::Normalize<4, type>(q, quat);

			type xy = q[0]*q[1];
			type yz = q[1]*q[2];
			type zx = q[2]*q[0];
			type x2 = q[0]*q[0];
			type y2 = q[1]*q[1];
			type z2 = q[2]*q[2];
			type xw = q[0]*q[3];
			type yw = q[1]*q[3];
			type zw = q[2]*q[3];

			ret[0] = 1.f - 2.f*(y2 + z2);
			ret[4] = 2.f*(xy - zw);
			ret[8] = 2.f*(zx + yw);
			ret[1] = 2.f*(xy + zw);
			ret[5] = 1.f - 2.f*(x2 + z2);
			ret[9] = 2.f*(yz - xw);
			ret[2] = 2.f*(zx - yw);
			ret[6] = 2.f*(yz + xw);
			ret[10] = 1.f - 2.f*(x2 + y2);
			ret[15] = 1.f;
		}

		template<typename type> void Scale(type* ret, type* scale_xyz)
		{
			memset(ret, 0, sizeof(type) * 16);
			ret[0] = scale_xyz[0];
			ret[5] = scale_xyz[1];
			ret[10] = scale_xyz[2];
			ret[15] = 1.0f;
		}
		template<typename type> void Translate(type* ret, type* trans_xyz)
		{
			memset(ret, 0, sizeof(type) * 16);
			ret[0] = 1.0f;
			ret[5] = 1.0f;
			ret[10] = 1.0f;
			ret[12] = trans_xyz[0];
			ret[13] = trans_xyz[1];
			ret[14] = trans_xyz[2];
			ret[15] = 1.0f;
		}


#ifdef _WITH_OPENCV_
		inline cv::Mat Perspective(float aspect, float z_near, float z_far, float fov)
		{
			float proj_mat_data[16];
			Perspective(proj_mat_data, aspect, z_near, z_far, fov);
			return cv::Mat(4, 4, CV_32FC1, proj_mat_data).clone();
		}

		inline cv::Mat LookAt(cv::Vec3f eye, cv::Vec3f at, cv::Vec3f up)
		{
			float data[16];
			LookAt(data, eye.val, at.val, up.val);
			return cv::Mat(4, 4, CV_32FC1, data).clone();
		}

		inline cv::Mat Rotation(cv::Vec3f rot_xyz)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			float tmp[16];
			if (rot_xyz[0] != 0.0)
			{
				RotationX(tmp, rot_xyz[0]);
				ret *= cv::Mat(4, 4, CV_32FC1, tmp);
			}
			if (rot_xyz[1] != 0.0)
			{
				RotationY(tmp, rot_xyz[1]);
				ret *= cv::Mat(4, 4, CV_32FC1, tmp);
			}
			if (rot_xyz[2] != 0.0)
			{
				RotationZ(tmp, rot_xyz[2]);
				ret *= cv::Mat(4, 4, CV_32FC1, tmp);
			}

			return ret;
		}

		inline cv::Mat RotationFromQuaternion(cv::Vec4f quat)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			RotationFromQuaternion((float*)ret.data, quat.val);
			return ret;
		}

		inline cv::Mat Scale(cv::Vec3f scale_xyz)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			Scale((float*)ret.data, scale_xyz.val);
			return ret;
		}

		inline cv::Mat Translate(cv::Vec3f trans_xyz)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			Translate((float*)ret.data, trans_xyz.val);
			return ret;
		}
#endif
	}
}
#endif

#ifdef _YXL_GRAPHIC_
namespace YXL
{
	template<typename v_type, typename i_type> void ComputeNormal(v_type* normals, 
		v_type* vertices, const int v_cnt, 
		i_type* tris, const int tri_cnt, const bool is_reverse_normal)
	{
		memset(normals, 0, sizeof(v_type)*v_cnt * 3);
		i_type idx[3];
		int tmp;
		v_type a[3], b[3], c[3], l2[3], face_normal[3];
		for (int i(0); i!= tri_cnt; ++i)
		{
			tmp = i * 3;
			idx[0] = tris[tmp + (is_reverse_normal ? 2 : 0)]*3;
			idx[1] = tris[tmp + 1]*3;
			idx[2] = tris[tmp + (is_reverse_normal ? 0 : 2)]*3;

			a[0] = vertices[idx[0]] - vertices[idx[1]];
			a[1] = vertices[idx[0] + 1] - vertices[idx[1] + 1];
			a[2] = vertices[idx[0] + 2] - vertices[idx[1] + 2];
			b[0] = vertices[idx[1]] - vertices[idx[2]];
			b[1] = vertices[idx[1] + 1] - vertices[idx[2] + 1];
			b[2] = vertices[idx[1] + 2] - vertices[idx[2] + 2];
			c[0] = vertices[idx[2]] - vertices[idx[0]];
			c[1] = vertices[idx[2] + 1] - vertices[idx[0] + 1];
			c[2] = vertices[idx[2] + 2] - vertices[idx[0] + 2];
			
			l2[0] = Vec::Dot<3>(a, a);
			l2[1] = Vec::Dot<3>(b, b);
			l2[2] = Vec::Dot<3>(c, c);
			Vec::Cross(face_normal, a, b);

			a[0] = 1.f / (l2[0] * l2[2]);
			a[1] = 1.f / (l2[0] * l2[1]);
			a[2] = 1.f / (l2[1] * l2[2]);

			tmp = idx[0];
			normals[tmp] += face_normal[0] * a[0];
			normals[tmp+1] += face_normal[1] * a[0];
			normals[tmp+2] += face_normal[2] * a[0];
			tmp = idx[1];
			normals[tmp] += face_normal[0] * a[1];
			normals[tmp + 1] += face_normal[1] * a[1];
			normals[tmp + 2] += face_normal[2] * a[1];
			tmp = idx[2];
			normals[tmp] += face_normal[0] * a[2];
			normals[tmp + 1] += face_normal[1] * a[2];
			normals[tmp + 2] += face_normal[2] * a[2];
		}
		for (int i(v_cnt); i--;)
			Vec::Normalize<3>(normals + i * 3, normals + i * 3);
	}

#ifdef _WITH_OPENCV_
	inline cv::Mat ComputeNormal(cv::Mat vertices, cv::Mat tris, bool is_reverse_normal)
	{
		if (vertices.channels() != 3 || false == (vertices.depth() == CV_32F || vertices.depth() == CV_64F) || tris.type() != CV_32SC3)
			return cv::Mat();
		cv::Mat normals = cv::Mat(vertices.size(), vertices.type());
		if(vertices.depth()==CV_32F)
			ComputeNormal((float*)normals.data, (float*)vertices.data, vertices.rows, (int*)tris.data, tris.rows, is_reverse_normal);
		else
			ComputeNormal((double*)normals.data, (double*)vertices.data, vertices.rows, (int*)tris.data, tris.rows, is_reverse_normal);
		return normals;
	}
#endif
}
#endif

#ifdef _YXL_IMG_PROC_
namespace YXL
{
	cv::Mat LIB_YXL_HELPER FilterImage(cv::Mat img, cv::Mat kernel, cv::Mat mask = cv::Mat());
}
#endif

#ifdef _YXL_IMG_CODEC_
namespace YXL
{
	namespace Image
	{
		//encode

#define DECLARE_ALL(marco) \
		marco(RGBA);\
		marco(BGRA);\
		marco(RGB);\
		marco(BGR);\
		marco(Grey);

#define DECODE_FUNC(name) void Decode##name(std::shared_ptr<unsigned char>& img, int& w, int& h, CStr& in_data);
		DECLARE_ALL(DECODE_FUNC)
#undef DECODE_FUNC
		//decode

#define ENCODE_FUNC(name) void EncodePNG_##name(std::shared_ptr<unsigned char>& out_data, int& out_data_size, unsigned char* img, const int w, const int h);
		DECLARE_ALL(ENCODE_FUNC)
#undef ENCODE_FUNC

		//webp
#define DECODE_FUNC_WEBP(name) void DecodeWebP_##name(std::shared_ptr<unsigned char>& img, int& w, int& h, CStr& in_data);
		DECLARE_ALL(DECODE_FUNC_WEBP);
#undef DECODE_FUNC_WEBP

#define ENCODE_FUNC_WEBP(name) void EncodeWebp_##name(std::shared_ptr<unsigned char>& out_data, int& out_data_size, unsigned char* img, const int w, const int h, const float quality);
		// @quality: 
		//	[0-100): lossy
		//	>=100: lossless
		DECLARE_ALL(ENCODE_FUNC_WEBP);
#undef ENCODE_FUNC_WEBP

#undef DECLARE_ALL
	}
}
#endif

#ifdef _YXL_MINI_Z_
#include <sstream>
namespace YXL
{
	namespace ZIP
	{
		enum struct ZIP_COMPRESSION
		{
			NO_COMPRESSION = 0,
			LEVEL_0,
			LEVEL_1,
			LEVEL_2,
			LEVEL_3,
			LEVEL_4,
			LEVEL_5,
			LEVEL_6,
			LEVEL_7,
			LEVEL_8,
			LEVEL_9,
			LEVEL_10,
			BEST_SPEED,
			BEST_COMPRESSION,
			UBER_COMPRESSION,
			DEFAULT_LEVEL,
		};
		class File
		{
		public:
			File(CStr& fn, CStr& data) :fn(fn), _data(data) {}
			File(CStr& fn, std::shared_ptr<const char> data, const size_t size):fn(fn), _data2(data), _data2_size(size){}

			const char* GetDataPtr()
			{
				return (_data2 == nullptr) ? _data.c_str() : _data2.get();
			}
			size_t GetDataSize()
			{
				return (_data2 == nullptr) ? _data.length() : _data2_size;
			}


#define READ_FUNC(Name, type) type Read##Name(){CheckData();type out;(*_ss)>>out; return out;}
			READ_FUNC(Int, int);
			READ_FUNC(Float, float);
			READ_FUNC(Double, double);
			READ_FUNC(String, std::string);
			READ_FUNC(Char, char);
#undef READ_FUNC
			
		private:
			void CheckData()
			{
				size_t size = GetDataSize();
				const char* ptr = GetDataPtr();

				std::string tmp = std::to_string((long long)ptr) + std::to_string(size);
				if (tmp == _data_id)
					return;
				_ss = std::shared_ptr<std::stringstream>(new std::stringstream);
				_ss->write(ptr, size);
				//(*_ss) << data;
				_data_id = tmp;
			}
		private:
			std::string fn;
			std::string _data;
			std::shared_ptr<const char> _data2 = nullptr;
			size_t _data2_size=0;

		private:
			std::shared_ptr<std::stringstream> _ss = nullptr;
			std::string _data_id;
		};

		class Zip
		{
		public:
			Zip(std::shared_ptr<const char> data, const size_t size, const bool is_unzip);
			Zip(const std::string& content, const bool is_unzip);

			bool IsFine() const;
			bool ToZip(std::shared_ptr<char>& zip, size_t& zip_size, const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);

			void AddFile(const std::string& fn, std::shared_ptr<const char> data, const size_t size);
			void AddFile(const std::string& fn, const std::string& content);

			void GetFiles(std::multimap<std::string, std::shared_ptr<File>>& files);
			void GetFiles(std::multimap<std::string, std::string>& files);
			void GetFiles(std::map<std::string, std::shared_ptr<File>>& files);
			void GetFiles(std::map<std::string, std::string>& files);

		public:
			static bool Unzip(std::multimap<std::string, std::shared_ptr<File>>& out_files, const char* zip, const size_t zip_size, const bool is_fn_lowercase = false);
			static bool RetrieveFiles(std::multimap<std::string, std::shared_ptr<File>>& out_files, CStr& zip_content, const std::vector<std::string>& files_to_get, const bool is_fn_lowercase = false);
			static bool ToZip(std::shared_ptr<char>& zip, size_t& zip_size,
				const std::map<std::string, std::string>& fn_data,
				const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);
			static bool ToZip(std::shared_ptr<char>& zip, size_t& zip_size,
				const std::multimap<std::string, std::shared_ptr<File>>& files,
				const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);
			static bool ZipAddFile(std::shared_ptr<char>& zip, size_t& zip_size,
				const char* in_zip, const size_t in_zip_size,
				const std::map<std::string, std::string>& fn_data,
				const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);
			static bool ZipAddFile(std::shared_ptr<char>& zip, size_t& zip_size,
				const char* in_zip, const size_t in_zip_size, std::multimap<std::string, std::shared_ptr<File>>& files,
				const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);

		private:
			static bool ToZip(std::shared_ptr<char>& zip, size_t& zip_size,
				const std::map<std::string, std::pair<const char*, size_t>>& fn_data,
				const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);
			static bool ZipAddFile(std::shared_ptr<char>& zip, size_t& zip_size,
				const char* in_zip, const size_t in_zip_size,
				const std::map<std::string, std::pair<const char*, size_t>>& fn_data,
				const ZIP_COMPRESSION compression = ZIP_COMPRESSION::DEFAULT_LEVEL);

		private:
			std::shared_ptr<const char> _data;
			size_t _size;
			std::string _content;

			//
			bool _is_fine = true;
			std::multimap<std::string, std::shared_ptr<File>> _files;
		};
	}
}
#endif

#ifdef _YXL_ENCRYPT_DATA_

#include "tweetnacl/tweetnacl.h"
namespace YXL
{
	namespace Crypt
	{
		class EncryptorBase
		{
		public:
			virtual ~EncryptorBase() {}

			virtual void Encrypt(std::string& out, const char* in, const size_t in_size, std::string nonce) = 0;
			virtual void Decrypt(std::string& out, const char* in, const size_t in_size, std::string nonce) = 0;

		protected:
			void CheckKey(const int len, std::string& key, CStr& txt)
			{
				if (key.length() != len)
				{
					key.resize(len, ' ');
					std::cout << txt << " resize to \"" << key << "\"" << std::endl;
				}
			}
		};

		class DefSymEncryptor : public EncryptorBase
		{
			typedef Tweetnacl::u8 KeyType;
		public:
			DefSymEncryptor(CStr& key)
				:_key(key) 
			{
				CheckKey(crypto_secretbox_KEYBYTES, _key, "key");
			}

			void Encrypt(std::string& out, const char* in, const size_t in_size, std::string nonce)
			{
				CheckKey(crypto_secretbox_NONCEBYTES, nonce, "nonce");
				out.resize(in_size);
				Tweetnacl::crypto_secretbox((KeyType*)&out[0], (const KeyType*)in, in_size, (const KeyType*)nonce.c_str(), (const KeyType*)_key.c_str());
			}

			void Decrypt(std::string& out, const char* in, const size_t in_size, std::string nonce)
			{
				CheckKey(crypto_secretbox_NONCEBYTES, nonce, "nonce");
				out.resize(in_size);
				Tweetnacl::crypto_secretbox_open((KeyType*)&out[0], (const KeyType*)in, in_size, (const KeyType*)nonce.c_str(), (const KeyType*)_key.c_str());
			}

		private:
			std::string _key;
		};

		class DefAsymEncryptor : public EncryptorBase
		{
			typedef Tweetnacl::u8 KeyType;
		public:
			DefAsymEncryptor(CStr& sec_key_A, CStr& pub_key_B)
				:_sec_key_A(sec_key_A), _pub_key_B(pub_key_B)
			{
				CheckKey(crypto_box_PUBLICKEYBYTES, _pub_key_B, "public key");
				CheckKey(crypto_box_SECRETKEYBYTES, _sec_key_A, "secret key");
				_k.resize(crypto_box_BEFORENMBYTES);
				Tweetnacl::crypto_box_beforenm((KeyType*)&_k[0], (const KeyType*)_pub_key_B.c_str(), (const KeyType*)_sec_key_A.c_str());
			}

			static void GenKeyPair(std::string& pk, std::string& sk)
			{
				sk.resize(crypto_box_SECRETKEYBYTES);
				pk.resize(crypto_box_PUBLICKEYBYTES);
				Tweetnacl::crypto_box_keypair((KeyType*)&pk[0], (KeyType*)&sk[0]);
			}

			void Encrypt(std::string& out, const char* in, const size_t in_size, std::string nonce)
			{
				CheckKey(crypto_secretbox_NONCEBYTES, nonce, "nonce");
				out.resize(in_size);
				Tweetnacl::crypto_box_afternm((KeyType*)&out[0], (const KeyType*)in, in_size, (const KeyType*)nonce.c_str(), (const KeyType*)_k.c_str());
			}
			void Decrypt(std::string& out, const char* in, const size_t in_size, std::string nonce)
			{
				CheckKey(crypto_secretbox_NONCEBYTES, nonce, "nonce");
				out.resize(in_size);
				Tweetnacl::crypto_box_open_afternm((KeyType*)&out[0], (const KeyType*)in, in_size, (const KeyType*)nonce.c_str(), (const KeyType*)_k.c_str());
			}
			
		private:
			std::string _sec_key_A;
			std::string _pub_key_B;
			std::string _k;
		};

		class SignerBase
		{
		public:
			virtual ~SignerBase() {}

			virtual void Sign(std::string& out, const char* in, const size_t in_size) = 0;
			virtual bool Verify(std::string&out, const char* in, const size_t in_size) = 0;

		protected:
			void CheckKey(const int len, std::string& key, CStr& txt)
			{
				if (key.length() != len)
				{
					key.resize(len, ' ');
					std::cout << txt << " resize to \"" << key << "\"" << std::endl;
				}
			}
		};

		class DefSigner : public SignerBase
		{
			typedef Tweetnacl::u8 KeyType;
		public:
			DefSigner(CStr& sec_key_A, CStr& pub_key_B)
				:_sec_key_A(sec_key_A), _pub_key_B(pub_key_B)
			{
				CheckKey(crypto_sign_PUBLICKEYBYTES, _pub_key_B, "public key");
				CheckKey(crypto_sign_SECRETKEYBYTES, _sec_key_A, "secret key");
			}

			static void GenKeyPair(std::string& pk, std::string& sk)
			{
				sk.resize(crypto_sign_SECRETKEYBYTES);
				pk.resize(crypto_sign_PUBLICKEYBYTES);
				Tweetnacl::crypto_sign_keypair((KeyType*)&pk[0], (KeyType*)&sk[0]);
			}

			void Sign(std::string& out, const char* in, const size_t in_size)
			{
				out.resize(in_size + crypto_sign_BYTES);
				size_t smlen(0);
				Tweetnacl::crypto_sign((KeyType*)&out[0], &smlen, (const KeyType*)in, in_size, (KeyType*)_sec_key_A.c_str());
				out.resize(smlen);
			}

			bool Verify(std::string& out, const char* in, const size_t in_size)
			{
				out.resize(in_size);
				size_t mlen;
				int ret = Tweetnacl::crypto_sign_open((KeyType*)&out[0], &mlen, (const KeyType*)in, in_size, (KeyType*)_pub_key_B.c_str());
				out.resize(mlen);
				return 0 == ret;
			}

		private:
			std::string _sec_key_A;
			std::string _pub_key_B;
		};

		class CryptData
		{
		public:
			CryptData(CStr&);


		};
	}
}

#endif

#ifdef _YXL_HASH_
namespace YXL
{
	namespace Hash
	{
		std::string LIB_YXL_HELPER SHA1(CStr& str);
		//sha2
		std::string LIB_YXL_HELPER SHA224(CStr& str);
		std::string LIB_YXL_HELPER SHA256(CStr& str);
		std::string LIB_YXL_HELPER SHA384(CStr& str);
		std::string LIB_YXL_HELPER SHA512(CStr& str);

		std::string LIB_YXL_HELPER MD5(CStr& str);
	}
}
#endif

#ifdef _YXL_CIPHER_
namespace YXL
{
	namespace Cipher
	{
		/* key length */
		const int AES_128 = 16;
		const int AES_192 = 24;
		const int AES_256 = 32;

		//ECB (Electronic Code Book) mode
		void LIB_YXL_HELPER AESCipherECB(CStr& str, CStr& key, std::string& res);
		//ECB (Electronic Code Book) mode
		void LIB_YXL_HELPER AESDecryptECB(CStr& str, CStr& key, std::string& res);

		//TODO: CBC, CFB, OFB mode
	}
}

#endif





#ifdef _YXL_CRYPTO_

namespace YXL
{
	namespace Crypt
	{
		//rsa
		void RSAGenerateKey(std::string& private_key, std::string& public_key, CStr& seed, const unsigned int key_len);
		void RSAEncrypt(std::string& out, CStr& message, CStr& public_key, CStr& seed);
		void RSADecrypt(std::string& out, CStr& ciphertext, CStr& private_key);

		void RSAGenerateKey_File(CStr& fn_private_key, CStr& fn_public_key, CStr& seed, const unsigned int key_len);
		void RSAEncrypt_File(CStr& fn_out, CStr& fn_message, CStr& public_key, CStr& seed);
		void RSADecrypt_File(CStr& fn_out, CStr& fn_ciphertext, CStr& private_key);

		void RSASign(std::string& signature, CStr& message, CStr& private_key);
		bool RSAVerify(CStr& signature, CStr& message, CStr& public_key);

		void RSASign_File(std::string& signature, CStr& fn_message, CStr& private_key);
		bool RSAVerify_File(CStr& signature, CStr& fn_message, CStr& public_key);

		//digest
#define _Digest(name) std::string name(CStr& str)
#define _DigestFile(name) std::string name##_File(CStr& fn_in)
		_Digest(MD5);
		_Digest(SHA1);
		_Digest(SHA224);
		_Digest(SHA256);
		_Digest(SHA384);
		_Digest(SHA512);
		_Digest(SHA3_256);
		_Digest(SHA3_384);
		_Digest(SHA3_512);
		/// \brief Tiger message digest
		/// \sa <a href="http://www.cryptolounge.org/wiki/Tiger">Tiger</a>
		_Digest(Tiger);
		std::string Tiger(CStr& str);
		/// \brief RIPEMD-128 message digest
		/// \details Digest size is 128-bits.
		/// \warning RIPEMD-128 is considered insecure, and should not be used unless you absolutely need it for compatibility.
		/// \sa <a href="http://www.weidai.com/scan-mirror/md.html#RIPEMD-128">RIPEMD-128</a>
		_Digest(RIPEMD128);
		/// \brief RIPEMD-160 message digest
		/// \details Digest size is 160-bits.
		/// \sa <a href="http://www.weidai.com/scan-mirror/md.html#RIPEMD-160">RIPEMD-160</a>
		_Digest(RIPEMD160);
		/// \brief Whirlpool message digest
		/// \details Crypto++ provides version 3.0 of the Whirlpool algorithm.
		///   This version of the algorithm was submitted for ISO standardization.
		/// \sa <a href="http://www.cryptopp.com/wiki/Whirlpool">Whirlpool</a>
		_Digest(Whirlpool);
		/// \brief HMAC
		/// \tparam T HashTransformation derived class
		/// \details HMAC derives from MessageAuthenticationCodeImpl. It calculates the HMAC using
		///   <tt>HMAC(K, text) = H(K XOR opad, H(K XOR ipad, text))</tt>.
		/// \sa <a href="http://www.weidai.com/scan-mirror/mac.html#HMAC">HMAC</a>
		//hex_key: empty for computing HMAC/SHA1 value for self test 
		std::string HMAC(CStr& hex_key, CStr& str);
		_DigestFile(MD5);
		_DigestFile(SHA1);
		_DigestFile(SHA224);
		_DigestFile(SHA256);
		_DigestFile(SHA384);
		_DigestFile(SHA512);
		_DigestFile(SHA3_256);
		_DigestFile(SHA3_384);
		_DigestFile(SHA3_512);
		_DigestFile(Tiger);
		_DigestFile(RIPEMD128);
		_DigestFile(RIPEMD160);
		_DigestFile(Whirlpool);
		std::string HMAC_File(CStr& hex_key, CStr& fn_in);
#undef _Digest
#undef _DigestFile

		//AES
#define _EncryptIV(name) void name(std::string& out, CStr& str, CStr& hex_key, CStr& hex_iv);
#define _Encrypt(name) void name(std::string& out, CStr& str, CStr& hex_key);
#define _EncryptFileIV(name) void name##_File(CStr& fn_out, CStr& fn_in, CStr& hex_key, CStr& hex_iv);
#define _EncryptFile(name) void name##_File(CStr& fn_out, CStr& fn_in, CStr& hex_key);
		_EncryptIV(AESEncryptCFB);
		_EncryptIV(AESDecryptCFB);
		_EncryptIV(AESEncryptOFB);
		_EncryptIV(AESDecryptOFB);
		_EncryptIV(AESEncryptCTR);
		_EncryptIV(AESDecryptCTR);
		_Encrypt(AESEncryptECB);
		_Encrypt(AESDecryptECB);
		_EncryptIV(AESEncryptCBC);
		_EncryptIV(AESDecryptCBC);

		_EncryptFileIV(AESEncryptCFB);
		_EncryptFileIV(AESDecryptCFB);
		_EncryptFileIV(AESEncryptOFB);
		_EncryptFileIV(AESDecryptOFB);
		_EncryptFileIV(AESEncryptCTR);
		_EncryptFileIV(AESDecryptCTR);
		_EncryptFile(AESEncryptECB);
		_EncryptFile(AESDecryptECB);
		_EncryptFileIV(AESEncryptCBC);
		_EncryptFileIV(AESDecryptCBC);
#undef _EncryptIV
#undef _Encrypt
#undef _EncryptFileIV
#undef _EncryptFile

#define _Encode(name) void name(std::string& out, CStr& str)
#define _EncodeFile(name) void name##_File(std::string& out, CStr& str)
		//hex
		_Encode(HexEncode);
		_Encode(HexDecode);
		_EncodeFile(HexEncode_File);
		_EncodeFile(HexDecode);
		//base64
		_Encode(Base64Encode);
		_Encode(Base64Decode);
		_EncodeFile(Base64Encode);
		_EncodeFile(Base64Decode);
#undef _Encode
#undef _EncodeFile
	}
}
#endif



#ifdef _YXL_GLFW_
namespace YXL
{
	class LIB_YXL_HELPER GLFWBase
	{
	public:
		virtual ~GLFWBase()
		{
			glfwDestroyWindow(_wnd);
		}
		virtual bool Init(const int wnd_w=1280, const int wnd_h=720, const bool hidden=false);
		virtual void Run();
		virtual void CleanUp() {}
		virtual void BeforeFrame(const int frame_id) {}
		virtual void AfterFrame(const int frame_id) {}
		virtual void Frame(const int frame_id) = 0;
		virtual void KeyCallback(int key, int scancode, int action, int mods) 
		{
			if (action == GLFW_PRESS)
			{
				switch (key)
				{
				case GLFW_KEY_ESCAPE:
					glfwSetWindowShouldClose(_wnd, GLFW_TRUE);
					break;
				default:
					break;
				}
			}
		}
		virtual void MouseButtonCallback(int button, int action, int mods)
		{
			//if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
			//	//do something
		}
		virtual void ScrollCallback(double xoffset, double yoffset)
		{

		}
		virtual void SetWindowTitle(const std::string title)
		{
			glfwSetWindowTitle(_wnd, title.c_str());
		}

	protected:
		GLFWwindow* _wnd = nullptr;

	private:
		int _frame_id = 0;
	};
}
#endif


#endif