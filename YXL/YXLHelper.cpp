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
		FILE *f = fopen(_S(fName), "w");
		if (f == NULL)
			return false;
		for (size_t i = 0; i < strs.size(); i++)
			fprintf(f, "%s\n", _S(strs[i]));
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

	std::string File::BrowseFile(const char * strFilter, bool isOpen, const std::string & def_dir, CStr & title)
	{
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
		ofn.lpstrFilter = strFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST;

		ofn.lpstrInitialDir = &opened_dir[0];
		ofn.lpstrTitle = title.c_str();

		if (isOpen) {
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			GetOpenFileNameA(&ofn);
			SetCurrentDirectoryA(dir.c_str());
			return Buffer;
		}

		GetSaveFileNameA(&ofn);

		SetCurrentDirectoryA(dir.c_str());

		return std::string(Buffer);
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

			return std::string(Buffer);
		}
		return std::string();
	}

	bool File::MkDir(CStr&  path)
	{
		if (path.size() == 0)
			return false;

		static char buffer[1024];
		strcpy_s(buffer, sizeof(buffer), _S(path));
		for (int i = 0; buffer[i] != 0; i++) {
			if (buffer[i] == '\\' || buffer[i] == '/') {
				buffer[i] = '\0';
				CreateDirectoryA(buffer, 0);
				buffer[i] = '/';
			}
		}
		return CreateDirectoryA(_S(path), 0)==TRUE;
	}

	int File::Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(srcNames, names, inDir);
		for (int i = 0; i < fNum; i++) {
			std::string dstName = cv::format("%s\\%.4d%s.%s", _S(dstDir), i, nameCommon, nameExt);
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
			::DeleteFileA(_S(dir + names[i]));
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
			RunProgram("Cmd.exe", cv::format("/c rmdir /s /q \"%s\"", _S(dir)), true, false);
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
		HANDLE hFind = ::FindFirstFileA(_S(nameW), &fileFindData);
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
		if (!FileExist(_S(runExeFile)))
			runExeFile = fileName;

		std::string wkDir = GetWkDir();

		SHELLEXECUTEINFOA  ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = _S(runExeFile);
		ShExecInfo.lpParameters = _S(parameters);
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
		auto dir = QFileDialog::getExistingDirectory(nullptr, QString::fromLocal8Bit(_S(title)));
		return (std::string)dir.toLocal8Bit();
	}

	std::string File::BrowseFile(const char* strFilter, bool isOpen, const std::string& def_dir, CStr& title)
	{
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

		QString qPath;
		if (isOpen)
			qPath = QFileDialog::getOpenFileName(nullptr, QString::fromLocal8Bit(_S(title)), QString::fromLocal8Bit(_S(opened_dir)), QString::fromLocal8Bit(strFilter));
		else
			qPath = QFileDialog::getSaveFileName(nullptr, QString::fromLocal8Bit(_S(title)), QString::fromLocal8Bit(_S(opened_dir)), QString::fromLocal8Bit(strFilter));

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
		strcpy(buffer, _S(_path));
		for (int i = 0; buffer[i] != 0; i++) {
			if (buffer[i] == '\\' || buffer[i] == '/') {
				buffer[i] = '\0';
				dir.mkdir(QString::fromLocal8Bit(buffer));
				buffer[i] = '/';
			}
		}
		return dir.mkdir(QString::fromLocal8Bit(_S(_path)));
	}

	int File::Rename(CStr& _srcNames, CStr& _dstDir, const char *nameCommon, const char *nameExt)
	{
		vecS names;
		std::string inDir;
		int fNum = GetNames(_srcNames, names, inDir);
		for (int i = 0; i < fNum; i++) {
			std::string dstName = cv::format("%s\\%.4d%s.%s", _S(_dstDir), i, nameCommon, nameExt);
			std::string srcName = inDir + names[i];
			QFile file;
			file.rename(QString::fromLocal8Bit(_S(srcName)), QString::fromLocal8Bit(_S(dstName)));
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

		QDir dir(QString::fromLocal8Bit(_S(folder)));
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
		if (!FileExist(_S(runExeFile)))
			runExeFile = fileName;

		std::string wkDir = GetWkDir();

		QProcess proc;
		QStringList param(QString::fromLocal8Bit(_S(parameters)));

		proc.start(QString::fromLocal8Bit(_S(fileName)), param);

		if (waiteF)
			proc.waitForFinished();
	}

#endif
#endif
}
#endif

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
#endif

#ifdef _YXL_OTHER_
namespace YXL
{
	namespace SHA1
	{
#undef BIG_ENDIAN_HOST  
		typedef unsigned int u32;

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

		
	}

	std::string SHA1Digest(std::string str)
	{
		using namespace SHA1;

		SHA1_CONTEXT ctx;

		unsigned char* tmp = new unsigned char[str.length()];
		for (int i(0); i != str.length(); ++i)
			tmp[i] = str[i];

		sha1_init(&ctx);
		sha1_write(&ctx, tmp, str.length());
		sha1_final(&ctx);

		delete[] tmp;

		char tmp2[41];
		std::string  hexstr = "0123456789abcdef";

		for (int i(0); i != 20; ++i)
		{
			tmp2[i * 2] = hexstr[int(u32(ctx.buf[i]) >> 4)];
			tmp2[i * 2 + 1] = hexstr[int(u32(ctx.buf[i]) & 15u)];
		}
		tmp2[40] = '\0';

		return tmp2;
	}
}
#endif

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
#endif

#ifdef _YXL_GLFW_
namespace YXL
{
	static std::map<GLFWwindow*, GLFWBase*> s_glfw_map;
	void GLFWKeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
	{
		if (s_glfw_map.find(window) != s_glfw_map.end())
			s_glfw_map[window]->KeyCallback(key, scancode, action, mods);
	}

	bool GLFWBase::Init(const int wnd_w, const int wnd_h, const bool hidden)
	{
		if (wnd_w <= 0 || wnd_h <= 0)
			return false;
		glfwInit();
		if(hidden)
			glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		_wnd = glfwCreateWindow(1280, 720, "", NULL, NULL);
		glfwMakeContextCurrent(_wnd);
		
		auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwSetWindowPos(_wnd, (mode->width-wnd_w)/2, (mode->height-wnd_h)/2);

		glfwSwapInterval(0);

		GLenum err = glewInit();
		if (GLEW_OK != err)
			return false;

		s_glfw_map[_wnd] = this;
		glfwSetKeyCallback(_wnd, YXL::GLFWKeyCallback);
		return true;
	}
	void GLFWBase::Run()
	{
		while (!glfwWindowShouldClose(_wnd))
		{
			glfwMakeContextCurrent(_wnd);
			Frame(_frame_id++);
			glfwSwapBuffers(_wnd);
			glfwPollEvents();
		}
		CleanUp();
	}
}

#endif