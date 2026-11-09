/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_text_mgr.h
    Desc:     functional for text rendering

              during the frame we accumulate text into the buffer, so when 
              we call Render() we draw it all at once and then flush the buffer

    Created:  05.07.2026 by DimaSkup
\**********************************************************************************/
#ifndef UI_TEXT_MGR
#define UI_TEXT_MGR

#include "types.h"
#include "cvector.h"
#include "ui_font.h"
#include <unordered_map>
#include <string>


//---------------------------------------------------------
// flags for manipulations with text
//---------------------------------------------------------
enum eTextFlags
{
    TEXT_CENTER_X = (1 << 0),    // centered horizontally
    TEXT_CENTER_Y = (1 << 1),    // centered vertically
    TEXT_SHADOWED = (1 << 2),
    TEXT_FADED    = (1 << 3),

    NUM_TEXT_FLAGS = 4,
    //TEXT_ALIGN_LEFT = ,    // aligned to the left edge of the screen
    //TEXT_ALIGN_RIGHT = ,
};

//---------------------------------------------------------
// which kinds of fonts we may have
//---------------------------------------------------------
enum eFontType : uint8
{
    FONT_TYPE_GAME = 0,
    FONT_TYPE_DEBUG,
    FONT_TYPE_DIALOG,
    NUM_FONT_TYPES
};

//---------------------------------------------------------
// class name:  TextMgr
//---------------------------------------------------------
class TextMgr
{
private:

    struct TextData
    {
        std::wstring text;
        int textPX;
        int textPY;
        Color textColor;
        uint16 textFlags;
        eFontType font;
    };

    struct TextShadowedData
    {
        std::wstring text;
        int textPX;
        int textPY;
        Color textColor;
        Color shadowColor;
        uint16 textFlags;
        int8 shadowOffX;
        int8 shadowOffY;
        eFontType font;
    };

    struct TextFadedData
    {
        std::wstring text;
        float  lifeTime;     // full lifetime (in seconds) of this text before complete fade
        float  currTime;     // remain time before complete fade
        int    textPX;
        int    textPY;
        Color  textColor;
        Color  shadowColor;
        uint16 textFlags; 
        int8   shadowOffX;
        int8   shadowOffY;
        eFontType font;
    };

public:
    TextMgr();

    void InitFont       (const eFontType type,
                         const TexID fontTexId,
                         const char* fontDataFilepath);

    void SetFontType    (const eFontType type);
    bool SetLocale      (const char* locale);

    int  GetFontHeight  (const eFontType type) const;
    int  GetCharWidth   (const eFontType type, const char ch);

    void Update         (const float dt);
    void Render         ();

    void ClearFadedTexts(void);
    void SetFadeSec     (const float sec);

    void SetPos         (const int x, const int y);
    void PushFlags      (const uint16 mask);
    void PopFlags       (const uint16 mask);
    void FlushFlags     (void);

    void SetTextColor   (uint8 r, uint8 g, uint8 b, uint8 a);
    void SetTextColor   (const Color c);

    void SetShadowOffset(const int x, const int y);
    void SetShadowColor (uint8 r, uint8 g, uint8 b, uint8 a);
    void SetShadowColor (const Color c);

    void Draw           (const char* str);
    void Draw           (const wchar_t* fmt, ...);

    const char** GetAvailLocales()    const;
    const size_t GetNumAvailLocales() const;
    const char*  GetCurrLocale()      const;
 
private:
    
    void DrawText        (const char* str);
    void DrawText        (const wchar_t* str);
    void DrawTextShadowed(const wchar_t* str);

    void ModifyTextPosAccordingToFlags(const uint8* text,
                                       const size_t textLen,
                                       const uint16 flags,
                                       int& top,
                                       int& left) const;

private:
    Font        fonts_[NUM_FONT_TYPES];
    Font*       pFont_;
    eFontType   currFont_;

    // buffers for text accumulation
    cvector<TextData>         textsBuf_;
    cvector<TextShadowedData> textsShadowedBuf_;
    cvector<TextFadedData>    textsFadedBuf_;    // THOSE DON'T FLUSH AFTER RENDERING; array of texts which dissapears during the time for some amount of seconds


    Color       textColor_;
    Color       shadowColor_;
    int         x_, y_;                 // top left corner position of the sentence
    int8_t      shadowOffX_;            // text shadow offset by X and Y
    int8_t      shadowOffY_;

    uint16      textFlags_;             // a bitmask with text positioning flags (look at the TextPosFlags enumeration)
                                        // (you need to manually push/pop desired flags before and after rendering the text)

    char        locale_[4];             // current locale ("en" / "ru" / "ua")

    float       fadeForSec_;            // text's fading duration

    std::unordered_map<std::string, std::wstring> keyToText_;
};

//---------------------------------------------------------
// a global instance of the text manager (I don't give a fuck)
//---------------------------------------------------------
extern TextMgr g_Text;


//*****************************************************************************
// INLINE FUNCTIONS
//*****************************************************************************

//---------------------------------------------------------
// get height of a font by input type
//---------------------------------------------------------
inline int TextMgr::GetFontHeight(const eFontType type) const
{
    assert(type >= 0 && type < NUM_FONT_TYPES);
    return fonts_[type].GetFontHeight();
}

//---------------------------------------------------------
// get width of input char related to the font type
//---------------------------------------------------------
inline int TextMgr::GetCharWidth(const eFontType type, const char ch)
{
    assert(type >= 0 && type < NUM_FONT_TYPES);
    return fonts_[type].GetCharWidth(ch);
}

//---------------------------------------------------------
// setup the current font 
// (all the draw calls will use this font until we set it to some another)
//---------------------------------------------------------
inline void TextMgr::SetFontType(const eFontType type)
{
    assert(type >= 0 && type < NUM_FONT_TYPES);
    currFont_ = type;
}

#endif