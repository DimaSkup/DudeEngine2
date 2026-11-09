/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_font.h
    Desc:     font of a single type
    Created:  23.08.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_FONT_H
#define UI_FONT_H

#include "types.h"
#include "rect.h"

class Font
{
private:
    struct TexOffset
    {
        uint16 x;
        uint16 y;
    };

public:
    Font();

    void Init(const TexID fontTexId, const char* fontDataFilepath);

    TexID GetFontTexId() const;
    int  GetFontHeight() const;
    uint8 GetStartChar() const;
    uint8 GetCellHeight() const;

    uint8 GetCharWidth(const uint8 ch) const;

    Recti GetCharTexCoords(const uint8 ch) const;

    int  GetTextWidth(const uint8* text, const size_t len);

private:
    void ReadFontDataFromFile(const char* fontDataFilepath);

private:
    static const int MAX_CHARS = 256;
    wchar_t          chars_[MAX_CHARS];
    TexOffset        charsOffsets_[MAX_CHARS];
    uint8            charsWidth_[MAX_CHARS];    // each character may have its own width

    TexID            fontTexId_;                // ID of a font texture atlas
                                                
    uint8            cellWidth_;                // cell's width of a single char 
    uint8            cellHeight_;               // cell's height of a single char
                                                
    uint8            charsInRow_;               // on the font texture
    uint8            charsInCol_;               // on the font texture
    uint8            startChar_;                // on the font texture we start with this character
    uint8            fontHeight_;
};


//*****************************************************************************
// INLINE FUNCTIONS
//*****************************************************************************

inline TexID Font::GetFontTexId() const
{
    return fontTexId_;
}

inline int Font::GetFontHeight() const
{
    return fontHeight_;
}

inline uint8 Font::GetStartChar() const
{
    return startChar_;
}

inline uint8 Font::GetCellHeight() const
{
    return cellHeight_;
}

inline uint8 Font::GetCharWidth(const uint8 ch) const
{
    return charsWidth_[ch];
}

//---------------------------------------------------------
// return char's edges in pixels (on texture): left, right, top, bottom
//---------------------------------------------------------
inline Recti Font::GetCharTexCoords(const uint8 ch) const
{
    const TexOffset& off = charsOffsets_[ch - startChar_];
    return Recti{ off.x, off.x + charsWidth_[ch], off.y, off.y + cellHeight_ };
}


#endif