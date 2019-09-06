#define _LIB_YXL_HELPER_IMPL
#include "YXLHelper.h"
#include <memory>

#ifdef _WITH_WINDOWS_
#include <shlobj.h>
#include <Commdlg.h>
#include <ShellAPI.h>
#endif

namespace YXL
{
#ifdef _YXL_OUT_STREAM_
	YXLOut yxlout;
#endif
#ifdef _YXL_TIME_
	Timer g_timer;
#endif
}

#ifdef _YXL_FILES_
namespace YXL
{
	vecS File::loadStrList(CStr &fName)
	{
		std::ifstream fIn(fName);
		std::string line;
		vecS strs;
		while (getline(fIn, line) && line.empty() == false)
			strs.push_back(line);
		return strs;
	}
	bool File::writeStrList(CStr &fName, const vecS &strs)
	{
		FILE *f = fopen(fName.c_str(), "w");
		if (f == NULL)
			return false;
		for (size_t i = 0; i < strs.size(); i++)
			fprintf(f, "%s\n", strs[i].c_str());
		fclose(f);
		return true;
	}

#ifdef _WITH_WINDOWS_
	std::pair<DWORD, DWORD> File::GetFileInfo(const std::string& file_path, FileInfo fi)
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

	const char * YXL::File::_BrowseFile(DWORD flags, const std::vector<std::pair<std::string, std::string>>& in_filters, bool isOpen, const std::string & def_dir, CStr & title)
	{
		std::string filters;
		{
			std::string empty(1, '\0');
			for (auto& pair : in_filters)
				filters += pair.first + " (" + pair.second + ")" + empty + pair.second + empty;
			if (in_filters.empty())
				filters = "All (*.*)" + empty + "*.*" + empty;
			filters += empty;
		}

		static char Buffer[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, Buffer);
		std::string dir = Buffer;

		std::string def_dir2 = def_dir;
		for (auto iter = def_dir2.begin(); iter != def_dir2.end(); ++iter)
		{
			if ('/' == *iter)
			{
				*iter = '\\';
			}
		}

		std::string opened_dir = dir + "\\" + def_dir2;
		if (false == FolderExist(opened_dir))
		{
			if (FolderExist(def_dir2))
			{
				opened_dir = def_dir2;
			}
			else
			{
				opened_dir = dir;
			}
		}

		memset(Buffer, 0, sizeof(Buffer));
		//strncpy(Buffer, opened_dir.c_str(), opened_dir.size());

		OPENFILENAMEA   ofn;
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = Buffer;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = filters.c_str();
		ofn.nFilterIndex = 1;
		ofn.Flags = flags;

		ofn.lpstrInitialDir = &opened_dir[0];
		ofn.lpstrTitle = title.c_str();

		if (isOpen) {
			ofn.Flags |= OFN_FILEMUSTEXIST;
			GetOpenFileNameA(&ofn);
			SetCurrentDirectoryA(dir.c_str());
			return Buffer;
		}

		GetSaveFileNameA(&ofn);

		SetCurrentDirectoryA(dir.c_str());

		return Buffer;
	}

	std::string File::BrowseFile(const  std::vector<std::pair<std::string, std::string>>& in_filters, bool isOpen, const std::string & def_dir, CStr & title)
	{
		return _BrowseFile(OFN_PATHMUSTEXIST | OFN_EXPLORER, in_filters, isOpen, def_dir, title);
	}

	vecS YXL::File::BrowseFileMultiSelect(const std::vector<std::pair<std::string, std::string>>& filters, bool isOpen, const std::string & def_dir, CStr & title)
	{
		auto buf = _BrowseFile(OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT, filters, isOpen, def_dir, title);
		vecS ret;

		int len = strlen(buf);

		std::string dir = buf + std::string("\\");
		buf += len + 1;

		while (true)
		{
			len = strlen(buf);
			if (0 == len)
				break;
			ret.push_back(dir + buf);
			buf += len + 1;
		}

		return ret;
	}

	std::string File::BrowseFolder(CStr & title)
	{
		static char Buffer[MAX_PATH];
		BROWSEINFOA bi;//Initial bi 	
		bi.hwndOwner = NULL;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = Buffer; // Dialog can't be shown if it's NULL
		bi.lpszTitle = title.c_str();
		bi.ulFlags = 0;
		bi.lpfn = NULL;
		bi.iImage = NULL;


		LPITEMIDLIST pIDList = SHBrowseForFolderA(&bi); // Show dialog
		if (pIDList) {
			SHGetPathFromIDListA(pIDList, Buffer);
			if (Buffer[strlen(Buffer) - 1] == '\\')
				Buffer[strlen(Buffer) - 1] = 0;

			return std::string(Buffer)+"\\";
		}
		return std::string();
	}

	bool File::MkDir(CStr&  path)
	{
		if (path.size() == 0)
			return false;

		static char buffer[1024];
		strcpy_s(buffer, sizeof(buffer), path.c_str());
		for (int i = 0; buffer[i] != 0; i++) {
			if (buffer[i] == '\\' || buffer[i] == '/') {
				buffer[i] = '\0';
				CreateDirectoryA(buffer, 0);
				buffer[i] = '/';
			}
		}
		return CreateDirectoryA(path.c_str(), 0)==TRUE;
	}

	int File::Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(srcNames, names, inDir);
		char buff[500];
		for (int i = 0; i < fNum; i++) {
			sprintf(buff, "%s\\%.4d%s.%s", dstDir.c_str(), i, nameCommon, nameExt);
			std::string dstName = buff;
			std::string srcName = inDir + names[i];
			::CopyFileA(srcName.c_str(), dstName.c_str(), FALSE);
		}
		return fNum;
	}

	void File::RmFile(CStr& fileW)
	{
		vecS names;
		std::string dir;
		int fNum = GetNames(fileW, names, dir);
		for (int i = 0; i < fNum; i++)
			::DeleteFileA((dir + names[i]).c_str());
	}

	void File::CleanFolder(CStr& dir, bool subFolder)
	{
		vecS names;
		int fNum = GetNames(dir + "/*.*", names);
		for (int i = 0; i < fNum; i++)
			RmFile(dir + "/" + names[i]);

		vecS subFolders;
		int subNum = GetSubFolders(dir, subFolders);
		if (subFolder)
			for (int i = 0; i < subNum; i++)
				CleanFolder(dir + "/" + subFolders[i], true);
	}

	void File::RmFolder(CStr& dir)
	{
		CleanFolder(dir);
		if (FolderExist(dir))
		{
			char buff[500];
			sprintf(buff, "/c rmdir /s /q \"%s\"", dir.c_str());
			RunProgram("Cmd.exe", buff, true, false);
		}
	}

	int File::GetSubFolders(CStr & folder, vecS & subFolders)
	{
		subFolders.clear();
		WIN32_FIND_DATAA fileFindData;
		std::string nameWC = folder + "\\*";
		HANDLE hFind = ::FindFirstFileA(nameWC.c_str(), &fileFindData);
		if (hFind == INVALID_HANDLE_VALUE)
			return 0;

		do {
			if (fileFindData.cFileName[0] == '.')
				continue; // filter the '..' and '.' in the path
			if (fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				subFolders.push_back(fileFindData.cFileName);
		} while (::FindNextFileA(hFind, &fileFindData));
		FindClose(hFind);
		return (int)subFolders.size();
	}

	int File::GetNames(CStr & nameW, vecS & names, std::string & dir)
	{
		dir = GetFolder(nameW);
		names.clear();
		names.reserve(6000);
		WIN32_FIND_DATAA fileFindData;
		HANDLE hFind = ::FindFirstFileA(nameW.c_str(), &fileFindData);
		if (hFind == INVALID_HANDLE_VALUE)
			return 0;

		do {
			if (fileFindData.cFileName[0] == '.')
				continue; // filter the '..' and '.' in the path
			if (fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue; // Ignore sub-folders
			names.push_back(fileFindData.cFileName);
		} while (::FindNextFileA(hFind, &fileFindData));
		FindClose(hFind);
		return (int)names.size();
	}

	int File::GetNames(CStr & rootFolder, CStr & fileW, vecS & names)
	{
		GetNames(rootFolder + fileW, names);
		vecS subFolders, tmpNames;
		int subNum = GetSubFolders(rootFolder, subFolders);
		for (int i = 0; i < subNum; i++) {
			subFolders[i] += "/";
			int subNum = GetNames(rootFolder + subFolders[i], fileW, tmpNames);
			for (int j = 0; j < subNum; j++)
				names.push_back(subFolders[i] + tmpNames[j]);
		}
		return (int)names.size();
	}

	int File::GetNamesNE(CStr & nameWC, vecS & names, std::string & dir, std::string & ext)
	{
		int fNum = GetNames(nameWC, names, dir);
		ext = GetExtention(nameWC);
		for (int i = 0; i < fNum; i++)
			names[i] = GetNameNE(names[i]);
		return fNum;
	}

	int File::GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names)
	{
		int fNum = GetNames(rootFolder, fileW, names);
		size_t extS = GetExtention(fileW).size();
		for (int i = 0; i < fNum; i++)
			names[i].resize(names[i].size() - extS);
		return fNum;
	}

	bool File::Move2Dir(CStr & srcW, CStr dstDir)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(srcW, names, inDir);
		bool r = true;
		for (int i = 0; i < fNum; i++)
			if (Move(inDir + names[i], dstDir + names[i]) == false)
				r = false;
		return r;
	}

	bool File::Copy2Dir(CStr & srcW, CStr dstDir)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(srcW, names, inDir);
		bool r = true;
		for (int i = 0; i < fNum; i++)
			if (Copy(inDir + names[i], dstDir + names[i]) == false)
				r = false;
		return r;
	}

	void File::RunProgram(CStr & fileName, CStr & parameters, bool waiteF, bool showW)
	{
		std::string runExeFile = fileName;
#ifdef _DEBUG
		runExeFile.insert(0, "..\\Debug\\");
#else
		runExeFile.insert(0, "..\\Release\\");
#endif // _DEBUG
		if (!FileExist(runExeFile.c_str()))
			runExeFile = fileName;

		std::string wkDir = GetWkDir();

		SHELLEXECUTEINFOA  ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = runExeFile.c_str();
		ShExecInfo.lpParameters = parameters.c_str();
		ShExecInfo.lpDirectory = wkDir.c_str();
		ShExecInfo.nShow = showW ? SW_SHOW : SW_HIDE;
		ShExecInfo.hInstApp = NULL;
		ShellExecuteExA(&ShExecInfo);

		//CmLog::LogLine("Run: %s %s\n", ShExecInfo.lpFile, ShExecInfo.lpParameters);

		if (waiteF)
			WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	}

#else
#ifdef _WITH_QT_
	std::shared_ptr<QApplication> File::_app = nullptr;

	std::string File::BrowseFolder(CStr& title)
	{
		auto dir = QFileDialog::getExistingDirectory(nullptr, QString::fromLocal8Bit(title.c_str()));
		return (std::string)dir.toLocal8Bit()+"\\";
	}

	std::string File::BrowseFile(const std::vector<std::pair<std::string, std::string>>& in_filters, bool isOpen, const std::string& def_dir, CStr& title)
	{
		std::string filters;
		{
			for (auto& pair : in_filters)
			{
				if (filters.empty() == false)
					filters += ";;";
				filters += pair.first + " (" + pair.second + ")";
			}
			if (in_filters.empty())
				filters += "All (*.*)";
		}

		auto wkdir = GetWkDir();
		std::string opened_dir = wkdir + "\\" + def_dir;
		if (false == FolderExist(opened_dir))
		{
			if (FolderExist(def_dir))
			{
				opened_dir = def_dir;
			}
			else
			{
				opened_dir = wkdir;
			}
		}
		std::string opened_dir = "F;\\";

		QString qPath;
		if (isOpen)
			qPath = QFileDialog::getOpenFileName(nullptr, QString::fromLocal8Bit(title.c_str()), QString::fromLocal8Bit(opened_dir.c_str()), QString::fromLocal8Bit(filters.c_str()));
		else
			qPath = QFileDialog::getSaveFileName(nullptr, QString::fromLocal8Bit(title.c_str()), QString::fromLocal8Bit(opened_dir.c_str()), QString::fromLocal8Bit(filters.c_str()));

		return (std::string)qPath.toLocal8Bit();
	}

	int File::GetNames(CStr &nameW, vecS &names, std::string &dir)
	{
		dir = GetFolder(nameW);
		names.clear();
		names.reserve(6000);

		QDir fromDir(QString::fromLocal8Bit(dir.c_str()));
		QStringList filters;
		filters.append(QString::fromLocal8Bit(nameW.substr(dir.length(), nameW.length() - dir.length()).c_str()));
		QFileInfoList fileInfoList = fromDir.entryInfoList(filters, QDir::Dirs | QDir::Files);
		foreach(QFileInfo fileInfo, fileInfoList)
		{
			if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
				continue;
			if (fileInfo.isFile())
			{
				names.push_back((std::string)fileInfo.fileName().toLocal8Bit());
			}
		}
		return (int)names.size();
	}

	int File::GetNames(CStr& rootFolder, CStr &fileW, vecS &names)
	{
		std::string dir;
		GetNames(rootFolder + fileW, names, dir);
		vecS subFolders, tmpNames;
		int subNum = GetSubFolders(rootFolder, subFolders);
		for (int i = 0; i < subNum; i++) {
			subFolders[i] += "/";
			int subNum = GetNames(rootFolder + subFolders[i], fileW, tmpNames);
			for (int j = 0; j < subNum; j++)
				names.push_back(subFolders[i] + tmpNames[j]);
		}
		return (int)names.size();
	}

	int File::GetNamesNE(CStr& nameWC, vecS &names, std::string &dir, std::string &ext)
	{
		int fNum = GetNames(nameWC, names, dir);
		ext = GetExtention(nameWC);
		for (int i = 0; i < fNum; i++)
			names[i] = GetNameNE(names[i]);
		return fNum;
	}

	int File::GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names)
	{
		int fNum = GetNames(rootFolder, fileW, names);
		size_t extS = GetExtention(fileW).size();
		for (int i = 0; i < fNum; i++)
			names[i].resize(names[i].size() - extS);
		return fNum;
	}

	bool File::MkDir(CStr&  _path)
	{
		if (_path.size() == 0)
			return false;

		QDir dir;
		static char buffer[1024];
		strcpy(buffer, _path.c_str());
		for (int i = 0; buffer[i] != 0; i++) {
			if (buffer[i] == '\\' || buffer[i] == '/') {
				buffer[i] = '\0';
				dir.mkdir(QString::fromLocal8Bit(buffer));
				buffer[i] = '/';
			}
		}
		return dir.mkdir(QString::fromLocal8Bit(_path.c_str()));
	}

	int File::Rename(CStr& _srcNames, CStr& _dstDir, const char *nameCommon, const char *nameExt)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(_srcNames, names, inDir);
		for (int i = 0; i < fNum; i++) {
			std::string dstName = cv::format("%s\\%.4d%s.%s", _dstDir.c_str(), i, nameCommon, nameExt);
			std::string srcName = inDir + names[i];
			QFile file;
			file.rename(QString::fromLocal8Bit(srcName.c_str()), QString::fromLocal8Bit(dstName.c_str()));
		}
		return fNum;
	}

	void File::RmFile(CStr& fileW)
	{
		vecS names;
		std::string dir;
		int fNum = GetNames(fileW, names, dir);
		for (int i = 0; i < fNum; i++)
		{
			QFile f(QString::fromStdString(dir + names[i]));
			f.remove();
		}
	}

	void File::CleanFolder(CStr& dir, bool subFolder)
	{
		vecS names;
		std::string tmp;
		int fNum = GetNames(dir + "/*.*", names, tmp);
		for (int i = 0; i < fNum; i++)
			RmFile(dir + "/" + names[i]);

		vecS subFolders;
		int subNum = GetSubFolders(dir, subFolders);
		if (subFolder)
			for (int i = 0; i < subNum; i++)
				CleanFolder(dir + "/" + subFolders[i], true);
	}

	int File::GetSubFolders(CStr& folder, vecS& subFolders)
	{
		subFolders.clear();

		QDir dir(QString::fromLocal8Bit(folder.c_str()));
		foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Dirs | QDir::Files))
		{
			QString strName = fileInfo.fileName();
			if ((strName == QString(".")) || (strName == QString("..")))
				continue;
			if (fileInfo.isDir())
				subFolders.push_back((std::string)strName.toLocal8Bit());
		}
		return (int)subFolders.size();
	}

	bool File::Move2Dir(CStr &srcW, CStr dstDir)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(srcW, names, inDir);
		bool r = true;
		for (int i = 0; i < fNum; i++)
			if (Move(inDir + names[i], dstDir + names[i]) == false)
				r = false;
		return r;
	}

	bool File::Copy2Dir(CStr &srcW, CStr dstDir)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(srcW, names, inDir);
		bool r = true;
		for (int i = 0; i < fNum; i++)
			if (Copy(inDir + names[i], dstDir + names[i]) == false)
				r = false;
		return r;
	}

	void File::RunProgram(CStr &fileName, CStr &parameters, bool waiteF, bool showW)
	{
		std::string runExeFile = fileName;
#ifdef _DEBUG
		runExeFile.insert(0, "..\\Debug\\");
#else
		runExeFile.insert(0, "..\\Release\\");
#endif // _DEBUG
		if (!FileExist(runExeFile.c_str()))
			runExeFile = fileName;

		std::string wkDir = GetWkDir();

		QProcess proc;
		QStringList param(QString::fromLocal8Bit(parameters.c_str()));

		proc.start(QString::fromLocal8Bit(fileName.c_str()), param);

		if (waiteF)
			proc.waitForFinished();
	}

#endif
#endif
}
#endif //_YXL_FILES_

#ifdef _YXL_UNION_FIND_
namespace YXL
{
	void UnionFind::Union(int a, int b)
	{
		a = Find(a);
		b = Find(b);
		if (a == b)
			return;
		if (_group_size[a] < _group_size[b])
			std::swap(a, b);
		_id[b] = a;
		_group_size[a] += _group_size[b];
		_group_size.erase(_group_size.find(b));
		--_group_cnt;
	}

	void UnionFind::Update()
	{
		_group_id.resize(_id.size());
		std::map<int, int> id_map;
		for (int i(0); i != _id.size(); ++i)
		{
			int ii = Find(i);
			if (id_map.find(ii) == id_map.end())
			{
				id_map[ii] = static_cast<int>(id_map.size());
				_groups[id_map[ii]].reserve(_group_size[ii]);
			}
			_group_id[i] = id_map[ii];
			_groups[id_map[ii]].push_back(i);
		}

	} 
}
#endif //_YXL_UNION_FIND_

#ifdef _YXL_IMG_PROC_
namespace YXL
{
	template<typename basicType, int ch> cv::Mat _FilterImage(cv::Mat img, cv::Mat kernel, cv::Mat mask)
	{
		bool no_mask = mask.empty() != false;
		if (kernel.type() != CV_32FC1 && kernel.rows != 1 && kernel.cols % 2 != 1 && (no_mask || mask.type() != CV_8UC1 || mask.size()!=img.size()))
		{
			yxlout << YXL_LOG_PREFIX << "params error..." << std::endl;
			return cv::Mat();
		}

		const int half_ksize = kernel.cols / 2;

		cv::Mat tmp(img.size(), img.type());
		for (int i(0); i != img.rows; ++i)
			for (int j(0); j != img.cols; ++j)
			{
				cv::Vec4d acc=cv::Vec4d::all(0);
				double cnt = 0;
				for (int k = (std::max)(0, j - half_ksize); k <= (std::min)(img.cols - 1, j + half_ksize); ++k)
					if (no_mask || mask.at<uchar>(i, k) > 0)
					{
						int m = k - (j - half_ksize);
						auto pix = img.at<cv::Vec<basicType, ch> >(i, k);
						for (int n(0); n != ch; ++n)
							acc[n] += kernel.at<float>(m)*pix[n];
						cnt += kernel.at<float>(m);
					}
				cv::Vec<basicType, ch>  pix= cv::Vec<basicType, ch>::all(0);
				if (no_mask || mask.at<uchar>(i, j) > 0)
					for (int n(0); n != ch; ++n)
						pix[n] = static_cast<basicType>(acc[n] / cnt);
				tmp.at<cv::Vec<basicType, ch> >(i, j) = pix;
			}
		cv::Mat res(img.size(), img.type());
		for (int i(0); i != img.rows; ++i)
			for (int j(0); j != img.cols; ++j)
			{
				cv::Vec4d acc = cv::Vec4d::all(0);
				double cnt = 0;
				for (int k = (std::max)(0, i - half_ksize); k <= (std::min)(img.rows - 1, i + half_ksize); ++k)
					if (no_mask || mask.at<uchar>(k, j) > 0)
					{
						int m = k - (i - half_ksize);
						auto pix = tmp.at<cv::Vec<basicType, ch> >(k, j);
						for (int n(0); n != ch; ++n)
							acc[n] += kernel.at<float>(m)*pix[n];
						cnt += kernel.at<float>(m);
					}
				cv::Vec<basicType, ch>  pix = cv::Vec<basicType, ch>::all(0);
				if (no_mask || mask.at<uchar>(i, j) > 0)
					for (int n(0); n != ch; ++n)
						pix[n] = static_cast<basicType>(acc[n] / cnt);
				res.at<cv::Vec<basicType, ch> >(i, j) = pix;
			}

		return res;
	}

	cv::Mat FilterImage(cv::Mat img, cv::Mat kernel, cv::Mat mask)
	{
		if (img.type() == CV_8UC1)
			return _FilterImage<uchar, 1>(img, kernel, mask);
		else if (img.type() == CV_8UC3)
			return _FilterImage<uchar, 3>(img, kernel, mask);
		else if (img.type() == CV_32FC1)
			return _FilterImage<float, 1>(img, kernel, mask);
		else if (img.type() == CV_32FC3)
			return _FilterImage<float, 3>(img, kernel, mask);
		else
			return cv::Mat();
	}
}
#endif //_YXL_IMG_PROC_

#ifdef _YXL_IMG_CODEC_
#define IMPL_ALL(marco) \
		marco(RGBA, 4, true);\
		marco(BGRA, 4, false);\
		marco(RGB, 3, true);\
		marco(BGR, 3, false);\
		marco(Grey, 1, false);

#ifdef _YXL_IMG_CODEC_STB_IMAGE_
#include "image/stb_image/stb_image.h"
#include "image/stb_image/stb_image_write.h"
namespace YXL
{
	namespace Image
	{
		void SwapRBChannel(std::shared_ptr<unsigned char>& data, const int w, const int h, const int ch)
		{
			if (ch < 3)
				return;
			const int len = w*h*ch;
			auto ptr = data.get();
			for (int i(0); i < len; i += ch)
				std::swap(ptr[i], ptr[i + 2]);
		}
		void DecodePNG(std::shared_ptr<unsigned char>& data, int& w, int& h, const int req_ch, CStr& in_data, const bool is_rgb)
		{
			int ch=0;
			data = nullptr;
			w = 0;
			h = 0;
			auto _data = stbi_load_from_memory(in_data.c_str(), in_data.length(), &w, &h, &ch, req_ch);
			if (_data == nullptr)
				return;
			if (req_ch == ch || (req_ch == 4 && ch == 3))
				data = std::shared_ptr<unsigned char>(_data);
			else
				std::cout <<__FILE__<<"["<<__FUNCTION__<< "]: something wrong..." << std::endl;

			if (req_ch >= 3 && is_rgb==false)
				SwapRBChannel(data, w, h, req_ch);
		}
		void EncodePNG(std::shared_ptr<unsigned char>& out_data, int& out_data_size, const unsigned char* img, const int w, const int h, const int ch, const bool is_rgb)
		{
			if (ch >= 3 && is_rgb==false)
			{
				std::shared_ptr<unsigned char> tmp(new unsigned char[w*h * ch]);
				memcpy(tmp.get(), img, w*h * ch);
				SwapRBChannel(tmp, w, h, ch);
				unsigned char *data = stbi_write_png_to_mem(tmp.get(), w*ch, w, h, ch, &out_data_size);
				out_data = std::shared_ptr<unsigned char>(data);
			}
			else
			{
				unsigned char *data = stbi_write_png_to_mem(img, w*ch, w, h, ch, &out_data_size);
				out_data = std::shared_ptr<unsigned char>(data);
			}
		}

#define IMPL_DECODE_FUNC(name, ch, is_rgb) void DecodePNG_##name(std::shared_ptr<unsigned char>& data, int& w, int& h, CStr& in_data)\
		{\
			DecodePNG(data, w, h, ch, in_data, is_rgb);\
		}
		IMPL_ALL(IMPL_DECODE_FUNC);
#undef IMPL_DECODE_FUNC

#define IMPL_ENCODE_FUNC(name, ch, is_rgb) \
		void EncodePNG_##name(std::shared_ptr<unsigned char>& out_data, int& out_data_size, const unsigned char* img, const int w, const int h)\
		{\
			EncodePNG(out_data, out_data_size, img, w, h, ch, is_rgb);\
		}
		IMPL_ALL(IMPL_ENCODE_FUNC);
#undef IMPL_ENCODE_FUNC
	}
}
#endif //_YXL_IMG_CODEC_STB_IMAGE_

#ifdef _YXL_IMG_CODEC_WEBP_
#include "image/webp/webp/encode.h"
#include "image/webp/webp/decode.h"
namespace YXL
{
	namespace Image
	{
		void DecodeWebP(std::shared_ptr<unsigned char>& out_data, int& w, int& h, const int req_ch, CStr& in_data, const bool is_rgb)
		{
			int _ch = req_ch == 1 ? 3 : req_ch;
			bool _is_rgb = req_ch == 1 ? true : is_rgb;

			typedef uint8_t* (*decode_func)(const uint8_t* data, size_t data_size, int* width, int* height);
			decode_func func[] = { WebPDecodeBGR, WebPDecodeBGRA, WebPDecodeRGB, WebPDecodeRGBA };

			const int func_idx = (_is_rgb ? 2 : 0) + (_ch == 3 ? 0 : 1);

			auto ret = func[func_idx](reinterpret_cast<const uint8_t*>(in_data.c_str()), in_data.length(), &w, &h);

			if (req_ch >= 3)
				out_data = std::shared_ptr<unsigned char>(ret);
			else
			{
				out_data = std::shared_ptr<unsigned char>(new unsigned char[w*h]);
				auto dst = out_data.get();
				int len = w*h;
				for (int i(0), ii(0); i != len; ++i, ii += _ch)
					dst[i] = ret[ii] * 0.299f + ret[ii + 1] * 0.587f + ret[ii + 2] * 0.114f;
			}
		}
		void EncodeWebP(std::shared_ptr<unsigned char>& out_data, int& out_data_size, 
			const unsigned char* img, const int w, const int h, const int ch, 
			const bool is_rgb, const float quality)
		{
			out_data = nullptr;
			int _ch = ch;
			const unsigned char* _img = img;

			std::shared_ptr<unsigned char> tmp = nullptr;
			if (ch == 1)
			{
				_ch = 4;
				tmp = std::shared_ptr<unsigned char>(new unsigned char[w*h*_ch]);
				auto dst = (int*)tmp.get();
				auto src = img;
				const int len = w*h;
				for(int i(0); i < len; ++i)
					dst[i]= ((int)(uint32_t)src[i] * 0x010101 | 0xff000000);
				_img = tmp.get();
			}
			
			typedef size_t(*encode_func)(const uint8_t* rgb, int width, int height, int stride, float quality_factor, uint8_t** output);
			typedef size_t (*encode_func2)(const uint8_t* rgb, int width, int height, int stride, uint8_t** output);
			const encode_func func[] = { WebPEncodeBGR, WebPEncodeBGRA, WebPEncodeRGB, WebPEncodeRGBA };
			const encode_func2 func2[] = { WebPEncodeLosslessBGR, WebPEncodeLosslessBGRA, WebPEncodeLosslessRGB, WebPEncodeLosslessRGBA };

			const int func_idx = (is_rgb ? 2 : 0) + (_ch == 3 ? 0 : 1);

			uint8_t* ptr = nullptr;
			if (quality < 100.f)
				out_data_size = func[func_idx](_img, w, h, w*_ch, quality, &ptr);
			else
				out_data_size = func2[func_idx](_img, w, h, w*_ch, &ptr);

			if (!out_data_size)
			{
				std::cout << __FILE__ << "[" << __FUNCTION__ << "]: something wrong..." << std::endl;
				return;
			}
			out_data = std::shared_ptr<unsigned char>(ptr);
		}
#define IMPL_DECODE_FUNC_WEBP(name, ch, is_rgb) \
		void DecodeWebP_##name(std::shared_ptr<unsigned char>& img, int & w, int & h, CStr & in_data)\
		{\
			DecodeWebP(img, w, h, ch, in_data, is_rgb);\
		}
		IMPL_ALL(IMPL_DECODE_FUNC_WEBP);
#undef IMPL_DECODE_FUNC_WEBP
#define IMPL_ENCODE_FUNC_WEBP(name, ch, is_rgb) \
		void EncodeWebP_##name(std::shared_ptr<unsigned char>& out_data, int& out_data_size, const unsigned char* img, const int w, const int h, const float quality)\
		{\
			EncodeWebP(out_data, out_data_size, img, w, h, ch, is_rgb, quality);\
		}
		IMPL_ALL(IMPL_ENCODE_FUNC_WEBP);
#undef IMPL_ENCODE_FUNC_WEBP
	}
}
#endif //_YXL_IMG_CODEC_WEBP_

#undef IMPL_ALL
#endif

#ifdef _YXL_COMPRESS_
#ifdef _YXL_COMPRESS_MINI_Z_
#include "miniz/miniz.h"
#include "miniz/miniz_zip.h"
namespace YXL
{
	namespace ZIP
	{
		static std::map<ZIP_COMPRESSION, mz_uint> _compression=
		{
			{ ZIP_COMPRESSION::NO_COMPRESSION, MZ_NO_COMPRESSION},
			{ ZIP_COMPRESSION::LEVEL_0, 0 },
			{ ZIP_COMPRESSION::LEVEL_1, 1 },
			{ ZIP_COMPRESSION::LEVEL_2, 2 },
			{ ZIP_COMPRESSION::LEVEL_3, 3 },
			{ ZIP_COMPRESSION::LEVEL_4, 4 },
			{ ZIP_COMPRESSION::LEVEL_5, 5 },
			{ ZIP_COMPRESSION::LEVEL_6, 6 },
			{ ZIP_COMPRESSION::LEVEL_7, 7 },
			{ ZIP_COMPRESSION::LEVEL_8, 8 },
			{ ZIP_COMPRESSION::LEVEL_9, 9 },
			{ ZIP_COMPRESSION::LEVEL_10, 10},
			{ ZIP_COMPRESSION::BEST_SPEED, MZ_BEST_SPEED},
			{ ZIP_COMPRESSION::BEST_COMPRESSION, MZ_BEST_COMPRESSION},
			{ ZIP_COMPRESSION::UBER_COMPRESSION, MZ_UBER_COMPRESSION},
			{ ZIP_COMPRESSION::DEFAULT_LEVEL, MZ_DEFAULT_LEVEL},
		};

		Zip::Zip()
		{
		}

		Zip::Zip(std::shared_ptr<const char> data, const size_t size, const bool is_unzip)
		{
			if (false == is_unzip)
			{
				_data = data;
				_size = size;
			}
			else
				_is_fine = Unzip(_files, data.get(), size, false);
		}

		Zip::Zip(const std::string & content, const bool is_unzip)
		{
			if (false == is_unzip)
				_content = content;
			else
				_is_fine = Unzip(_files, content.c_str(), content.size(), false);
		}

		bool Zip::IsFine() const
		{
			return _is_fine;
		}

		bool Zip::Unzip()
		{
			if (_content.empty())
				return true;
			_is_fine = Unzip(_files, _content.c_str(), _content.size(), false);
			_content = "";
			return _is_fine;
		}

		bool Zip::ToZip(std::shared_ptr<char>& zip, size_t & zip_size, const ZIP_COMPRESSION compression)
		{
			if (_data)
				return ZipAddFile(zip, zip_size, _data.get(), _size, _files, compression);
			else if (_content.empty() == false)
				return ZipAddFile(zip, zip_size, _content.c_str(), _content.length(), _files, compression);
			else
				return ToZip(zip, zip_size, _files, compression);
		}

		void Zip::AddFile(const std::string& fn, std::shared_ptr<const char> data, const size_t size)
		{
			_files.insert({ fn, std::shared_ptr<File>(new File(fn, data, size)) });
		}

		void Zip::AddFile(const std::string & fn, const std::string & content)
		{
			_files.insert({ fn, std::shared_ptr<File>(new File(fn, content)) });
		}

		void Zip::RemoveFile(const std::string & fn)
		{
			std::multimap<std::string, std::shared_ptr<File>>::iterator iter;
			while ((iter = _files.find(fn)) != _files.end())
				_files.erase(iter);
		}

		std::shared_ptr<const File> Zip::GetFile(const std::string& fn) const
		{
			auto iter = _files.find(fn);
			return iter == _files.end() ? nullptr : iter->second;
		}

		void Zip::GetFiles(std::multimap<std::string, std::shared_ptr<File>>& files) const
		{
			files = _files;
		}

		void Zip::GetFiles(std::multimap<std::string, std::string>& files) const
		{
			for (auto pair : _files)
				files.insert({ pair.first, std::string(pair.second->GetDataPtr(), pair.second->GetDataSize()) });
		}

		void Zip::GetFiles(std::map<std::string, std::shared_ptr<File>>& files) const
		{
			for (auto pair : _files)
				files[pair.first] = pair.second;
		}

		void Zip::GetFiles(std::map<std::string, std::string>& files) const
		{
			for (auto pair : _files)
				files[pair.first] = std::string(pair.second->GetDataPtr(), pair.second->GetDataSize());
		}

		bool Zip::Unzip(std::multimap<std::string, std::shared_ptr<File>>& out_files, const char * zip, const size_t zip_size, const bool is_fn_lowercase)
		{
			//out_files.clear();
			mz_zip_archive zip_archive;
			mz_zip_zero_struct(&zip_archive);

			auto status = mz_zip_reader_init_mem(&zip_archive, zip, zip_size, 0);
			if (!status)
				return false;
			int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
			if (fileCount == 0)
			{
				mz_zip_reader_end(&zip_archive);
				return false;
			}
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
			{
				mz_zip_reader_end(&zip_archive);
				return false;
			}
			// Get root folder
			std::string lastDir = "";
			// Get and print information about each file in the archive.
			for (int i = 0; i < fileCount; i++)
			{
				if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
					continue;
				if (mz_zip_reader_is_file_a_directory(&zip_archive, i))
					continue; // skip directories for now
							  //string fileName = file_stat.m_filename; // make path relative
							  //get_filename(fileName);
				size_t size = 0;
				auto ret = (char*)mz_zip_reader_extract_to_heap(&zip_archive, i, &size, 0);
				if (ret)
				{
					std::string fn= file_stat.m_filename;
					if (is_fn_lowercase)
						std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
					auto f = std::shared_ptr<File>(new File(fn, std::shared_ptr<const char>(ret), size));
					out_files.insert({ fn, f });
				}
			}

			// Close the archive, freeing any resources it was using
			mz_zip_reader_end(&zip_archive);
			return true;
		}

		bool Zip::RetrieveFiles(std::multimap<std::string, std::shared_ptr<File>>& out_files, CStr & zip_content, const std::vector<std::string>& files_to_get, const bool is_fn_lowercase)
		{
			out_files.clear();
			mz_zip_archive zip_archive;
			mz_zip_zero_struct(&zip_archive);

			auto status = mz_zip_reader_init_mem(&zip_archive, zip_content.c_str(), zip_content.length(), 0);
			if (!status)
				return false;
			int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
			if (fileCount == 0)
			{
				mz_zip_reader_end(&zip_archive);
				return false;
			}
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zip_archive, 0, &file_stat))
			{
				mz_zip_reader_end(&zip_archive);
				return false;
			}
			// Get root folder
			std::string lastDir = "";

			std::map<std::string, int> flags;
			// Get and print information about each file in the archive.
			for (int i = 0; i < fileCount; i++)
			{
				if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
					continue;
				if (std::find(files_to_get.begin(), files_to_get.end(), file_stat.m_filename) == files_to_get.end())
					continue;
				size_t size = 0;
				auto ret = (char*)mz_zip_reader_extract_to_heap(&zip_archive, i, &size, 0);
				if (ret)
				{
					std::string fn = file_stat.m_filename;
					if (is_fn_lowercase)
						std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
					auto f = std::shared_ptr<File>(new File(fn, std::shared_ptr<const char>(ret), size));
					out_files.insert({ fn, f });
					++flags[fn];
				}
			}

			// Close the archive, freeing any resources it was using
			mz_zip_reader_end(&zip_archive);

			return files_to_get.size() == flags.size();
		}

		bool Zip::ToZip(std::shared_ptr<char>& zip, size_t & zip_size, const std::map<std::string, std::string>& fn_data, const ZIP_COMPRESSION compression)
		{
			std::map<std::string, std::pair<const char*, size_t>> f;
			for (auto pair : fn_data)
				f.insert({ pair.first,{ pair.second.c_str(), pair.second.length() } });
			return ToZip(zip, zip_size, f, compression);
		}

		bool Zip::ToZip(std::shared_ptr<char>& zip, size_t& zip_size, const std::multimap<std::string, std::shared_ptr<File>>& files, const ZIP_COMPRESSION compression)
		{
			std::map<std::string, std::pair<const char*, size_t>> f;
			for (auto pair : files)
				f.insert({ pair.first, {pair.second->GetDataPtr(), pair.second->GetDataSize()} });
			return ToZip(zip, zip_size, f, compression);
		}

		bool Zip::ZipAddFile(std::shared_ptr<char>& zip, size_t & zip_size, const char * in_zip, const size_t in_zip_size, const std::map<std::string, std::string>& fn_data, const ZIP_COMPRESSION compression)
		{
			std::map<std::string, std::pair<const char*, size_t>> f;
			for (auto pair : fn_data)
				f.insert({ pair.first,{ pair.second.c_str(), pair.second.length() } });
			return ZipAddFile(zip, zip_size, in_zip, in_zip_size, f, compression);
		}

		bool Zip::ZipAddFile(std::shared_ptr<char>& zip, size_t & zip_size, const char * in_zip, const size_t in_zip_size, std::multimap<std::string, std::shared_ptr<File>>& files, const ZIP_COMPRESSION compression)
		{
			std::map<std::string, std::pair<const char*, size_t>> f;
			for (auto pair : files)
				f.insert({ pair.first,{ pair.second->GetDataPtr(), pair.second->GetDataSize() } });
			return ZipAddFile(zip, zip_size, in_zip, in_zip_size, f, compression);
		}

		bool Zip::ToZip(std::shared_ptr<char>& zip, size_t& zip_size, const std::map<std::string, std::pair<const char*, size_t>>& files, const ZIP_COMPRESSION compression)
		{
			zip_size = 0;
			mz_zip_archive zip_archive;
			mz_zip_zero_struct(&zip_archive);

			auto status = mz_zip_writer_init_heap(&zip_archive, 0, 0);
			if (!status)
				return false;
			for (auto& file : files)
				mz_zip_writer_add_mem(&zip_archive, file.first.c_str(), file.second.first, file.second.second, _compression[compression]);
			char* _zip = nullptr;
			mz_bool ret = mz_zip_writer_finalize_heap_archive(&zip_archive, (void**)&_zip, &zip_size);
			ret &= mz_zip_writer_end(&zip_archive);
			zip = std::shared_ptr<char>(_zip);
			return ret;
		}

		bool Zip::ZipAddFile(std::shared_ptr<char>& zip, size_t& zip_size, const char* in_zip, const size_t in_zip_size,
			const std::map<std::string, std::pair<const char*, size_t>>& files, const ZIP_COMPRESSION compression)
		{
			mz_zip_archive zip_archive;
			mz_zip_zero_struct(&zip_archive);

			//no need to release
			char* buf = new char[in_zip_size];
			memcpy(buf, in_zip, in_zip_size);

			auto status = mz_zip_reader_init_mem(&zip_archive, buf, in_zip_size, 0);
			if (!status)
				return false;
			mz_zip_writer_init_from_reader(&zip_archive, nullptr);
			for (auto& file : files)
				mz_zip_writer_add_mem(&zip_archive, file.first.c_str(), file.second.first, file.second.second, _compression[compression]);
			char* _zip = nullptr;
			mz_bool ret = mz_zip_writer_finalize_heap_archive(&zip_archive, (void**)&_zip, &zip_size);
			ret &= mz_zip_writer_end(&zip_archive);
			zip = std::shared_ptr<char>(_zip);


			return ret;
		}
	}
}
#endif //_YXL_COMPRESS_MINI_Z_
#endif //_YXL_COMPRESS_

#ifdef _YXL_CRYPTOGRAPHIC_
#ifdef _YXL_CRYPTOGRAPHIC_HASH_
namespace YXL
{
	namespace Hash
	{
		namespace _SHA1
		{
#undef BIG_ENDIAN_HOST  
#define u32 unsigned int

			/****************
			* Rotate a 32 bit integer by n bytes
			*/
#if defined(__GNUC__) && defined(__i386__)  
			static inline u32
				rol(u32 x, int n)
			{
				__asm__("roll %%cl,%0"
					:"=r" (x)
					: "0" (x), "c" (n));
				return x;
			}
#else  
#define rol(x,n) ( ((x) << (n)) | ((x) >> (32-(n))) )  
#endif  


			typedef struct {
				u32  h0, h1, h2, h3, h4;
				u32  nblocks;
				unsigned char buf[64];
				int  count;
			} SHA1_CONTEXT;


			void sha1_init(SHA1_CONTEXT *hd)
			{
				hd->h0 = 0x67452301;
				hd->h1 = 0xefcdab89;
				hd->h2 = 0x98badcfe;
				hd->h3 = 0x10325476;
				hd->h4 = 0xc3d2e1f0;
				hd->nblocks = 0;
				hd->count = 0;
			}


			/****************
			* Transform the message X which consists of 16 32-bit-words
			*/
			static void transform(SHA1_CONTEXT *hd, unsigned char *data)
			{
				u32 a, b, c, d, e, tm;
				u32 x[16];

				/* get values from the chaining vars */
				a = hd->h0;
				b = hd->h1;
				c = hd->h2;
				d = hd->h3;
				e = hd->h4;

#ifdef BIG_ENDIAN_HOST  
				memcpy(x, data, 64);
#else  
				{
					int i;
					unsigned char *p2;
					for (i = 0, p2 = (unsigned char*)x; i < 16; i++, p2 += 4)
					{
						p2[3] = *data++;
						p2[2] = *data++;
						p2[1] = *data++;
						p2[0] = *data++;
					}
				}
#endif  


#define K1  0x5A827999L  
#define K2  0x6ED9EBA1L  
#define K3  0x8F1BBCDCL  
#define K4  0xCA62C1D6L  
#define F1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )  
#define F2(x,y,z)   ( x ^ y ^ z )  
#define F3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )  
#define F4(x,y,z)   ( x ^ y ^ z )  
#define M(i) ( tm =   x[i&0x0f] ^ x[(i-14)&0x0f] ^ x[(i - 8) & 0x0f] ^ x[(i - 3) & 0x0f] , (x[i & 0x0f] = rol(tm, 1)))
#define R(a,b,c,d,e,f,k,m)  do { e += rol( a, 5 ) + f(b, c, d) + k + m;   	b = rol(b, 30); } while (0)

				R(a, b, c, d, e, F1, K1, x[0]);
				R(e, a, b, c, d, F1, K1, x[1]);
				R(d, e, a, b, c, F1, K1, x[2]);
				R(c, d, e, a, b, F1, K1, x[3]);
				R(b, c, d, e, a, F1, K1, x[4]);
				R(a, b, c, d, e, F1, K1, x[5]);
				R(e, a, b, c, d, F1, K1, x[6]);
				R(d, e, a, b, c, F1, K1, x[7]);
				R(c, d, e, a, b, F1, K1, x[8]);
				R(b, c, d, e, a, F1, K1, x[9]);
				R(a, b, c, d, e, F1, K1, x[10]);
				R(e, a, b, c, d, F1, K1, x[11]);
				R(d, e, a, b, c, F1, K1, x[12]);
				R(c, d, e, a, b, F1, K1, x[13]);
				R(b, c, d, e, a, F1, K1, x[14]);
				R(a, b, c, d, e, F1, K1, x[15]);
				R(e, a, b, c, d, F1, K1, M(16));
				R(d, e, a, b, c, F1, K1, M(17));
				R(c, d, e, a, b, F1, K1, M(18));
				R(b, c, d, e, a, F1, K1, M(19));
				R(a, b, c, d, e, F2, K2, M(20));
				R(e, a, b, c, d, F2, K2, M(21));
				R(d, e, a, b, c, F2, K2, M(22));
				R(c, d, e, a, b, F2, K2, M(23));
				R(b, c, d, e, a, F2, K2, M(24));
				R(a, b, c, d, e, F2, K2, M(25));
				R(e, a, b, c, d, F2, K2, M(26));
				R(d, e, a, b, c, F2, K2, M(27));
				R(c, d, e, a, b, F2, K2, M(28));
				R(b, c, d, e, a, F2, K2, M(29));
				R(a, b, c, d, e, F2, K2, M(30));
				R(e, a, b, c, d, F2, K2, M(31));
				R(d, e, a, b, c, F2, K2, M(32));
				R(c, d, e, a, b, F2, K2, M(33));
				R(b, c, d, e, a, F2, K2, M(34));
				R(a, b, c, d, e, F2, K2, M(35));
				R(e, a, b, c, d, F2, K2, M(36));
				R(d, e, a, b, c, F2, K2, M(37));
				R(c, d, e, a, b, F2, K2, M(38));
				R(b, c, d, e, a, F2, K2, M(39));
				R(a, b, c, d, e, F3, K3, M(40));
				R(e, a, b, c, d, F3, K3, M(41));
				R(d, e, a, b, c, F3, K3, M(42));
				R(c, d, e, a, b, F3, K3, M(43));
				R(b, c, d, e, a, F3, K3, M(44));
				R(a, b, c, d, e, F3, K3, M(45));
				R(e, a, b, c, d, F3, K3, M(46));
				R(d, e, a, b, c, F3, K3, M(47));
				R(c, d, e, a, b, F3, K3, M(48));
				R(b, c, d, e, a, F3, K3, M(49));
				R(a, b, c, d, e, F3, K3, M(50));
				R(e, a, b, c, d, F3, K3, M(51));
				R(d, e, a, b, c, F3, K3, M(52));
				R(c, d, e, a, b, F3, K3, M(53));
				R(b, c, d, e, a, F3, K3, M(54));
				R(a, b, c, d, e, F3, K3, M(55));
				R(e, a, b, c, d, F3, K3, M(56));
				R(d, e, a, b, c, F3, K3, M(57));
				R(c, d, e, a, b, F3, K3, M(58));
				R(b, c, d, e, a, F3, K3, M(59));
				R(a, b, c, d, e, F4, K4, M(60));
				R(e, a, b, c, d, F4, K4, M(61));
				R(d, e, a, b, c, F4, K4, M(62));
				R(c, d, e, a, b, F4, K4, M(63));
				R(b, c, d, e, a, F4, K4, M(64));
				R(a, b, c, d, e, F4, K4, M(65));
				R(e, a, b, c, d, F4, K4, M(66));
				R(d, e, a, b, c, F4, K4, M(67));
				R(c, d, e, a, b, F4, K4, M(68));
				R(b, c, d, e, a, F4, K4, M(69));
				R(a, b, c, d, e, F4, K4, M(70));
				R(e, a, b, c, d, F4, K4, M(71));
				R(d, e, a, b, c, F4, K4, M(72));
				R(c, d, e, a, b, F4, K4, M(73));
				R(b, c, d, e, a, F4, K4, M(74));
				R(a, b, c, d, e, F4, K4, M(75));
				R(e, a, b, c, d, F4, K4, M(76));
				R(d, e, a, b, c, F4, K4, M(77));
				R(c, d, e, a, b, F4, K4, M(78));
				R(b, c, d, e, a, F4, K4, M(79));

#undef K1
#undef K2
#undef K3
#undef K4
#undef F1
#undef F2
#undef F3
#undef F4
#undef M
#undef R
#undef rol

				/* Update chaining vars */
				hd->h0 += a;
				hd->h1 += b;
				hd->h2 += c;
				hd->h3 += d;
				hd->h4 += e;
			}

			/* Update the message digest with the contents
			* of INBUF with length INLEN.
			*/
			static void sha1_write(SHA1_CONTEXT *hd, unsigned char *inbuf, size_t inlen)
			{
				if (hd->count == 64) { /* flush the buffer */
					transform(hd, hd->buf);
					hd->count = 0;
					hd->nblocks++;
				}
				if (!inbuf)
					return;
				if (hd->count) {
					for (; inlen && hd->count < 64; inlen--)
						hd->buf[hd->count++] = *inbuf++;
					sha1_write(hd, NULL, 0);
					if (!inlen)
						return;
				}

				while (inlen >= 64) {
					transform(hd, inbuf);
					hd->count = 0;
					hd->nblocks++;
					inlen -= 64;
					inbuf += 64;
				}
				for (; inlen && hd->count < 64; inlen--)
					hd->buf[hd->count++] = *inbuf++;
			}


			/* The routine final terminates the computation and
			* returns the digest.
			* The handle is prepared for a new cycle, but adding bytes to the
			* handle will the destroy the returned buffer.
			* Returns: 20 bytes representing the digest.
			*/

			static void sha1_final(SHA1_CONTEXT *hd)
			{
				u32 t, msb, lsb;
				unsigned char *p;

				sha1_write(hd, NULL, 0); /* flush */;

				t = hd->nblocks;
				/* multiply by 64 to make a byte count */
				lsb = t << 6;
				msb = t >> 26;
				/* add the count */
				t = lsb;
				if ((lsb += hd->count) < t)
					msb++;
				/* multiply by 8 to make a bit count */
				t = lsb;
				lsb <<= 3;
				msb <<= 3;
				msb |= t >> 29;

				if (hd->count < 56) { /* enough room */
					hd->buf[hd->count++] = 0x80; /* pad */
					while (hd->count < 56)
						hd->buf[hd->count++] = 0;  /* pad */
				}
				else { /* need one extra block */
					hd->buf[hd->count++] = 0x80; /* pad character */
					while (hd->count < 64)
						hd->buf[hd->count++] = 0;
					sha1_write(hd, NULL, 0);  /* flush */;
					memset(hd->buf, 0, 56); /* fill next block with zeroes */
				}
				/* append the 64 bit count */
				hd->buf[56] = msb >> 24;
				hd->buf[57] = msb >> 16;
				hd->buf[58] = msb >> 8;
				hd->buf[59] = msb;
				hd->buf[60] = lsb >> 24;
				hd->buf[61] = lsb >> 16;
				hd->buf[62] = lsb >> 8;
				hd->buf[63] = lsb;
				transform(hd, hd->buf);

				p = hd->buf;
#ifdef BIG_ENDIAN_HOST  
#define X(a) do { *(u32*)p = hd->h##a ; p += 4; } while(0)  
#else /* little endian */  
#define X(a) do { *p++ = hd->h##a >> 24; *p++ = hd->h##a >> 16;    *p++ = hd->h##a >> 8; *p++ = hd->h##a; } while (0)
#endif  
				X(0);
				X(1);
				X(2);
				X(3);
				X(4);
#undef X  
			}

#undef u32
		}

		namespace _SHA2
		{
			/*
			YXL: this code is copy from https://github.com/ogay/sha2
			*/

			/*
			* FIPS 180-2 SHA-224/256/384/512 implementation
			* Last update: 02/02/2007
			* Issue date:  04/30/2005
			*
			* Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
			* All rights reserved.
			*
			* Redistribution and use in source and binary forms, with or without
			* modification, are permitted provided that the following conditions
			* are met:
			* 1. Redistributions of source code must retain the above copyright
			*    notice, this list of conditions and the following disclaimer.
			* 2. Redistributions in binary form must reproduce the above copyright
			*    notice, this list of conditions and the following disclaimer in the
			*    documentation and/or other materials provided with the distribution.
			* 3. Neither the name of the project nor the names of its contributors
			*    may be used to endorse or promote products derived from this software
			*    without specific prior written permission.
			*
			* THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
			* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
			* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
			* ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
			* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
			* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
			* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
			* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
			* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
			* SUCH DAMAGE.
			*/

#if 0
#define UNROLL_LOOPS /* Enable loops unrolling */
#endif

#include <string.h>

#define SHA224_DIGEST_SIZE ( 224 / 8)
#define SHA256_DIGEST_SIZE ( 256 / 8)
#define SHA384_DIGEST_SIZE ( 384 / 8)
#define SHA512_DIGEST_SIZE ( 512 / 8)

#define SHA256_BLOCK_SIZE  ( 512 / 8)
#define SHA512_BLOCK_SIZE  (1024 / 8)
#define SHA384_BLOCK_SIZE  SHA512_BLOCK_SIZE
#define SHA224_BLOCK_SIZE  SHA256_BLOCK_SIZE

#define uint8 unsigned char
#define uint32 unsigned int
#define uint64 unsigned long long

			typedef struct {
				unsigned int tot_len;
				unsigned int len;
				unsigned char block[2 * SHA256_BLOCK_SIZE];
				uint32 h[8];
			} sha256_ctx;

			typedef struct {
				unsigned int tot_len;
				unsigned int len;
				unsigned char block[2 * SHA512_BLOCK_SIZE];
				uint64 h[8];
			} sha512_ctx;

			typedef sha512_ctx sha384_ctx;
			typedef sha256_ctx sha224_ctx;

#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define SHA256_F1(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))

#define SHA512_F1(x) (ROTR(x, 28) ^ ROTR(x, 34) ^ ROTR(x, 39))
#define SHA512_F2(x) (ROTR(x, 14) ^ ROTR(x, 18) ^ ROTR(x, 41))
#define SHA512_F3(x) (ROTR(x,  1) ^ ROTR(x,  8) ^ SHFR(x,  7))
#define SHA512_F4(x) (ROTR(x, 19) ^ ROTR(x, 61) ^ SHFR(x,  6))

#define UNPACK32(x, str)                      \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}

#define PACK32(str, x)                        \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}

#define UNPACK64(x, str)                      \
{                                             \
    *((str) + 7) = (uint8) ((x)      );       \
    *((str) + 6) = (uint8) ((x) >>  8);       \
    *((str) + 5) = (uint8) ((x) >> 16);       \
    *((str) + 4) = (uint8) ((x) >> 24);       \
    *((str) + 3) = (uint8) ((x) >> 32);       \
    *((str) + 2) = (uint8) ((x) >> 40);       \
    *((str) + 1) = (uint8) ((x) >> 48);       \
    *((str) + 0) = (uint8) ((x) >> 56);       \
}

#define PACK64(str, x)                        \
{                                             \
    *(x) =   ((uint64) *((str) + 7)      )    \
           | ((uint64) *((str) + 6) <<  8)    \
           | ((uint64) *((str) + 5) << 16)    \
           | ((uint64) *((str) + 4) << 24)    \
           | ((uint64) *((str) + 3) << 32)    \
           | ((uint64) *((str) + 2) << 40)    \
           | ((uint64) *((str) + 1) << 48)    \
           | ((uint64) *((str) + 0) << 56);   \
}

			/* Macros used for loops unrolling */

#define SHA256_SCR(i)                         \
{                                             \
    w[i] =  SHA256_F4(w[i -  2]) + w[i -  7]  \
          + SHA256_F3(w[i - 15]) + w[i - 16]; \
}

#define SHA512_SCR(i)                         \
{                                             \
    w[i] =  SHA512_F4(w[i -  2]) + w[i -  7]  \
          + SHA512_F3(w[i - 15]) + w[i - 16]; \
}

#define SHA256_EXP(a, b, c, d, e, f, g, h, j)               \
{                                                           \
    t1 = wv[h] + SHA256_F2(wv[e]) + CH(wv[e], wv[f], wv[g]) \
         + sha256_k[j] + w[j];                              \
    t2 = SHA256_F1(wv[a]) + MAJ(wv[a], wv[b], wv[c]);       \
    wv[d] += t1;                                            \
    wv[h] = t1 + t2;                                        \
}

#define SHA512_EXP(a, b, c, d, e, f, g ,h, j)               \
{                                                           \
    t1 = wv[h] + SHA512_F2(wv[e]) + CH(wv[e], wv[f], wv[g]) \
         + sha512_k[j] + w[j];                              \
    t2 = SHA512_F1(wv[a]) + MAJ(wv[a], wv[b], wv[c]);       \
    wv[d] += t1;                                            \
    wv[h] = t1 + t2;                                        \
}

			uint32 sha224_h0[8] =
			{ 0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
				0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4 };

			uint32 sha256_h0[8] =
			{ 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
				0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };

			uint64 sha384_h0[8] =
			{ 0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL,
				0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
				0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL,
				0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL };

			uint64 sha512_h0[8] =
			{ 0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
				0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
				0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
				0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL };

			uint32 sha256_k[64] =
			{ 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
				0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
				0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
				0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
				0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
				0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
				0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
				0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
				0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

			uint64 sha512_k[80] =
			{ 0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL,
				0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
				0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL,
				0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
				0xd807aa98a3030242ULL, 0x12835b0145706fbeULL,
				0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
				0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL,
				0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
				0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL,
				0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
				0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL,
				0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
				0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL,
				0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
				0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
				0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
				0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL,
				0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
				0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL,
				0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
				0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL,
				0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
				0xd192e819d6ef5218ULL, 0xd69906245565a910ULL,
				0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
				0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL,
				0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
				0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL,
				0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
				0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL,
				0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
				0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL,
				0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
				0xca273eceea26619cULL, 0xd186b8c721c0c207ULL,
				0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
				0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL,
				0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
				0x28db77f523047d84ULL, 0x32caab7b40c72493ULL,
				0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
				0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL,
				0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL };

			/* SHA-256 functions */

			void sha256_transf(sha256_ctx *ctx, const unsigned char *message,
				unsigned int block_nb)
			{
				uint32 w[64];
				uint32 wv[8];
				uint32 t1, t2;
				const unsigned char *sub_block;
				int i;

#ifndef UNROLL_LOOPS
				int j;
#endif

				for (i = 0; i < (int)block_nb; i++) {
					sub_block = message + (i << 6);

#ifndef UNROLL_LOOPS
					for (j = 0; j < 16; j++) {
						PACK32(&sub_block[j << 2], &w[j]);
					}

					for (j = 16; j < 64; j++) {
						SHA256_SCR(j);
					}

					for (j = 0; j < 8; j++) {
						wv[j] = ctx->h[j];
					}

					for (j = 0; j < 64; j++) {
						t1 = wv[7] + SHA256_F2(wv[4]) + CH(wv[4], wv[5], wv[6])
							+ sha256_k[j] + w[j];
						t2 = SHA256_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
						wv[7] = wv[6];
						wv[6] = wv[5];
						wv[5] = wv[4];
						wv[4] = wv[3] + t1;
						wv[3] = wv[2];
						wv[2] = wv[1];
						wv[1] = wv[0];
						wv[0] = t1 + t2;
					}

					for (j = 0; j < 8; j++) {
						ctx->h[j] += wv[j];
					}
#else
					PACK32(&sub_block[0], &w[0]); PACK32(&sub_block[4], &w[1]);
					PACK32(&sub_block[8], &w[2]); PACK32(&sub_block[12], &w[3]);
					PACK32(&sub_block[16], &w[4]); PACK32(&sub_block[20], &w[5]);
					PACK32(&sub_block[24], &w[6]); PACK32(&sub_block[28], &w[7]);
					PACK32(&sub_block[32], &w[8]); PACK32(&sub_block[36], &w[9]);
					PACK32(&sub_block[40], &w[10]); PACK32(&sub_block[44], &w[11]);
					PACK32(&sub_block[48], &w[12]); PACK32(&sub_block[52], &w[13]);
					PACK32(&sub_block[56], &w[14]); PACK32(&sub_block[60], &w[15]);

					SHA256_SCR(16); SHA256_SCR(17); SHA256_SCR(18); SHA256_SCR(19);
					SHA256_SCR(20); SHA256_SCR(21); SHA256_SCR(22); SHA256_SCR(23);
					SHA256_SCR(24); SHA256_SCR(25); SHA256_SCR(26); SHA256_SCR(27);
					SHA256_SCR(28); SHA256_SCR(29); SHA256_SCR(30); SHA256_SCR(31);
					SHA256_SCR(32); SHA256_SCR(33); SHA256_SCR(34); SHA256_SCR(35);
					SHA256_SCR(36); SHA256_SCR(37); SHA256_SCR(38); SHA256_SCR(39);
					SHA256_SCR(40); SHA256_SCR(41); SHA256_SCR(42); SHA256_SCR(43);
					SHA256_SCR(44); SHA256_SCR(45); SHA256_SCR(46); SHA256_SCR(47);
					SHA256_SCR(48); SHA256_SCR(49); SHA256_SCR(50); SHA256_SCR(51);
					SHA256_SCR(52); SHA256_SCR(53); SHA256_SCR(54); SHA256_SCR(55);
					SHA256_SCR(56); SHA256_SCR(57); SHA256_SCR(58); SHA256_SCR(59);
					SHA256_SCR(60); SHA256_SCR(61); SHA256_SCR(62); SHA256_SCR(63);

					wv[0] = ctx->h[0]; wv[1] = ctx->h[1];
					wv[2] = ctx->h[2]; wv[3] = ctx->h[3];
					wv[4] = ctx->h[4]; wv[5] = ctx->h[5];
					wv[6] = ctx->h[6]; wv[7] = ctx->h[7];

					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 0); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 1);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 2); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 3);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 4); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 5);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 6); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 7);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 8); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 9);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 10); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 11);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 12); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 13);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 14); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 15);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 16); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 17);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 18); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 19);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 20); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 21);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 22); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 23);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 24); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 25);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 26); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 27);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 28); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 29);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 30); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 31);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 32); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 33);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 34); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 35);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 36); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 37);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 38); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 39);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 40); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 41);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 42); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 43);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 44); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 45);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 46); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 47);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 48); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 49);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 50); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 51);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 52); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 53);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 54); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 55);
					SHA256_EXP(0, 1, 2, 3, 4, 5, 6, 7, 56); SHA256_EXP(7, 0, 1, 2, 3, 4, 5, 6, 57);
					SHA256_EXP(6, 7, 0, 1, 2, 3, 4, 5, 58); SHA256_EXP(5, 6, 7, 0, 1, 2, 3, 4, 59);
					SHA256_EXP(4, 5, 6, 7, 0, 1, 2, 3, 60); SHA256_EXP(3, 4, 5, 6, 7, 0, 1, 2, 61);
					SHA256_EXP(2, 3, 4, 5, 6, 7, 0, 1, 62); SHA256_EXP(1, 2, 3, 4, 5, 6, 7, 0, 63);

					ctx->h[0] += wv[0]; ctx->h[1] += wv[1];
					ctx->h[2] += wv[2]; ctx->h[3] += wv[3];
					ctx->h[4] += wv[4]; ctx->h[5] += wv[5];
					ctx->h[6] += wv[6]; ctx->h[7] += wv[7];
#endif /* !UNROLL_LOOPS */
				}
			}

			void sha256_init(sha256_ctx *ctx)
			{
#ifndef UNROLL_LOOPS
				int i;
				for (i = 0; i < 8; i++) {
					ctx->h[i] = sha256_h0[i];
				}
#else
				ctx->h[0] = sha256_h0[0]; ctx->h[1] = sha256_h0[1];
				ctx->h[2] = sha256_h0[2]; ctx->h[3] = sha256_h0[3];
				ctx->h[4] = sha256_h0[4]; ctx->h[5] = sha256_h0[5];
				ctx->h[6] = sha256_h0[6]; ctx->h[7] = sha256_h0[7];
#endif /* !UNROLL_LOOPS */

				ctx->len = 0;
				ctx->tot_len = 0;
			}

			void sha256_update(sha256_ctx *ctx, const unsigned char *message,
				unsigned int len)
			{
				unsigned int block_nb;
				unsigned int new_len, rem_len, tmp_len;
				const unsigned char *shifted_message;

				tmp_len = SHA256_BLOCK_SIZE - ctx->len;
				rem_len = len < tmp_len ? len : tmp_len;

				memcpy(&ctx->block[ctx->len], message, rem_len);

				if (ctx->len + len < SHA256_BLOCK_SIZE) {
					ctx->len += len;
					return;
				}

				new_len = len - rem_len;
				block_nb = new_len / SHA256_BLOCK_SIZE;

				shifted_message = message + rem_len;

				sha256_transf(ctx, ctx->block, 1);
				sha256_transf(ctx, shifted_message, block_nb);

				rem_len = new_len % SHA256_BLOCK_SIZE;

				memcpy(ctx->block, &shifted_message[block_nb << 6],
					rem_len);

				ctx->len = rem_len;
				ctx->tot_len += (block_nb + 1) << 6;
			}

			void sha256_final(sha256_ctx *ctx, unsigned char *digest)
			{
				unsigned int block_nb;
				unsigned int pm_len;
				unsigned int len_b;

#ifndef UNROLL_LOOPS
				int i;
#endif

				block_nb = (1 + ((SHA256_BLOCK_SIZE - 9)
					< (ctx->len % SHA256_BLOCK_SIZE)));

				len_b = (ctx->tot_len + ctx->len) << 3;
				pm_len = block_nb << 6;

				memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
				ctx->block[ctx->len] = 0x80;
				UNPACK32(len_b, ctx->block + pm_len - 4);

				sha256_transf(ctx, ctx->block, block_nb);

#ifndef UNROLL_LOOPS
				for (i = 0; i < 8; i++) {
					UNPACK32(ctx->h[i], &digest[i << 2]);
				}
#else
				UNPACK32(ctx->h[0], &digest[0]);
				UNPACK32(ctx->h[1], &digest[4]);
				UNPACK32(ctx->h[2], &digest[8]);
				UNPACK32(ctx->h[3], &digest[12]);
				UNPACK32(ctx->h[4], &digest[16]);
				UNPACK32(ctx->h[5], &digest[20]);
				UNPACK32(ctx->h[6], &digest[24]);
				UNPACK32(ctx->h[7], &digest[28]);
#endif /* !UNROLL_LOOPS */
			}

			void sha256(const unsigned char *message, unsigned int len, unsigned char *digest)
			{
				sha256_ctx ctx;

				sha256_init(&ctx);
				sha256_update(&ctx, message, len);
				sha256_final(&ctx, digest);
			}

			/* SHA-512 functions */

			void sha512_transf(sha512_ctx *ctx, const unsigned char *message,
				unsigned int block_nb)
			{
				uint64 w[80];
				uint64 wv[8];
				uint64 t1, t2;
				const unsigned char *sub_block;
				int i, j;

				for (i = 0; i < (int)block_nb; i++) {
					sub_block = message + (i << 7);

#ifndef UNROLL_LOOPS
					for (j = 0; j < 16; j++) {
						PACK64(&sub_block[j << 3], &w[j]);
					}

					for (j = 16; j < 80; j++) {
						SHA512_SCR(j);
					}

					for (j = 0; j < 8; j++) {
						wv[j] = ctx->h[j];
					}

					for (j = 0; j < 80; j++) {
						t1 = wv[7] + SHA512_F2(wv[4]) + CH(wv[4], wv[5], wv[6])
							+ sha512_k[j] + w[j];
						t2 = SHA512_F1(wv[0]) + MAJ(wv[0], wv[1], wv[2]);
						wv[7] = wv[6];
						wv[6] = wv[5];
						wv[5] = wv[4];
						wv[4] = wv[3] + t1;
						wv[3] = wv[2];
						wv[2] = wv[1];
						wv[1] = wv[0];
						wv[0] = t1 + t2;
					}

					for (j = 0; j < 8; j++) {
						ctx->h[j] += wv[j];
					}
#else
					PACK64(&sub_block[0], &w[0]); PACK64(&sub_block[8], &w[1]);
					PACK64(&sub_block[16], &w[2]); PACK64(&sub_block[24], &w[3]);
					PACK64(&sub_block[32], &w[4]); PACK64(&sub_block[40], &w[5]);
					PACK64(&sub_block[48], &w[6]); PACK64(&sub_block[56], &w[7]);
					PACK64(&sub_block[64], &w[8]); PACK64(&sub_block[72], &w[9]);
					PACK64(&sub_block[80], &w[10]); PACK64(&sub_block[88], &w[11]);
					PACK64(&sub_block[96], &w[12]); PACK64(&sub_block[104], &w[13]);
					PACK64(&sub_block[112], &w[14]); PACK64(&sub_block[120], &w[15]);

					SHA512_SCR(16); SHA512_SCR(17); SHA512_SCR(18); SHA512_SCR(19);
					SHA512_SCR(20); SHA512_SCR(21); SHA512_SCR(22); SHA512_SCR(23);
					SHA512_SCR(24); SHA512_SCR(25); SHA512_SCR(26); SHA512_SCR(27);
					SHA512_SCR(28); SHA512_SCR(29); SHA512_SCR(30); SHA512_SCR(31);
					SHA512_SCR(32); SHA512_SCR(33); SHA512_SCR(34); SHA512_SCR(35);
					SHA512_SCR(36); SHA512_SCR(37); SHA512_SCR(38); SHA512_SCR(39);
					SHA512_SCR(40); SHA512_SCR(41); SHA512_SCR(42); SHA512_SCR(43);
					SHA512_SCR(44); SHA512_SCR(45); SHA512_SCR(46); SHA512_SCR(47);
					SHA512_SCR(48); SHA512_SCR(49); SHA512_SCR(50); SHA512_SCR(51);
					SHA512_SCR(52); SHA512_SCR(53); SHA512_SCR(54); SHA512_SCR(55);
					SHA512_SCR(56); SHA512_SCR(57); SHA512_SCR(58); SHA512_SCR(59);
					SHA512_SCR(60); SHA512_SCR(61); SHA512_SCR(62); SHA512_SCR(63);
					SHA512_SCR(64); SHA512_SCR(65); SHA512_SCR(66); SHA512_SCR(67);
					SHA512_SCR(68); SHA512_SCR(69); SHA512_SCR(70); SHA512_SCR(71);
					SHA512_SCR(72); SHA512_SCR(73); SHA512_SCR(74); SHA512_SCR(75);
					SHA512_SCR(76); SHA512_SCR(77); SHA512_SCR(78); SHA512_SCR(79);

					wv[0] = ctx->h[0]; wv[1] = ctx->h[1];
					wv[2] = ctx->h[2]; wv[3] = ctx->h[3];
					wv[4] = ctx->h[4]; wv[5] = ctx->h[5];
					wv[6] = ctx->h[6]; wv[7] = ctx->h[7];

					j = 0;

					do {
						SHA512_EXP(0, 1, 2, 3, 4, 5, 6, 7, j); j++;
						SHA512_EXP(7, 0, 1, 2, 3, 4, 5, 6, j); j++;
						SHA512_EXP(6, 7, 0, 1, 2, 3, 4, 5, j); j++;
						SHA512_EXP(5, 6, 7, 0, 1, 2, 3, 4, j); j++;
						SHA512_EXP(4, 5, 6, 7, 0, 1, 2, 3, j); j++;
						SHA512_EXP(3, 4, 5, 6, 7, 0, 1, 2, j); j++;
						SHA512_EXP(2, 3, 4, 5, 6, 7, 0, 1, j); j++;
						SHA512_EXP(1, 2, 3, 4, 5, 6, 7, 0, j); j++;
					} while (j < 80);

					ctx->h[0] += wv[0]; ctx->h[1] += wv[1];
					ctx->h[2] += wv[2]; ctx->h[3] += wv[3];
					ctx->h[4] += wv[4]; ctx->h[5] += wv[5];
					ctx->h[6] += wv[6]; ctx->h[7] += wv[7];
#endif /* !UNROLL_LOOPS */
				}
			}

			void sha512_init(sha512_ctx *ctx)
			{
#ifndef UNROLL_LOOPS
				int i;
				for (i = 0; i < 8; i++) {
					ctx->h[i] = sha512_h0[i];
				}
#else
				ctx->h[0] = sha512_h0[0]; ctx->h[1] = sha512_h0[1];
				ctx->h[2] = sha512_h0[2]; ctx->h[3] = sha512_h0[3];
				ctx->h[4] = sha512_h0[4]; ctx->h[5] = sha512_h0[5];
				ctx->h[6] = sha512_h0[6]; ctx->h[7] = sha512_h0[7];
#endif /* !UNROLL_LOOPS */

				ctx->len = 0;
				ctx->tot_len = 0;
			}

			void sha512_update(sha512_ctx *ctx, const unsigned char *message,
				unsigned int len)
			{
				unsigned int block_nb;
				unsigned int new_len, rem_len, tmp_len;
				const unsigned char *shifted_message;

				tmp_len = SHA512_BLOCK_SIZE - ctx->len;
				rem_len = len < tmp_len ? len : tmp_len;

				memcpy(&ctx->block[ctx->len], message, rem_len);

				if (ctx->len + len < SHA512_BLOCK_SIZE) {
					ctx->len += len;
					return;
				}

				new_len = len - rem_len;
				block_nb = new_len / SHA512_BLOCK_SIZE;

				shifted_message = message + rem_len;

				sha512_transf(ctx, ctx->block, 1);
				sha512_transf(ctx, shifted_message, block_nb);

				rem_len = new_len % SHA512_BLOCK_SIZE;

				memcpy(ctx->block, &shifted_message[block_nb << 7],
					rem_len);

				ctx->len = rem_len;
				ctx->tot_len += (block_nb + 1) << 7;
			}

			void sha512_final(sha512_ctx *ctx, unsigned char *digest)
			{
				unsigned int block_nb;
				unsigned int pm_len;
				unsigned int len_b;

#ifndef UNROLL_LOOPS
				int i;
#endif

				block_nb = 1 + ((SHA512_BLOCK_SIZE - 17)
					< (ctx->len % SHA512_BLOCK_SIZE));

				len_b = (ctx->tot_len + ctx->len) << 3;
				pm_len = block_nb << 7;

				memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
				ctx->block[ctx->len] = 0x80;
				UNPACK32(len_b, ctx->block + pm_len - 4);

				sha512_transf(ctx, ctx->block, block_nb);

#ifndef UNROLL_LOOPS
				for (i = 0; i < 8; i++) {
					UNPACK64(ctx->h[i], &digest[i << 3]);
				}
#else
				UNPACK64(ctx->h[0], &digest[0]);
				UNPACK64(ctx->h[1], &digest[8]);
				UNPACK64(ctx->h[2], &digest[16]);
				UNPACK64(ctx->h[3], &digest[24]);
				UNPACK64(ctx->h[4], &digest[32]);
				UNPACK64(ctx->h[5], &digest[40]);
				UNPACK64(ctx->h[6], &digest[48]);
				UNPACK64(ctx->h[7], &digest[56]);
#endif /* !UNROLL_LOOPS */
			}

			void sha512(const unsigned char *message, unsigned int len,
				unsigned char *digest)
			{
				sha512_ctx ctx;

				sha512_init(&ctx);
				sha512_update(&ctx, message, len);
				sha512_final(&ctx, digest);
			}

			void sha384_init(sha384_ctx *ctx)
			{
#ifndef UNROLL_LOOPS
				int i;
				for (i = 0; i < 8; i++) {
					ctx->h[i] = sha384_h0[i];
				}
#else
				ctx->h[0] = sha384_h0[0]; ctx->h[1] = sha384_h0[1];
				ctx->h[2] = sha384_h0[2]; ctx->h[3] = sha384_h0[3];
				ctx->h[4] = sha384_h0[4]; ctx->h[5] = sha384_h0[5];
				ctx->h[6] = sha384_h0[6]; ctx->h[7] = sha384_h0[7];
#endif /* !UNROLL_LOOPS */

				ctx->len = 0;
				ctx->tot_len = 0;
			}

			void sha384_update(sha384_ctx *ctx, const unsigned char *message,
				unsigned int len)
			{
				unsigned int block_nb;
				unsigned int new_len, rem_len, tmp_len;
				const unsigned char *shifted_message;

				tmp_len = SHA384_BLOCK_SIZE - ctx->len;
				rem_len = len < tmp_len ? len : tmp_len;

				memcpy(&ctx->block[ctx->len], message, rem_len);

				if (ctx->len + len < SHA384_BLOCK_SIZE) {
					ctx->len += len;
					return;
				}

				new_len = len - rem_len;
				block_nb = new_len / SHA384_BLOCK_SIZE;

				shifted_message = message + rem_len;

				sha512_transf(ctx, ctx->block, 1);
				sha512_transf(ctx, shifted_message, block_nb);

				rem_len = new_len % SHA384_BLOCK_SIZE;

				memcpy(ctx->block, &shifted_message[block_nb << 7],
					rem_len);

				ctx->len = rem_len;
				ctx->tot_len += (block_nb + 1) << 7;
			}

			void sha384_final(sha384_ctx *ctx, unsigned char *digest)
			{
				unsigned int block_nb;
				unsigned int pm_len;
				unsigned int len_b;

#ifndef UNROLL_LOOPS
				int i;
#endif

				block_nb = (1 + ((SHA384_BLOCK_SIZE - 17)
					< (ctx->len % SHA384_BLOCK_SIZE)));

				len_b = (ctx->tot_len + ctx->len) << 3;
				pm_len = block_nb << 7;

				memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
				ctx->block[ctx->len] = 0x80;
				UNPACK32(len_b, ctx->block + pm_len - 4);

				sha512_transf(ctx, ctx->block, block_nb);

#ifndef UNROLL_LOOPS
				for (i = 0; i < 6; i++) {
					UNPACK64(ctx->h[i], &digest[i << 3]);
				}
#else
				UNPACK64(ctx->h[0], &digest[0]);
				UNPACK64(ctx->h[1], &digest[8]);
				UNPACK64(ctx->h[2], &digest[16]);
				UNPACK64(ctx->h[3], &digest[24]);
				UNPACK64(ctx->h[4], &digest[32]);
				UNPACK64(ctx->h[5], &digest[40]);
#endif /* !UNROLL_LOOPS */
			}

			/* SHA-384 functions */

			void sha384(const unsigned char *message, unsigned int len,
				unsigned char *digest)
			{
				sha384_ctx ctx;

				sha384_init(&ctx);
				sha384_update(&ctx, message, len);
				sha384_final(&ctx, digest);
			}

			void sha224_init(sha224_ctx *ctx)
			{
#ifndef UNROLL_LOOPS
				int i;
				for (i = 0; i < 8; i++) {
					ctx->h[i] = sha224_h0[i];
				}
#else
				ctx->h[0] = sha224_h0[0]; ctx->h[1] = sha224_h0[1];
				ctx->h[2] = sha224_h0[2]; ctx->h[3] = sha224_h0[3];
				ctx->h[4] = sha224_h0[4]; ctx->h[5] = sha224_h0[5];
				ctx->h[6] = sha224_h0[6]; ctx->h[7] = sha224_h0[7];
#endif /* !UNROLL_LOOPS */

				ctx->len = 0;
				ctx->tot_len = 0;
			}

			void sha224_update(sha224_ctx *ctx, const unsigned char *message,
				unsigned int len)
			{
				unsigned int block_nb;
				unsigned int new_len, rem_len, tmp_len;
				const unsigned char *shifted_message;

				tmp_len = SHA224_BLOCK_SIZE - ctx->len;
				rem_len = len < tmp_len ? len : tmp_len;

				memcpy(&ctx->block[ctx->len], message, rem_len);

				if (ctx->len + len < SHA224_BLOCK_SIZE) {
					ctx->len += len;
					return;
				}

				new_len = len - rem_len;
				block_nb = new_len / SHA224_BLOCK_SIZE;

				shifted_message = message + rem_len;

				sha256_transf(ctx, ctx->block, 1);
				sha256_transf(ctx, shifted_message, block_nb);

				rem_len = new_len % SHA224_BLOCK_SIZE;

				memcpy(ctx->block, &shifted_message[block_nb << 6],
					rem_len);

				ctx->len = rem_len;
				ctx->tot_len += (block_nb + 1) << 6;
			}

			void sha224_final(sha224_ctx *ctx, unsigned char *digest)
			{
				unsigned int block_nb;
				unsigned int pm_len;
				unsigned int len_b;

#ifndef UNROLL_LOOPS
				int i;
#endif

				block_nb = (1 + ((SHA224_BLOCK_SIZE - 9)
					< (ctx->len % SHA224_BLOCK_SIZE)));

				len_b = (ctx->tot_len + ctx->len) << 3;
				pm_len = block_nb << 6;

				memset(ctx->block + ctx->len, 0, pm_len - ctx->len);
				ctx->block[ctx->len] = 0x80;
				UNPACK32(len_b, ctx->block + pm_len - 4);

				sha256_transf(ctx, ctx->block, block_nb);

#ifndef UNROLL_LOOPS
				for (i = 0; i < 7; i++) {
					UNPACK32(ctx->h[i], &digest[i << 2]);
				}
#else
				UNPACK32(ctx->h[0], &digest[0]);
				UNPACK32(ctx->h[1], &digest[4]);
				UNPACK32(ctx->h[2], &digest[8]);
				UNPACK32(ctx->h[3], &digest[12]);
				UNPACK32(ctx->h[4], &digest[16]);
				UNPACK32(ctx->h[5], &digest[20]);
				UNPACK32(ctx->h[6], &digest[24]);
#endif /* !UNROLL_LOOPS */
			}

			/* SHA-224 functions */

			void sha224(const unsigned char *message, unsigned int len,
				unsigned char *digest)
			{
				sha224_ctx ctx;

				sha224_init(&ctx);
				sha224_update(&ctx, message, len);
				sha224_final(&ctx, digest);
			}

#undef SHA224_DIGEST_SIZE
#undef SHA256_DIGEST_SIZE
#undef SHA384_DIGEST_SIZE
#undef SHA512_DIGEST_SIZE

#undef SHA256_BLOCK_SIZE
#undef SHA512_BLOCK_SIZE
#undef SHA384_BLOCK_SIZE
#undef SHA224_BLOCK_SIZE

#undef uint8
#undef uint32
#undef uint64

#undef SHFR
#undef ROTR
#undef ROTL
#undef CH
#undef MAJ

#undef SHA256_F1
#undef SHA256_F2
#undef SHA256_F3
#undef SHA256_F4

#undef SHA512_F1
#undef SHA512_F2
#undef SHA512_F3
#undef SHA512_F4

#undef UNPACK32
#undef PACK32
#undef UNPACK64
#undef PACK64

#undef SHA256_SCR
#undef SHA512_SCR
#undef SHA256_EXP
#undef SHA512_EXP
		}

		namespace _MD5
		{
			/*
			YXL: this code is copy from https://github.com/prophetss/MD5-SHA2-AES
			*/

			/*
			* This code implements the MD5 message-digest algorithm.
			* The algorithm is due to Ron Rivest.  This code was
			* written by Colin Plumb in 1993, no copyright is claimed.
			* This code is in the public domain; do with it what you wish.
			*
			* Equivalent code is available from RSA Data Security, Inc.
			* This code has been tested against that, and is equivalent,
			* except that you don't need to include two pages of legalese
			* with every copy.
			*
			* To compute the message digest of a chunk of bytes, declare an
			* MD5Context structure, pass it to MD5Init, call MD5Update as
			* needed on buffers full of bytes, and then call MD5Final, which
			* will fill a supplied 16-byte array with the digest.
			*/

#define MD5_HASHBYTES 16

#define u_int32_t unsigned int

			typedef struct MD5Context {
				u_int32_t buf[4];
				u_int32_t bits[2];
				unsigned char in[64];
			} MD5_CTX;

#if __BYTE_ORDER == 1234
#define byteReverse(buf, len)	/* Nothing */
#else
			void byteReverse(unsigned char *buf, unsigned longs);

			/*
			* Note: this code is harmless on little-endian machines.
			*/
			void byteReverse(unsigned char *buf, unsigned longs)
			{
				u_int32_t t;
				do {
					t = (u_int32_t)((unsigned)buf[3] << 8 | buf[2]) << 16 |
						((unsigned)buf[1] << 8 | buf[0]);
					*(u_int32_t *)buf = t;
					buf += 4;
				} while (--longs);
			}
#endif

			void MD5Transform(u_int32_t buf[4], u_int32_t const in[16]);

			/*
			* Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
			* initialization constants.
			*/
			void MD5Init(MD5_CTX *ctx)
			{
				ctx->buf[0] = 0x67452301;
				ctx->buf[1] = 0xefcdab89;
				ctx->buf[2] = 0x98badcfe;
				ctx->buf[3] = 0x10325476;

				ctx->bits[0] = 0;
				ctx->bits[1] = 0;
			}

			/*
			* Update context to reflect the concatenation of another buffer full
			* of bytes.
			*/
			void MD5Update(MD5_CTX *ctx, unsigned char const *buf, unsigned len)
			{
				u_int32_t t;

				/* Update bitcount */

				t = ctx->bits[0];
				if ((ctx->bits[0] = t + ((u_int32_t)len << 3)) < t)
					ctx->bits[1]++;		/* Carry from low to high */
				ctx->bits[1] += len >> 29;

				t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

										/* Handle any leading odd-sized chunks */

				if (t) {
					unsigned char *p = (unsigned char *)ctx->in + t;

					t = 64 - t;
					if (len < t) {
						memcpy(p, buf, len);
						return;
					}
					memcpy(p, buf, t);
					byteReverse(ctx->in, 16);
					MD5Transform(ctx->buf, (u_int32_t *)ctx->in);
					buf += t;
					len -= t;
				}
				/* Process data in 64-byte chunks */

				while (len >= 64) {
					memcpy(ctx->in, buf, 64);
					byteReverse(ctx->in, 16);
					MD5Transform(ctx->buf, (u_int32_t *)ctx->in);
					buf += 64;
					len -= 64;
				}

				/* Handle any remaining bytes of data. */

				memcpy(ctx->in, buf, len);
			}

			/*
			* Final wrapup - pad to 64-byte boundary with the bit pattern
			* 1 0* (64-bit count of bits processed, MSB-first)
			*/
			void MD5Final(unsigned char digest[MD5_HASHBYTES], MD5_CTX *ctx)
			{
				unsigned count;
				unsigned char *p;

				/* Compute number of bytes mod 64 */
				count = (ctx->bits[0] >> 3) & 0x3F;

				/* Set the first char of padding to 0x80.  This is safe since there is
				always at least one byte free */
				p = ctx->in + count;
				*p++ = 0x80;

				/* Bytes of padding needed to make 64 bytes */
				count = 64 - 1 - count;

				/* Pad out to 56 mod 64 */
				if (count < 8) {
					/* Two lots of padding:  Pad the first block to 64 bytes */
					memset(p, 0, count);
					byteReverse(ctx->in, 16);
					MD5Transform(ctx->buf, (u_int32_t *)ctx->in);

					/* Now fill the next block with 56 bytes */
					memset(ctx->in, 0, 56);
				}
				else {
					/* Pad block to 56 bytes */
					memset(p, 0, count - 8);
				}
				byteReverse(ctx->in, 14);

				/* Append length in bits and transform */
				((u_int32_t *)ctx->in)[14] = ctx->bits[0];
				((u_int32_t *)ctx->in)[15] = ctx->bits[1];

				MD5Transform(ctx->buf, (u_int32_t *)ctx->in);
				byteReverse((unsigned char *)ctx->buf, 4);
				memcpy(digest, ctx->buf, 16);
				memset((char *)ctx, 0, sizeof(ctx));	/* In case it's sensitive */
			}

			/* The four core functions - F1 is optimized somewhat */

			/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

			/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )



			/*
			* The core of the MD5 algorithm, this alters an existing MD5 hash to
			* reflect the addition of 16 longwords of new data.  MD5Update blocks
			* the data and converts bytes into longwords for this routine.
			*/
			void MD5Transform(u_int32_t buf[4], u_int32_t const in[16])
			{
				register u_int32_t a, b, c, d;

				a = buf[0];
				b = buf[1];
				c = buf[2];
				d = buf[3];

				MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
				MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
				MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
				MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
				MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
				MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
				MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
				MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
				MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
				MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
				MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
				MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
				MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
				MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
				MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
				MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

				MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
				MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
				MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
				MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
				MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
				MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
				MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
				MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
				MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
				MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
				MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
				MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
				MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
				MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
				MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
				MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

				MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
				MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
				MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
				MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
				MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
				MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
				MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
				MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
				MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
				MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
				MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
				MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
				MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
				MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
				MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
				MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

				MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
				MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
				MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
				MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
				MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
				MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
				MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
				MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
				MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
				MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
				MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
				MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
				MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
				MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
				MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
				MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

				buf[0] += a;
				buf[1] += b;
				buf[2] += c;
				buf[3] += d;
			}

			void MD5End(MD5_CTX *ctx)
			{
				int i;
				unsigned char digest[MD5_HASHBYTES];
				static const char hex[] = "0123456789abcdef";
				
				MD5Final(digest, ctx);
				for (i = 0; i<MD5_HASHBYTES; i++) {
					ctx->in[i*2] = hex[digest[i] >> 4];
					ctx->in[i*2 + 1] = hex[digest[i] & 0x0f];
				}
				ctx->in[i*2] = '\0';
			}

#undef MD5_HASHBYTES
#undef F1
#undef F2
#undef F3
#undef F4
#undef MD5STEP

#undef u_int32_t
		}

		std::string ToStr(unsigned char* res, int len)
		{
			std::string tmp2(len,' ');
			std::string  hexstr = "0123456789abcdef";

			for (int i(0); i != len/2; ++i)
			{
				tmp2[i * 2] = hexstr[int(unsigned int(res[i]) >> 4)];
				tmp2[i * 2 + 1] = hexstr[int(unsigned int(res[i]) & 15u)];
			}
			return tmp2;
		}

		std::string SHA1(CStr& str)
		{
			using namespace _SHA1;

			SHA1_CONTEXT ctx;

			std::string tmp = str;

			sha1_init(&ctx);
			sha1_write(&ctx, (unsigned char*)&str[0], str.length());
			sha1_final(&ctx);

			return ToStr(ctx.buf, 40);
		}

		std::string SHA224(CStr& str)
		{
			unsigned char digest[56];
			_SHA2::sha224((const unsigned char*)str.c_str(), str.length(), digest);
			return ToStr(digest, 56);
		}

		std::string SHA256(CStr& str)
		{
			unsigned char digest[64];
			_SHA2::sha256((const unsigned char*)str.c_str(), str.length(), digest);
			return ToStr(digest, 64);
		}

		std::string SHA384(CStr& str)
		{
			unsigned char digest[96];
			_SHA2::sha384((const unsigned char*)str.c_str(), str.length(), digest);
			return ToStr(digest, 96);
		}

		std::string SHA512(CStr& str)
		{
			unsigned char digest[128];
			_SHA2::sha512((const unsigned char*)str.c_str(), str.length(), digest);
			return ToStr(digest, 128);
		}

		std::string MD5(CStr& str)
		{
			using namespace _MD5;

			MD5_CTX ctx;

			MD5Init(&ctx);
			MD5Update(&ctx, (const unsigned char*)str.data(), str.length());
			MD5End(&ctx);

			return std::string((char*)ctx.in);
		}
	}
}
#endif //_YXL_CRYPTOGRAPHIC_HASH_

#ifdef _YXL_CRYPTOGRAPHIC_CIPHER_
namespace YXL
{
	namespace Cipher
	{
		namespace _AES_ECB
		{
			/*
			YXL: this code is copy from https://github.com/dhuertas/AES
			*/

#if defined(__linux__) || defined(__linux)
#include <unistd.h>
#define _O_BINARY	0
#endif

#define AES_SUCCESS	0
#define AES_ERROR	-1		

			/*
			* Addition in GF(2^8)
			* http://en.wikipedia.org/wiki/Finite_field_arithmetic
			*/
			uint8_t gadd(uint8_t a, uint8_t b) {
				return a^b;
			}

			/*
			* Subtraction in GF(2^8)
			* http://en.wikipedia.org/wiki/Finite_field_arithmetic
			*/
			uint8_t gsub(uint8_t a, uint8_t b) {
				return a^b;
			}

			/*
			* Multiplication in GF(2^8)
			* http://en.wikipedia.org/wiki/Finite_field_arithmetic
			* Irreducible polynomial m(x) = x8 + x4 + x3 + x + 1
			*/
			uint8_t gmult(uint8_t a, uint8_t b) {

				uint8_t p = 0, i = 0, hbs = 0;

				for (i = 0; i < 8; i++) {
					if (b & 1) {
						p ^= a;
					}

					hbs = a & 0x80;
					a <<= 1;
					if (hbs) a ^= 0x1b; // 0000 0001 0001 1011	
					b >>= 1;
				}

				return (uint8_t)p;
			}

			/*
			* Addition of 4 byte words
			* m(x) = x4+1
			*/
			void coef_add(uint8_t a[], uint8_t b[], uint8_t d[]) {

				d[0] = a[0] ^ b[0];
				d[1] = a[1] ^ b[1];
				d[2] = a[2] ^ b[2];
				d[3] = a[3] ^ b[3];
			}

			/*
			* Multiplication of 4 byte words
			* m(x) = x4+1
			*/
			void coef_mult(uint8_t *a, uint8_t *b, uint8_t *d) {

				d[0] = gmult(a[0], b[0]) ^ gmult(a[3], b[1]) ^ gmult(a[2], b[2]) ^ gmult(a[1], b[3]);
				d[1] = gmult(a[1], b[0]) ^ gmult(a[0], b[1]) ^ gmult(a[3], b[2]) ^ gmult(a[2], b[3]);
				d[2] = gmult(a[2], b[0]) ^ gmult(a[1], b[1]) ^ gmult(a[0], b[2]) ^ gmult(a[3], b[3]);
				d[3] = gmult(a[3], b[0]) ^ gmult(a[2], b[1]) ^ gmult(a[1], b[2]) ^ gmult(a[0], b[3]);
			}

			/*
			* The cipher Key.
			*/
			int K;

			/*
			* Number of columns (32-bit words) comprising the State. For this
			* standard, Nb = 4.
			*/
			int Nb = 4;

			/*
			* Number of 32-bit words comprising the Cipher Key. For this
			* standard, Nk = 4, 6, or 8.
			*/
			int Nk;

			/*
			* Number of rounds, which is a function of  Nk  and  Nb (which is
			* fixed). For this standard, Nr = 10, 12, or 14.
			*/
			int Nr;

			/*
			* S-box transformation table
			*/
			static uint8_t s_box[256] = {
				// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
				0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, // 0
				0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, // 1
				0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, // 2
				0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, // 3
				0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, // 4
				0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, // 5
				0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, // 6
				0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, // 7
				0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, // 8
				0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, // 9
				0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, // a
				0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, // b
				0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, // c
				0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, // d
				0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, // e
				0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };// f

			/*
			* Inverse S-box transformation table
			*/
			static uint8_t inv_s_box[256] = {
				// 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
				0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb, // 0
				0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb, // 1
				0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e, // 2
				0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25, // 3
				0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92, // 4
				0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84, // 5
				0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06, // 6
				0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b, // 7
				0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73, // 8
				0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e, // 9
				0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b, // a
				0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4, // b
				0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f, // c
				0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef, // d
				0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61, // e
				0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };// f

			/*
			* Generates the round constant Rcon[i]
			*/
			uint8_t R[] = { 0x02, 0x00, 0x00, 0x00 };

			uint8_t * Rcon(uint8_t i) {

				if (i == 1) {
					R[0] = 0x01; // x^(1-1) = x^0 = 1
				}
				else if (i > 1) {
					R[0] = 0x02;
					i--;
					while (i - 1 > 0) {
						R[0] = gmult(R[0], 0x02);
						i--;
					}
				}

				return R;
			}

			/*
			* Transformation in the Cipher and Inverse Cipher in which a Round
			* Key is added to the State using an XOR operation. The length of a
			* Round Key equals the size of the State (i.e., for Nb = 4, the Round
			* Key length equals 128 bits/16 bytes).
			*/
			void add_round_key(uint8_t *state, uint8_t *w, uint8_t r) {

				uint8_t c;

				for (c = 0; c < Nb; c++) {
					state[Nb * 0 + c] = state[Nb * 0 + c] ^ w[4 * Nb*r + 4 * c + 0];   //debug, so it works for Nb !=4 
					state[Nb * 1 + c] = state[Nb * 1 + c] ^ w[4 * Nb*r + 4 * c + 1];
					state[Nb * 2 + c] = state[Nb * 2 + c] ^ w[4 * Nb*r + 4 * c + 2];
					state[Nb * 3 + c] = state[Nb * 3 + c] ^ w[4 * Nb*r + 4 * c + 3];
				}
			}

			/*
			* Transformation in the Cipher that takes all of the columns of the
			* State and mixes their data (independently of one another) to
			* produce new columns.
			*/
			void mix_columns(uint8_t *state) {

				uint8_t a[] = { 0x02, 0x01, 0x01, 0x03 }; // a(x) = {02} + {01}x + {01}x2 + {03}x3
				uint8_t i, j, col[4], res[4];

				for (j = 0; j < Nb; j++) {
					for (i = 0; i < 4; i++) {
						col[i] = state[Nb*i + j];
					}

					coef_mult(a, col, res);

					for (i = 0; i < 4; i++) {
						state[Nb*i + j] = res[i];
					}
				}
			}

			/*
			* Transformation in the Inverse Cipher that is the inverse of
			* MixColumns().
			*/
			void inv_mix_columns(uint8_t *state) {

				uint8_t a[] = { 0x0e, 0x09, 0x0d, 0x0b }; // a(x) = {0e} + {09}x + {0d}x2 + {0b}x3
				uint8_t i, j, col[4], res[4];

				for (j = 0; j < Nb; j++) {
					for (i = 0; i < 4; i++) {
						col[i] = state[Nb*i + j];
					}

					coef_mult(a, col, res);

					for (i = 0; i < 4; i++) {
						state[Nb*i + j] = res[i];
					}
				}
			}

			/*
			* Transformation in the Cipher that processes the State by cyclically
			* shifting the last three rows of the State by different offsets.
			*/
			void shift_rows(uint8_t *state) {

				uint8_t i, k, s, tmp;

				for (i = 1; i < 4; i++) {
					// shift(1,4)=1; shift(2,4)=2; shift(3,4)=3
					// shift(r, 4) = r;
					s = 0;
					while (s < i) {
						tmp = state[Nb*i + 0];

						for (k = 1; k < Nb; k++) {
							state[Nb*i + k - 1] = state[Nb*i + k];
						}

						state[Nb*i + Nb - 1] = tmp;
						s++;
					}
				}
			}

			/*
			* Transformation in the Inverse Cipher that is the inverse of
			* ShiftRows().
			*/
			void inv_shift_rows(uint8_t *state) {

				uint8_t i, k, s, tmp;

				for (i = 1; i < 4; i++) {
					s = 0;
					while (s < i) {
						tmp = state[Nb*i + Nb - 1];

						for (k = Nb - 1; k > 0; k--) {
							state[Nb*i + k] = state[Nb*i + k - 1];
						}

						state[Nb*i + 0] = tmp;
						s++;
					}
				}
			}

			/*
			* Transformation in the Cipher that processes the State using a non­
			* linear byte substitution table (S-box) that operates on each of the
			* State bytes independently.
			*/
			void sub_bytes(uint8_t *state) {

				uint8_t i, j;
				uint8_t row, col;

				for (i = 0; i < 4; i++) {
					for (j = 0; j < Nb; j++) {
						row = (state[Nb*i + j] & 0xf0) >> 4;
						col = state[Nb*i + j] & 0x0f;
						state[Nb*i + j] = s_box[16 * row + col];
					}
				}
			}

			/*
			* Transformation in the Inverse Cipher that is the inverse of
			* SubBytes().
			*/
			void inv_sub_bytes(uint8_t *state) {

				uint8_t i, j;
				uint8_t row, col;

				for (i = 0; i < 4; i++) {
					for (j = 0; j < Nb; j++) {
						row = (state[Nb*i + j] & 0xf0) >> 4;
						col = state[Nb*i + j] & 0x0f;
						state[Nb*i + j] = inv_s_box[16 * row + col];
					}
				}
			}

			/*
			* Function used in the Key Expansion routine that takes a four-byte
			* input word and applies an S-box to each of the four bytes to
			* produce an output word.
			*/
			void sub_word(uint8_t *w) {

				uint8_t i;

				for (i = 0; i < 4; i++) {
					w[i] = s_box[16 * ((w[i] & 0xf0) >> 4) + (w[i] & 0x0f)];
				}
			}

			/*
			* Function used in the Key Expansion routine that takes a four-byte
			* word and performs a cyclic permutation.
			*/
			void rot_word(uint8_t *w) {

				uint8_t tmp;
				uint8_t i;

				tmp = w[0];

				for (i = 0; i < 3; i++) {
					w[i] = w[i + 1];
				}

				w[3] = tmp;
			}

			/*
			* Key Expansion
			*/
			void key_expansion(const uint8_t *key, uint8_t *w) {

				uint8_t tmp[4];
				uint8_t i;
				uint8_t len = Nb*(Nr + 1);

				for (i = 0; i < Nk; i++) {
					w[4 * i + 0] = key[4 * i + 0];
					w[4 * i + 1] = key[4 * i + 1];
					w[4 * i + 2] = key[4 * i + 2];
					w[4 * i + 3] = key[4 * i + 3];
				}

				for (i = Nk; i < len; i++) {
					tmp[0] = w[4 * (i - 1) + 0];
					tmp[1] = w[4 * (i - 1) + 1];
					tmp[2] = w[4 * (i - 1) + 2];
					tmp[3] = w[4 * (i - 1) + 3];

					if (i%Nk == 0) {

						rot_word(tmp);
						sub_word(tmp);
						coef_add(tmp, Rcon(i / Nk), tmp);

					}
					else if (Nk > 6 && i%Nk == 4) {

						sub_word(tmp);

					}

					w[4 * i + 0] = w[4 * (i - Nk) + 0] ^ tmp[0];
					w[4 * i + 1] = w[4 * (i - Nk) + 1] ^ tmp[1];
					w[4 * i + 2] = w[4 * (i - Nk) + 2] ^ tmp[2];
					w[4 * i + 3] = w[4 * (i - Nk) + 3] ^ tmp[3];
				}
			}

			/*int Nb=4, for  compile error*/
#define NB_T	4
			void cipher(const uint8_t *in, uint8_t *out, uint8_t *w) {

				uint8_t state[4 * NB_T];
				uint8_t r, i, j;

				for (i = 0; i < 4; i++) {
					for (j = 0; j < Nb; j++) {
						state[Nb*i + j] = in[i + 4 * j];
					}
				}

				add_round_key(state, w, 0);

				for (r = 1; r < Nr; r++) {
					sub_bytes(state);
					shift_rows(state);
					mix_columns(state);
					add_round_key(state, w, r);
				}

				sub_bytes(state);
				shift_rows(state);
				add_round_key(state, w, Nr);

				for (i = 0; i < 4; i++) {
					for (j = 0; j < Nb; j++) {
						out[i + 4 * j] = state[Nb*i + j];
					}
				}
			}

			void inv_cipher(const uint8_t *in, uint8_t *out, uint8_t *w) {

				uint8_t state[4 * NB_T];
				uint8_t r, i, j;

				for (i = 0; i < 4; i++) {
					for (j = 0; j < Nb; j++) {
						state[Nb*i + j] = in[i + 4 * j];
					}
				}

				add_round_key(state, w, Nr);

				for (r = Nr - 1; r >= 1; r--) {
					inv_shift_rows(state);
					inv_sub_bytes(state);
					add_round_key(state, w, r);
					inv_mix_columns(state);
				}

				inv_shift_rows(state);
				inv_sub_bytes(state);
				add_round_key(state, w, 0);

				for (i = 0; i < 4; i++) {
					for (j = 0; j < Nb; j++) {
						out[i + 4 * j] = state[Nb*i + j];
					}
				}
			}

			/* 密钥初始化 */
			static int aes_init(const uint8_t *key, size_t key_len, uint8_t **w)
			{
				if (*w != NULL)
					return AES_SUCCESS;

				switch (key_len) {
				default:
				case 16: Nk = 4; Nr = 10; break;
				case 24: Nk = 6; Nr = 12; break;
				case 32: Nk = 8; Nr = 14; break;
				}

				*w = new unsigned char[Nb*(Nr + 1) * 4];
				key_expansion(key, *w);

				return AES_SUCCESS;
			}

#undef AES_BUFSIZ

#undef AES_SUCCESS
#undef AES_ERROR

#undef NB_T

#if defined(__linux__) || defined(__linux)
#undef _O_BINARY
#endif
		}

		void AESCipherECB(CStr& str, CStr& key, std::string& res)
		{
#define NB_T	4
			using namespace _AES_ECB;
			const uint8_t* in = (const uint8_t*)str.data();
			res.resize((str.length()+ 4 * NB_T-1)/(4*NB_T)*(4*NB_T));
			uint8_t* out= (uint8_t*)res.data();

			size_t quotient = str.length() >> 4;
			uint8_t *w = NULL; // expanded key
			aes_init((uint8_t*)&key[0], key.length(), &w);
			int i;
#pragma omp parallel for
			for (i = 0; i < quotient; i++)
				cipher(in + i * 4 * Nb, out + i * 4 * Nb, w);

			/* padding */
			uint8_t padding_bit[4 * NB_T];
			uint8_t j = 0;
			quotient = quotient << 4;
			for (i = quotient; i < str.length(); i++, j++)
				padding_bit[i - quotient] = in[i];

			for (i = j; i < 4 * Nb; i++)
				padding_bit[i] = 4 * Nb - j;

			cipher(padding_bit, out + quotient, w);
#undef NB_T
		}

		void AESDecryptECB(CStr& str, CStr& key, std::string& res)
		{
			using namespace _AES_ECB;
			size_t quotient = str.length() >> 4;
			uint8_t *w = NULL; // expanded key
			aes_init((uint8_t*)&key[0], key.length(), &w);

			const uint8_t* in = (const uint8_t*)str.data();
			res.resize(str.length());
			uint8_t* out = (uint8_t*)res.data();

			int i;
#pragma omp parallel for
			for (i = 0; i < quotient; i++)
				inv_cipher(in + i * 4 * Nb, out + i * 4 * Nb, w);

			/* padding length */
			uint8_t j = out[str.length() - 1];

			///* remove padding data */
			res.resize(res.length() - j);
			//*out_len = in_len - j;

			/*for (i = 1; i <= j; i++)
				out[in_len - i] = '\0';*/
		}

	}
}
#endif //_YXL_CRYPTOGRAPHIC_CIPHER_

#ifdef _YXL_CRYPTOGRAPHIC_CRYPTO_
namespace YXL
{
	namespace Crypt
	{
		CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption _CryptoPPBase::s_globalRNG;
		bool _CryptoPPBase::_is_init = false;
	}
}
#endif //_YXL_CRYPTOGRAPHIC_CRYPTO_

#endif //_YXL_CRYPTOGRAPHIC_

#ifdef _YXL_GLFW_
namespace YXL
{
	static std::map<GLFWwindow*, GLFWBase*> s_glfw_map;
	void _GLFWKeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
	{
		if (s_glfw_map.find(window) != s_glfw_map.end())
			s_glfw_map[window]->KeyCallback(key, scancode, action, mods);
	}
	void _GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (s_glfw_map.find(window) != s_glfw_map.end())
			s_glfw_map[window]->MouseButtonCallback(button, action, mods);
	}
	void _GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (s_glfw_map.find(window) != s_glfw_map.end())
			s_glfw_map[window]->ScrollCallback(xoffset, yoffset);
	}
	void _GLFWCursorPosCallback(GLFWwindow* window, double x, double y)
	{
		if (s_glfw_map.find(window) != s_glfw_map.end())
			s_glfw_map[window]->CursorPosCallback(x, y);
	}

	bool GLFWBase::Init(const int wnd_w, const int wnd_h, const bool hidden)
	{
		if (wnd_w <= 0 || wnd_h <= 0)
			return false;
		static bool inited = false;
		if(inited==false)
			glfwInit();
		if(hidden)
			glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		_wnd = glfwCreateWindow(1280, 720, "", NULL, NULL);
		glfwMakeContextCurrent(_wnd);
		
		auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwSetWindowPos(_wnd, (mode->width-wnd_w)/2, (mode->height-wnd_h)/2);

		glfwSwapInterval(0);

		if (inited == false)
		{
			GLenum err = glewInit();
			if (GLEW_OK != err)
				return false;
		}

		s_glfw_map[_wnd] = this;
		glfwSetKeyCallback(_wnd, YXL::_GLFWKeyCallback);
		glfwSetMouseButtonCallback(_wnd, YXL::_GLFWMouseButtonCallback);
		glfwSetCursorPosCallback(_wnd, _GLFWCursorPosCallback);
		glfwSetScrollCallback(_wnd, YXL::_GLFWScrollCallback);

		inited = true;

		return true;
	}
	void GLFWBase::Run()
	{
		while (!glfwWindowShouldClose(_wnd))
		{
			glfwMakeContextCurrent(_wnd);
			BeforeFrame(_frame_id);
			Frame(_frame_id);
			AfterFrame(_frame_id);
			glfwSwapBuffers(_wnd);
			glfwPollEvents();
			_frame_id++;
		}
		CleanUp();
	}
}

#endif //_YXL_GLFW_

