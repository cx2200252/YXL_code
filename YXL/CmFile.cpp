/******************************************************
									CmFile.cpp


******************************************************/
#define _LIB_CMFILE_IMPL
#pragma warning(disable:4819)

#include <algorithm>
#include "CmFile.h"
#ifdef _WITH_WINDOWS_
#include <shlobj.h>
#include <Commdlg.h>
#include <ShellAPI.h>
#endif

//this file is adopted from the source code provided by the author of paper "BING: Binarized Normed Gradients for Objectness Estimation at 300fps"

vecS CmFile::loadStrList(CStr &fName)
{
	std::ifstream fIn(fName);
	std::string line;
	vecS strs;
	while (getline(fIn, line) && line.size())
		strs.push_back(line);
	return strs;
}

bool CmFile::writeStrList(CStr &fName, const vecS &strs)
{
	FILE *f=fopen(_S(fName), "w");
	if (f == NULL)
		return false;
	for (size_t i = 0; i < strs.size(); i++)
		fprintf(f, "%s\n", _S(strs[i]));
	fclose(f);
	return true;
}



#ifdef _WITH_WINDOWS_
BOOL CmFile::MkDir(CStr&  _path)
{
	if (_path.size() == 0)
		return FALSE;

	static char buffer[1024];
	strcpy_s(buffer, sizeof(buffer), _S(_path));
	for (int i = 0; buffer[i] != 0; i++) {
		if (buffer[i] == '\\' || buffer[i] == '/') {
			buffer[i] = '\0';
			CreateDirectoryA(buffer, 0);
			buffer[i] = '/';
		}
	}
	return CreateDirectoryA(_S(_path), 0);
}

std::string CmFile::BrowseFolder(CStr& title)
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

std::string CmFile::BrowseFile(const char* strFilter, bool isOpen, const std::string& def_dir, CStr& title)
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

int CmFile::Rename(CStr& _srcNames, CStr& _dstDir, const char *nameCommon, const char *nameExt)
{
	vecS names;
	std::string inDir;
	int fNum = GetNames(_srcNames, names, inDir);
	for (int i = 0; i < fNum; i++) {
		std::string dstName = cv::format("%s\\%.4d%s.%s", _S(_dstDir), i, nameCommon, nameExt);
		std::string srcName = inDir + names[i];
		::CopyFileA(srcName.c_str(), dstName.c_str(), FALSE);
	}
	return fNum;
}

void CmFile::RmFolder(CStr& dir)
{
	CleanFolder(dir);
	if (FolderExist(dir))
		RunProgram("Cmd.exe", cv::format("/c rmdir /s /q \"%s\"", _S(dir)), true, false);
}

void CmFile::CleanFolder(CStr& dir, bool subFolder)
{
	vecS names;
	int fNum = CmFile::GetNames(dir + "/*.*", names);
	for (int i = 0; i < fNum; i++)
		RmFile(dir + "/" + names[i]);

	vecS subFolders;
	int subNum = GetSubFolders(dir, subFolders);
	if (subFolder)
		for (int i = 0; i < subNum; i++)
			CleanFolder(dir + "/" + subFolders[i], true);
}

int CmFile::GetSubFolders(CStr& folder, vecS& subFolders)
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

// Get image names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
int CmFile::GetNames(CStr &nameW, vecS &names, std::string &dir)
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

int CmFile::GetNames(CStr& rootFolder, CStr &fileW, vecS &names)
{
	GetNames(rootFolder + fileW, names);
	vecS subFolders, tmpNames;
	int subNum = CmFile::GetSubFolders(rootFolder, subFolders);
	for (int i = 0; i < subNum; i++) {
		subFolders[i] += "/";
		int subNum = GetNames(rootFolder + subFolders[i], fileW, tmpNames);
		for (int j = 0; j < subNum; j++)
			names.push_back(subFolders[i] + tmpNames[j]);
	}
	return (int)names.size();
}

int CmFile::GetNamesNE(CStr& nameWC, vecS &names, std::string &dir, std::string &ext)
{
	int fNum = GetNames(nameWC, names, dir);
	ext = GetExtention(nameWC);
	for (int i = 0; i < fNum; i++)
		names[i] = GetNameNE(names[i]);
	return fNum;
}

int CmFile::GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names)
{
	int fNum = GetNames(rootFolder, fileW, names);
	size_t extS = GetExtention(fileW).size();
	for (int i = 0; i < fNum; i++)
		names[i].resize(names[i].size() - extS);
	return fNum;
}

BOOL CmFile::Move2Dir(CStr &srcW, CStr dstDir)
{
	vecS names;
	std::string inDir;
	int fNum = CmFile::GetNames(srcW, names, inDir);
	BOOL r = TRUE;
	for (int i = 0; i < fNum; i++)
		if (Move(inDir + names[i], dstDir + names[i]) == FALSE)
			r = FALSE;
	return r;
}

BOOL CmFile::Copy2Dir(CStr &srcW, CStr dstDir)
{
	vecS names;
	std::string inDir;
	int fNum = CmFile::GetNames(srcW, names, inDir);
	BOOL r = TRUE;
	for (int i = 0; i < fNum; i++)
		if (Copy(inDir + names[i], dstDir + names[i]) == FALSE)
			r = FALSE;
	return r;
}

void CmFile::RunProgram(CStr &fileName, CStr &parameters, bool waiteF, bool showW)
{
	std::string runExeFile = fileName;
#ifdef _DEBUG
	runExeFile.insert(0, "..\\Debug\\");
#else
	runExeFile.insert(0, "..\\Release\\");
#endif // _DEBUG
	if (!CmFile::FileExist(_S(runExeFile)))
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
std::shared_ptr<QApplication> CmFile::_app = nullptr;
bool CmFile::MkDir(CStr&  _path)
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
	return dir.mkdir(QString::fromLocal8Bit(_path.c_str()));
}

std::string CmFile::BrowseFolder(CStr& title)
{
	auto dir = QFileDialog::getExistingDirectory(nullptr, QString::fromLocal8Bit(title.c_str()));
	return (std::string)dir.toLocal8Bit();
}

std::string CmFile::BrowseFile(const char* strFilter, bool isOpen, const std::string& def_dir, CStr& title)
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
		qPath = QFileDialog::getOpenFileName(nullptr, QString::fromLocal8Bit(title.c_str()), QString::fromLocal8Bit(opened_dir.c_str()), QString::fromLocal8Bit(strFilter));
	else
		qPath = QFileDialog::getSaveFileName(nullptr, QString::fromLocal8Bit(title.c_str()), QString::fromLocal8Bit(opened_dir.c_str()), QString::fromLocal8Bit(strFilter));

	return (std::string)qPath.toLocal8Bit();
}

int CmFile::Rename(CStr& _srcNames, CStr& _dstDir, const char *nameCommon, const char *nameExt)
{
	vecS names;
	std::string inDir;
	int fNum = GetNames(_srcNames, names, inDir);
	for (int i = 0; i < fNum; i++) {
		std::string dstName = cv::format("%s\\%.4d%s.%s", _S(_dstDir), i, nameCommon, nameExt);
		std::string srcName = inDir + names[i];
		QFile file;
		file.rename(QString::fromLocal8Bit(srcName.c_str()), QString::fromLocal8Bit(dstName.c_str()));
	}
	return fNum;
}

void CmFile::RmFolder(CStr& dir)
{
	QDir tmp;
	tmp.rmdir(QString::fromLocal8Bit(dir.c_str()));
}

void CmFile::CleanFolder(CStr& dir, bool subFolder)
{
	vecS names;
	std::string tmp;
	int fNum = CmFile::GetNames(dir + "/*.*", names, tmp);
	for (int i = 0; i < fNum; i++)
		RmFile(dir + "/" + names[i]);

	vecS subFolders;
	int subNum = GetSubFolders(dir, subFolders);
	if (subFolder)
		for (int i = 0; i < subNum; i++)
			CleanFolder(dir + "/" + subFolders[i], true);
}

int CmFile::GetSubFolders(CStr& folder, vecS& subFolders)
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

// Get image names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
int CmFile::GetNames(CStr &nameW, vecS &names, std::string &dir)
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

int CmFile::GetNames(CStr& rootFolder, CStr &fileW, vecS &names)
{
	std::string dir;
	GetNames(rootFolder + fileW, names, dir);
	vecS subFolders, tmpNames;
	int subNum = CmFile::GetSubFolders(rootFolder, subFolders);
	for (int i = 0; i < subNum; i++) {
		subFolders[i] += "/";
		int subNum = GetNames(rootFolder + subFolders[i], fileW, tmpNames);
		for (int j = 0; j < subNum; j++)
			names.push_back(subFolders[i] + tmpNames[j]);
	}
	return (int)names.size();
}

int CmFile::GetNamesNE(CStr& nameWC, vecS &names, std::string &dir, std::string &ext)
{
	int fNum = GetNames(nameWC, names, dir);
	ext = GetExtention(nameWC);
	for (int i = 0; i < fNum; i++)
		names[i] = GetNameNE(names[i]);
	return fNum;
}

int CmFile::GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names)
{
	int fNum = GetNames(rootFolder, fileW, names);
	size_t extS = GetExtention(fileW).size();
	for (int i = 0; i < fNum; i++)
		names[i].resize(names[i].size() - extS);
	return fNum;
}

bool CmFile::Move2Dir(CStr &srcW, CStr dstDir)
{
	vecS names;
	std::string inDir;
	int fNum = CmFile::GetNames(srcW, names, inDir);
	bool r = true;
	for (int i = 0; i < fNum; i++)
		if (Move(inDir + names[i], dstDir + names[i]) == false)
			r = false;
	return r;
}

bool CmFile::Copy2Dir(CStr &srcW, CStr dstDir)
{
	vecS names;
	std::string inDir;
	int fNum = CmFile::GetNames(srcW, names, inDir);
	bool r = true;
	for (int i = 0; i < fNum; i++)
		if (Copy(inDir + names[i], dstDir + names[i]) == false)
			r = false;
	return r;
}

void CmFile::RunProgram(CStr &fileName, CStr &parameters, bool waiteF, bool showW)
{
	std::string runExeFile = fileName;
#ifdef _DEBUG
	runExeFile.insert(0, "..\\Debug\\");
#else
	runExeFile.insert(0, "..\\Release\\");
#endif // _DEBUG
	if (!CmFile::FileExist(_S(runExeFile)))
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
