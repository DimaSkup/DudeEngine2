/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   file_sys.h
    Desc:       utils for working with files paths
    Created:    DimaSkup
\**********************************************************************************/
#ifndef FILE_SYS_H
#define FILE_SYS_H

class FileSys
{
public:

    static bool Exists(const char* filePath);
    static void GetPathFromProjRoot(const char* fullPath, char* outPath);

    static bool CreateDir(const char* dirPath);
    static void CopyFileInto(const char* srcPath, const char* dstPath);

    static bool IsPathSupported(const char* path);
    static bool IsPathSupported(const wchar_t* path);

    static int GetLastSlashOffset(const char* path);

    static void GetParentPath(const char* path, char* outPath);
    static void GetFileName  (const char* path, char* outFilename);
    static void GetFileStem  (const char* path, char* outStem);
    static void GetFileExt   (const char* path, char* outExt);

    static char* GetBasePath();
    static void GetRelPath(const char* fullPath, char* outRelPath);

    static void ToForwardSlash(char* outRelPath);
    static void ToBackSlash(char* outPath);

    // work with directories
    static bool OpenDir(const char* parentDirPath);
    static void CloseDir();
    static bool IsDirExist(const char* dirPath);
    static bool GetNextDirName(char* outDirName);

private:
    static bool IsEmptyPath(const char* str);
    static bool IsEmptyPath(const wchar_t* wstr);
};

//==================================================================================
// inline functions
//==================================================================================

inline bool FileSys::IsEmptyPath(const char* str)
{
    return ((str == nullptr) || (str[0] == '\0'));
}

inline bool FileSys::IsEmptyPath(const wchar_t* wstr)
{
    return ((wstr == nullptr) || (wstr[0] == L'\0'));
}

#endif