/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: file.h
    Desc:     helpers for writing data into files and reading data from files
    Created:  28.06.2026 by DimaSkup
\**********************************************************************************/
#ifndef FILE_H
#define FILE_H

#include "log.h"
#include "vec.h"
#include <stdio.h>
#include <assert.h>


//*****************************************************************************
//                               READERS
//*****************************************************************************

//---------------------------------------------------------
// Desc:  read in a pair [key => string_value] from a file
// Args:  key - expected key of the field
// Out:   read string
//---------------------------------------------------------
inline void FileReadStr(FILE* fp, const char* key, char* outStr)
{
    assert(fp);
    assert(key && key[0] != '\0');
    assert(outStr);

    // skip symbols at the beginning until the key begins, and then read values
    char fmt[64];
    sprintf(fmt, "%%*[^%c]%s%%s", key[0], key);

    if (fscanf(fp, fmt, outStr) != 1)
        LogErr(LOG, "error reading a string (by key: \"%s\") from a file", key);
    fgetc(fp);
}

//---------------------------------------------------------
// Desc:  read in a pair [key => float_value] from a file
// Args:  key    - expected key of the field
// Ret:   read value
//---------------------------------------------------------
inline float FileReadFlt(FILE* fp, const char* key)
{
    assert(fp);
    assert(key && key[0] != '\0');

    char fmt[64];
    float outFlt = 0;
    
    // skip symbols at the beginning until the key begins, and then read values
    sprintf(fmt, "%%*[^%c]%s%%f", key[0], key);

    if (fscanf(fp, fmt, &outFlt) != 1)
        LogErr(LOG, "error reading a float (by key: \"%s\") from a file", key);
    fgetc(fp);

    return outFlt;
}

//---------------------------------------------------------
// Desc:  read in a pair [key => float float] from a file
// Args:  key    - expected key of the field
// Ret:   read pair of 2 floats (vec2) 
//---------------------------------------------------------
inline Vec2 FileReadFlt2(FILE* fp, const char* key)
{
    assert(fp);
    assert(key && key[0] != '\0');

    char fmt[64];
    Vec2 v = { 0,0 };

    // skip symbols at the beginning until the key begins, and then read values
    sprintf(fmt, "%%*[^%c]%s%%f %%f", key[0], key);

    if (fscanf(fp, fmt, &v.x, &v.y) != 2)
        LogErr(LOG, "error reading a Vec2 (by key: \"%s\") from a file", key);
    fgetc(fp);

    return v;
}

//---------------------------------------------------------
// Desc:  read in a pair [key => int_value] from a file
// Args:  level  - number of symbols to skip at the beginning of read line
//        key    - expected key of the field
// Ret:   read value
//---------------------------------------------------------
inline int FileReadInt(FILE* fp, const char* key)
{
    assert(fp);
    assert(key && key[0] != '\0');

    char fmt[64];
    int outInt = 0;

    // skip symbols at the beginning until the key begins, and then read values
    sprintf(fmt, "%%*[^%c]%s%%d", key[0], key);

    if (fscanf(fp, fmt, &outInt) != 1)
        LogErr(LOG, "error reading an int (by key: \"%s\") from a file", key);
    fgetc(fp);

    return outInt;
}

//---------------------------------------------------------
// Desc:  read in a pair [key => int int] from a file
// Args:  key    - expected key of the field
// Ret:   read pair of 2 integers (Vec2i) 
//---------------------------------------------------------
inline Vec2i FileReadInt2(FILE* fp, const char* key)
{
    assert(fp);
    assert(key && key[0] != '\0');

    char fmt[64];
    Vec2i v = { 0,0 };

    // skip symbols at the beginning until the key begins, and then read values
    sprintf(fmt, "%%*[^%c]%s%%d %%d", key[0], key);

    if (fscanf(fp, fmt, &v.x, &v.y) != 2)
        LogErr(LOG, "error reading a Vec2i (by key: \"%s\") from a file", key);
    fgetc(fp);

    return v;
}


//*****************************************************************************
//                              WRITERS
//*****************************************************************************

//---------------------------------------------------------
// push tabs into a file according to the level
//---------------------------------------------------------
inline void PrintOffset(FILE* fp, int level)
{
    assert(fp && level >= 0);

    while (level > 0)
    {
        fputc('\t', fp);
        --level;
    }
}


#endif