/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_font.cpp
    Desc:     font of a single type
    Created:  23.08.2026 by DimaSkup
\**********************************************************************************/
#include "ui_font.h"
#include "log.h"
#include <stdio.h>
#include <memory.h>
#include <assert.h>


//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
Font::Font() :
    fontTexId_(0),
    cellWidth_(0),
    cellHeight_(0),
    charsInRow_(0),
    charsInCol_(0),
    startChar_(0),
    fontHeight_(0)
{
    memset(chars_,        0, sizeof(chars_));
    memset(charsOffsets_, 0, sizeof(charsOffsets_));
    memset(charsWidth_,   0, sizeof(charsWidth_));
}

//---------------------------------------------------------
// initialize this font:  setup its texture and load its data from a file
//---------------------------------------------------------
void Font::Init(const TexID fontTexId, const char* fontDataFilepath)
{
    assert(fontTexId != INVALID_TEX_ID);
    assert(fontDataFilepath && fontDataFilepath[0] != '\0');

    ReadFontDataFromFile(fontDataFilepath);
    fontTexId_ = fontTexId;
}

//---------------------------------------------------------
// get width in pixels of input text for this particular font
//---------------------------------------------------------
int Font::GetTextWidth(const uint8* str, const size_t len)
{
    assert(str && str[0] != '\0');
    assert(len > 0);

    int width = 0;

    for (size_t i = 0; i < len; ++i)
        width += charsWidth_[str[i]];   // str[i] is an index of a character by itself

    return width;
}

//---------------------------------------------------------
// read in font's params from a file
//---------------------------------------------------------
void Font::ReadFontDataFromFile(const char* filepath)
{
    assert(filepath && filepath[0] != '\0');

    FILE* fp = fopen(filepath, "r");
    if (!fp)
        LogFatal(LOG, "%s%s", ERR_OPEN_FILE, filepath);

    int count = 0;
    int imageWidth = 0;
    int imageHeight = 0;
    int cellWidth = 0;
    int cellHeight = 0;
    int startChar = 0;
    int fontHeight = 0;
    int fontWidth = 0;    // (0 means default width of the character)

    //
    // read font params from a file
    //
    count = fscanf(fp, "Image Width,%d\n", &imageWidth);
    assert(count == 1);

    count = fscanf(fp, "Image Height,%d\n", &imageHeight);
    assert(count == 1);

    count = fscanf(fp, "Cell Width,%d\n", &cellWidth);
    assert(count == 1);

    count = fscanf(fp, "Cell Height,%d\n", &cellHeight);
    assert(count == 1);

    count = fscanf(fp, "Start Char,%d\n", &startChar);
    assert(count == 1);

    // skip a line with font name
    int c;
    while ((c = fgetc(fp)) != '\n');

    count = fscanf(fp, "Font Height,%d\n", &fontHeight);
    assert(count == 1);

    count = fscanf(fp, "Font Width (0 is default),%d\n", &fontWidth);
    assert(count == 1);

    //
    // check params
    //
    assert(imageWidth > 0);
    assert(imageHeight > 0);
    assert(cellWidth > 0 && cellWidth <= UINT8_MAX);
    assert(cellHeight > 0 && cellHeight <= UINT8_MAX);
    assert(startChar > 0 && startChar <= UINT8_MAX);
    assert(fontHeight > 0 && fontHeight <= UINT8_MAX);
    assert(fontWidth >= 0);

    //
    // store params
    //
    cellWidth_ = cellWidth;
    cellHeight_ = cellHeight;
    charsInRow_ = imageWidth / cellWidth;
    charsInCol_ = imageHeight / cellHeight;
    startChar_ = startChar;
    fontHeight_ = fontHeight;

    //
    // calc properties of each character which will be used for rendering the font
    //
    const int numChars = charsInRow_ * charsInCol_;
    assert(numChars <= MAX_CHARS);

    memset(chars_,        0, sizeof(chars_));
    memset(charsOffsets_, 0, sizeof(charsOffsets_));
    memset(charsWidth_,   0, sizeof(charsWidth_));


    // fill in chars array with actual chars
    for (int i = 0; i < MAX_CHARS; ++i)
        chars_[i] = (wchar_t)(startChar + i);

    // calc offset (in pixels) of each character on the font texture
    for (int x = 0; x < charsInRow_; ++x)
    {
        for (int y = 0; y < charsInCol_; ++y)
        {
            TexOffset& off = charsOffsets_[charsInCol_ * y + x];
            off.x = cellWidth_ * x;
            off.y = cellHeight_ * y;
        }
    }

    // read in width of each character
    for (int i = 0; i < MAX_CHARS; ++i)
    {
        int charIdx = -1;
        int baseWidth = -1;

        count = fscanf(fp, "Char %d Base Width,%d\n", &charIdx, &baseWidth);
        assert(count == 2);
        assert(charIdx == i);
        assert(baseWidth > 0 && baseWidth < UINT8_MAX);

        charsWidth_[i] = baseWidth;
    }

    fclose(fp);
}