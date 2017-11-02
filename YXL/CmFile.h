/******************************************************
									CmFile.h


******************************************************/
#ifndef _CM_FILE_H_2014_4_19_
#define _CM_FILE_H_2014_4_19_

#ifndef LIB_BS_OPTIMISE
#ifdef _LIB_BS_OPTIMISE_IMPL
#define LIB_BS_OPTIMISE __declspec(dllexport)
#else
#define LIB_BS_OPTIMISE __declspec(dllimport)
#endif
#endif

#define POINTER_64 __ptr64

#pragma warning(disable:4819)

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
//#define _CRT_SECURE_NO_WARNINGS
#endif


#include <string>
#include <vector>
#include <Windows.h>
#include <omp.h>
#include <fstream>
#include <opencv2\opencv.hpp>

//this file is adopted from the source code provided by the author of paper "BING: Binarized Normed Gradients for Objectness Estimation at 300fps"

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

#pragma comment( lib, CV_LIB("core"))
#pragma comment( lib, CV_LIB("imgproc"))
#pragma comment( lib, CV_LIB("highgui"))

#if (2 < CV_MAJOR_VERSION)
#pragma comment( lib, CV_LIB("imgcodecs"))
#endif

typedef const std::string CStr;
typedef std::vector<std::string> vecS;
typedef std::vector<int> vecI;
typedef const cv::Mat CMat;
typedef std::vector<std::string> vecS;
typedef std::vector<cv::Mat> vecM;
typedef std::vector<float> vecF;
typedef std::vector<double> vecD;
#define _S(str) ((str).c_str())

class LIB_BS_OPTIMISE CmFile
{
public:
	static std::string BrowseFile(const char* strFilter = "Images (*.jpg;*.png)\0*.jpg;*.png\0All (*.*)\0*.*\0\0", bool isOpen = true, const std::string& dir = "", CStr& title = "BrowseFile");
	static std::string BrowseFolder(CStr& title="BrowseFolder");

	static inline std::string GetFolder(CStr& path);
	static inline std::string GetName(CStr& path);
	static inline std::string GetNameNE(CStr& path);
	static inline std::string GetPathNE(CStr& path);

	// Get file names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
	static int GetNames(CStr &nameW, vecS &names, std::string &dir = std::string());
	static int GetNames(CStr& rootFolder, CStr &fileW, vecS &names);
	static int GetNamesNE(CStr& nameWC, vecS &names, std::string &dir = std::string(), std::string &ext = std::string());
	static int GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names);
	static inline std::string GetExtention(CStr name);

	static inline bool FileExist(CStr& filePath);
	static inline bool FilesExist(CStr& fileW);
	static inline bool FolderExist(CStr& strPath);

	static inline std::string GetWkDir();

	static BOOL MkDir(CStr&  path);

	// Eg: RenameImages("D:/DogImages/*.jpg", "F:/Images", "dog", ".jpg");
	static int Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt);

	static inline void RmFile(CStr& fileW);
	static void RmFolder(CStr& dir);
	static void CleanFolder(CStr& dir, bool subFolder = false);

	static int GetSubFolders(CStr& folder, vecS& subFolders);

	inline static BOOL Copy(CStr &src, CStr &dst, BOOL failIfExist = FALSE);
	inline static BOOL Move(CStr &src, CStr &dst, DWORD dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	static BOOL Move2Dir(CStr &srcW, CStr dstDir);
	static BOOL Copy2Dir(CStr &srcW, CStr dstDir);

	//Load mask image and threshold thus noisy by compression can be removed
	static cv::Mat LoadMask(CStr& fileName);

	static void WriteNullFile(CStr& fileName) { FILE *f; fopen_s(&f, _S(fileName), "w"); fclose(f); }

	static void ChkImgs(CStr &imgW);

	static void RunProgram(CStr &fileName, CStr &parameters = "", bool waiteF = false, bool showW = true);

	static void SegOmpThrdNum(double ratio = 0.8);

	// Copy files and add suffix. e.g. copyAddSuffix("./*.jpg", "./Imgs/", "_Img.jpg")
	static void copyAddSuffix(CStr &srcW, CStr &dstDir, CStr &dstSuffix);

	static vecS loadStrList(CStr &fName);
	static bool writeStrList(CStr &fName, const vecS &strs);
};


/************************************************************************/
/* Implementation of inline functions                                   */
/************************************************************************/
std::string CmFile::GetFolder(CStr& path)
{
	return path.substr(0, path.find_last_of("\\/") + 1);
}

std::string CmFile::GetName(CStr& path)
{
	size_t start = path.find_last_of("\\/") + 1;
	size_t end = path.find_last_not_of(' ') + 1;
	return path.substr(start, end - start);
}

std::string CmFile::GetNameNE(CStr& path)
{
	size_t start = path.find_last_of("\\/") + 1;
	size_t end = path.find_last_of('.');
	if (end >= 0)
		return path.substr(start, end - start);
	else
		return path.substr(start, path.find_last_not_of(' ') + 1 - start);
}

std::string CmFile::GetPathNE(CStr& path)
{
	size_t end = path.find_last_of('.');
	if (end >= 0)
		return path.substr(0, end);
	else
		return path.substr(0, path.find_last_not_of(' ') + 1);
}

std::string CmFile::GetExtention(CStr name)
{
	return name.substr(name.find_last_of('.'));
}

BOOL CmFile::Copy(CStr &src, CStr &dst, BOOL failIfExist)
{
	return ::CopyFileA(src.c_str(), dst.c_str(), failIfExist);
}

BOOL CmFile::Move(CStr &src, CStr &dst, DWORD dwFlags)
{
	return MoveFileExA(src.c_str(), dst.c_str(), dwFlags);
}

void CmFile::RmFile(CStr& fileW)
{
	vecS names;
	std::string dir;
	int fNum = CmFile::GetNames(fileW, names, dir);
	for (int i = 0; i < fNum; i++)
		::DeleteFileA(_S(dir + names[i]));
}

// Test whether a file exist
bool CmFile::FileExist(CStr& filePath)
{
	if (filePath.size() == 0)
		return false;

	return  GetFileAttributesA(_S(filePath)) != INVALID_FILE_ATTRIBUTES; // ||  GetLastError() != ERROR_FILE_NOT_FOUND;
}

bool CmFile::FilesExist(CStr& fileW)
{
	vecS names;
	int fNum = GetNames(fileW, names);
	return fNum > 0;
}

std::string CmFile::GetWkDir()
{
	std::string wd;
	wd.resize(1024);
	DWORD len = GetCurrentDirectoryA(1024, &wd[0]);
	wd.resize(len);
	return wd;
}

bool CmFile::FolderExist(CStr& strPath)
{
	int i = (int)strPath.size() - 1;
	for (; i >= 0 && (strPath[i] == '\\' || strPath[i] == '/'); i--)
		;
	std::string str = strPath.substr(0, i + 1);

	WIN32_FIND_DATAA  wfd;
	HANDLE hFind = FindFirstFileA(_S(str), &wfd);
	bool rValue = (hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	FindClose(hFind);
	return rValue;
}

/************************************************************************/
/*                   Implementations                                    */
/************************************************************************/

#endif