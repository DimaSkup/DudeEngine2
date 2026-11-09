/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   file_sys.cpp
    Desc:       implementation of utils for working with files paths
    Created:    by DimaSkup
\**********************************************************************************/
#include "file_sys.h"
#include "log.h"
#include "consts.h"
#include <stdio.h>        // for using FILE
#include <ctype.h>        // for using isalpha, isdigit
#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include <errno.h>
#include <assert.h>
#include "include/tinydir/tinydir.h"

#ifdef _WIN32
#include <direct.h>
#define make_dir(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define make_dir(path) mkdir(path, 0777) // 0777 sets read/write/execute permissions
#endif


//-----------------------------------------------------
// a static pointer to the currently opened directory
//-----------------------------------------------------
static tinydir_dir* s_pDir = nullptr;

//-----------------------------------------------------
// Desc:  convert input char string into wide string
// Args:  - str:      input source string of chars
//        - outWStr:  output string of wide chars
//-----------------------------------------------------
inline static void StrToWide(const char* str, wchar_t* outWStr)
{
    if (outWStr == nullptr)
    {
        LogErr(LOG, "in-out wide string == NULL: so we cannot convert str => wstr");
        return;
    }

    if (!str || str[0] == '\0')
    {
        LogErr(LOG, "empty src string");
        outWStr[0] = L'\0';
        return;
    }

    const size_t sz = strlen(str) + 1;
    mbstowcs(outWStr, str, sz);
}

//-----------------------------------------------------
// Desc:  convert input wide string into char string
// Args:  - wstr:    input source string of wide chars
//        - outStr:  output string of chars
//-----------------------------------------------------
static void ToStr(const wchar_t* wstr, char* outStr)
{
    if (outStr == nullptr)
    {
        LogErr(LOG, "in-out string == NULL: so we cannot convert wstr => str");
        return;
    }

    if (!wstr || wstr[0] == L'\0')
    {
        outStr[0] = '\0';
        return;
    }

    // calculating the length of the multibyte string
    size_t len = wcstombs(nullptr, wstr, 0) + 1;

    if (len == 0)
    {
        outStr[0] = '\0';
        return;
    }

    // convert wstring to string
    size_t sz = wcstombs(outStr, wstr, len);

    // if we got some err
    if (sz == (size_t)-1)
    {
        LogErr(LOG, "failed convert wstr => str");
        outStr[0] = '\0';
        return;
    }
}

//-----------------------------------------------------
// check if file by filePath (relatively to the working directory) exists
//-----------------------------------------------------
bool FileSys::Exists(const char* filePath)
{
    if (IsEmptyPath(filePath))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return false;
    }

    FILE* pFile = nullptr;

    // check if such file exists
    if ((pFile = fopen(filePath, "r+")) == nullptr)
        return false;

    fclose(pFile);
    return true;
}

//-----------------------------------------------------
// return relative path from the project root
//-----------------------------------------------------
void FileSys::GetPathFromProjRoot(const char* fullPath, char* outPath)
{
    if ((!fullPath) || (fullPath[0] == '\0'))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return;
    }

    if (!outPath)
    {
        LogErr(LOG, "in-out path == NULL");
        return;
    }

    const char* found = strstr(fullPath, ENGINE_NAME);

    // if we found the substring we copy all the text after "EngineName"
    if (found != nullptr)
        strcpy(outPath, found + strlen(ENGINE_NAME));

    else
        outPath[0] = '\0';
}

//-----------------------------------------------------
// Desc:   check if engine supports this filepath
//         (path can contain only: alpha symbols, digit, \\, /, -, _, . (dot) )
//-----------------------------------------------------
bool FileSys::IsPathSupported(const char* path)
{
    if (IsEmptyPath(path))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return false;
    }

    for (int i = 0; i < path[i] != '\0'; ++i)
    {
        bool isValid = false;

        isValid |= (bool)isalpha(path[i]);
        isValid |= (bool)isdigit(path[i]);
        isValid |= (path[i] == '\\');
        isValid |= (path[i] == '/');
        isValid |= (path[i] == '_');
        isValid |= (path[i] == '-');
        isValid |= (path[i] == '.');
        isValid |= (path[i] == ':');

        if (!isValid)
        {
            LogErr(LOG, "path isn't supported: %s\n (path can contain only: alpha symbols, digit, \\, /, and _)", path);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------
// Desc:   check if engine supports this filepath
//         (path can contain only: alpha symbols, digit, \\, /, -, _, . (dot) )
//-----------------------------------------------------
bool FileSys::IsPathSupported(const wchar_t* path)
{
    if (IsEmptyPath(path))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return false;
    }

    for (int i = 0; i < path[i] != '\0'; ++i)
    {
        bool isValid = false;

        isValid |= (bool)isalpha(path[i]);
        isValid |= (bool)isdigit(path[i]);
        isValid |= (path[i] == L'\\');
        isValid |= (path[i] == L'/');
        isValid |= (path[i] == L'_');
        isValid |= (path[i] == L'-');
        isValid |= (path[i] == L'.');
        isValid |= (path[i] == L':');

        if (!isValid)
        {
            LogErr(LOG, "path isn't supported: %ls\n (path can contain only: alpha symbols, digit, \\, /, and _)", path);
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------
// Desc:   get position of a last slash in the input path
//-----------------------------------------------------
int FileSys::GetLastSlashOffset(const char* path)
{
    if (IsEmptyPath(path))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return -1;
    }

    const char* offset1 = strrchr(path, '\\');
    const char* offset2 = strrchr(path, '/');

    // if no slash or backslash
    if ((offset1 == NULL) && (offset2 == NULL))
        return 0;


    ptrdiff_t offset = 0;

    if (offset1 != NULL)
    {
        const ptrdiff_t off = offset1 - path;

        if (offset < off)
            offset = off;
    }

    if (offset2 != NULL)
    {
        const ptrdiff_t off = offset2 - path;
        if (offset < off)
            offset = off;
    }

    return (int)offset;
}

//-----------------------------------------------------
// Desc:   get parent path by input path
//-----------------------------------------------------
void FileSys::GetParentPath(const char* path, char* outPath)
{
    const char* lastSlash = path + GetLastSlashOffset(path);
    const ptrdiff_t pathLen = lastSlash - path + 1;

    strncpy(outPath, path, pathLen);
    outPath[pathLen] = '\0';                           // put extra NULL
}

//-----------------------------------------------------
// returns the filename by input path
//-----------------------------------------------------
void FileSys::GetFileName(const char* path, char* outFilename)
{
    const ptrdiff_t offset = GetLastSlashOffset(path);
    strcpy(outFilename, path + offset + 1);
    //outFilename[offset] = '\0';              // put extra NULL
}

//-----------------------------------------------------
// create a new directory by input path
//-----------------------------------------------------
bool FileSys::CreateDir(const char* dirPath)
{
    if (IsEmptyPath(dirPath))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return false;
    }

    if (make_dir(dirPath) != 0)
    {
        if (errno == EEXIST)
            LogErr(LOG, "directory already exists: %s", dirPath);

        else
            LogErr(LOG, "can't create a directory: %s", dirPath);

        return false;
    }

    LogMsg(LOG, "directory is created successfully: %s", dirPath);
    return true;
}

//-----------------------------------------------------
// copy a file by filePath into a new location
//-----------------------------------------------------
void FileSys::CopyFileInto(const char* srcPath, const char* dstPath)
{
    // check input args
    if (IsEmptyPath(srcPath))
    {
        LogErr(LOG, "empty src filepath");
        return;
    }

    if (IsEmptyPath(dstPath))
    {
        LogErr(LOG, "empty dst filepath");
        return;
    }

    //---------------------------------

    FILE* fpSrc = NULL;
    FILE* fpDst = NULL;
    uint8_t buf[4096];
    size_t bytesRead = 0;

    // check if dst file doesn't already exists
    if ((fpDst = fopen(dstPath, "r+")) != NULL)
    {
        LogErr(LOG, "dst file already exists: %s", dstPath);
        return;
    }

    fpSrc = fopen(srcPath, "rb");
    if (!fpSrc)
    {
        LogErr(LOG, "can't open a source file: %s", srcPath);
        goto copy_end;
    }

    fpDst = fopen(dstPath, "wb");
    if (!fpDst)
    {
        LogErr(LOG, "can't create a destination file: %s", dstPath);
        goto copy_end;
    }

    // read from source file and write into destination file using the buffer
    while ((bytesRead = fread(buf, 1, sizeof(buf), fpSrc)) > 0)
    {
        size_t bytesWritten = fwrite(buf, 1, bytesRead, fpDst);

        if (bytesWritten < bytesRead)
        {
            LogErr(LOG, "error writing data into dst file: %s", dstPath);
            goto copy_end;
        }
    }

    LogMsg("File (%s) copied into (%s)\n", srcPath, dstPath);

copy_end:
    if (fpSrc)  fclose(fpSrc);
    if (fpDst)  fclose(fpDst);
}

//-----------------------------------------------------
// returns the stem path component (filename without the final extension)
//-----------------------------------------------------
void FileSys::GetFileStem(const char* path, char* outStem)
{
    // define where stem starts (what is the offset)
    const ptrdiff_t offset = GetLastSlashOffset(path);
    const char* stemStart = path + offset + 1;

    // define where is the dot
    const char* dotPos = strrchr(path, '.');
    ptrdiff_t stemLen = 0;

    if (dotPos)
        stemLen = dotPos - stemStart;
    else
        stemLen = strlen(path) - offset;

    // make the output string
    strncpy(outStem, stemStart, stemLen);
    outStem[stemLen] = '\0';                           // put extra NULL
}

//-----------------------------------------------------
// returns the file extension path component
//-----------------------------------------------------
void FileSys::GetFileExt(const char* path, char* outExt)
{
    if (IsEmptyPath(path))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return;
    }

    const char* dotPos = strrchr(path, '.');

    if (dotPos)
    {
        strcpy(outExt, dotPos);
        outExt[strlen(dotPos)] = '\0';                     // put extra NULL
    }
    else
    {
        strcpy(outExt, "invalid");
    }
}

//-----------------------------------------------------
// get the directory where the application was run from.
//-----------------------------------------------------
char* FileSys::GetBasePath()
{
    return SDL_GetBasePath();
}

//-----------------------------------------------------
// convert all the '\\' symbols in the path into '/' symbols
//-----------------------------------------------------
void FileSys::ToForwardSlash(char* outRelPath)
{
    if (!outRelPath || outRelPath[0] == '\0')
        return;

    for (size_t i = 0; i < strlen(outRelPath); ++i)
    {
        if (outRelPath[i] == '\\')
            outRelPath[i] = '/';
    }
}

//-----------------------------------------------------
// convert all the '/' symbols in the path into '\\' symbols 
//-----------------------------------------------------
void FileSys::ToBackSlash(char* outPath)
{
    if (!outPath || outPath[0] == '\0')
        return;

    for (size_t i = 0; i < strlen(outPath); ++i)
    {
        if (outPath[i] == '/')
            outPath[i] = '\\';
    }
}

//-----------------------------------------------------
// Desc:  get a path relatively to the current working dir (where the app was run from)
// Args:  fullPath - path from OS sys root
// Out:   relative path
//-----------------------------------------------------
void FileSys::GetRelPath(const char* fullPath, char* outRelPath)
{
    if (IsEmptyPath(fullPath))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return;
    }

    const char* basePath = FileSys::GetBasePath();
    const size_t len = strlen(basePath);

    bool bPathStartFromSysRoot = (strncmp(fullPath, basePath, len) == 0);
    if (!bPathStartFromSysRoot)
        return;

    strcpy(outRelPath, fullPath + len);
    ToForwardSlash(outRelPath);
}

//-----------------------------------------------------
// open a directory by input path
//-----------------------------------------------------
bool FileSys::OpenDir(const char* parentDirPath)
{
    if (s_pDir != nullptr)
    {
        LogErr(LOG, "DUDE! you forget to close the previous dir!");
        CloseDir();
    }

    if (IsEmptyPath(parentDirPath))
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return false;
    }

    wchar_t wParentDirPath[512];
    if (strlen(parentDirPath) > 512)
    {
        LogErr(LOG, "input path is too long");
        return false;
    }

    assert(s_pDir == nullptr);
    s_pDir = new tinydir_dir;

    // open a directory
    StrToWide(parentDirPath, wParentDirPath);

    if (tinydir_open(s_pDir, wParentDirPath) != 0)
    {
        LogErr(LOG, "can't open directory: %s", parentDirPath);
        delete s_pDir;
        s_pDir = nullptr;
        return false;
    }

    return true;
}

//-----------------------------------------------------
// check if there is a directory/folder by input path
//-----------------------------------------------------
bool FileSys::IsDirExist(const char* dirPath)
{
    // if we can't open a dir then there is no such directory
    if (!OpenDir(dirPath))
        return false;

    CloseDir();
    return true;
}

//-----------------------------------------------------
// close currently opened directory
//-----------------------------------------------------
void FileSys::CloseDir()
{
    if (s_pDir == nullptr)
        return;

    tinydir_close(s_pDir);
    delete s_pDir;
    s_pDir = nullptr;
}

//-----------------------------------------------------
// Desc:  get next child directory name from the currently opened parent directory
// Ret:   false - if we finished reading of the child directories
//-----------------------------------------------------
bool FileSys::GetNextDirName(char* outDirName)
{
    if (!s_pDir->has_next)
        return false;

    tinydir_file file;
    tinydir_readfile(s_pDir, &file);

    if (file.is_dir && file.name[0] != L'.')
    {
        char dirName[64];
        ToStr(file.name, dirName);

        strcpy(outDirName, dirName);
        outDirName[strlen(dirName)] = '\0';
    }

    tinydir_next(s_pDir);
    return true;
}