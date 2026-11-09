/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename: ui_text_mgr.cpp
    Desc:     implementation of functional for text rendering
    Created:  05.07.2026 by DimaSkup
\**********************************************************************************/
#include "ui_text_mgr.h"
#include "log.h"
#include "asset_mgr.h"
#include "r_main.h"
#include "macros.h"
#include "file.h"
#include <wchar.h>
#include <locale.h>


//---------------------------------------------------------
// a global instance of the text manager (I don't give a fuck)
//---------------------------------------------------------
TextMgr g_Text;


//*****************************************************************************
// helpers for characters encoding convertation
//*****************************************************************************

//---------------------------------------------------------
// Desc:  convert UTF-16 character into Windows-1251 character
//---------------------------------------------------------
uint8 CharUtf16ToWin1251(const wchar_t src)
{
    // character set (0-127)
    if (src < 128)
        return (char)src;

    // character set (128-255)
    switch (src)
    {
        // cyrillic upper case
        case 0x0410:    return 0xC0;
        case 0x0411:    return 0xC1;
        case 0x0412:    return 0xC2;
        case 0x0413:    return 0xC3;
        case 0x0414:    return 0xC4;
        case 0x0415:    return 0xC5;
        case 0x0416:    return 0xC6;
        case 0x0417:    return 0xC7;
        case 0x0418:    return 0xC8;
        case 0x0419:    return 0xC9;
        case 0x041A:    return 0xCA;
        case 0x041B:    return 0xCB;
        case 0x041C:    return 0xCC;
        case 0x041D:    return 0xCD;
        case 0x041E:    return 0xCE;
        case 0x041F:    return 0xCF;

        case 0x0420:    return 0xD0;
        case 0x0421:    return 0xD1;
        case 0x0422:    return 0xD2;
        case 0x0423:    return 0xD3;
        case 0x0424:    return 0xD4;
        case 0x0425:    return 0xD5;
        case 0x0426:    return 0xD6;
        case 0x0427:    return 0xD7;
        case 0x0428:    return 0xD8;
        case 0x0429:    return 0xD9;
        case 0x042A:    return 0xDA;
        case 0x042B:    return 0x9B;
        case 0x042C:    return 0xDC;
        case 0x042D:    return 0xDD;
        case 0x042E:    return 0xDE;
        case 0x042F:    return 0xDF;

        // cyrillic lower case
        case 0x0430:    return 0xE0;
        case 0x0431:    return 0xE1;
        case 0x0432:    return 0xE2;
        case 0x0433:    return 0xE3;
        case 0x0434:    return 0xE4;
        case 0x0435:    return 0xE5;
        case 0x0436:    return 0xE6;
        case 0x0437:    return 0xE7;
        case 0x0438:    return 0xE8;
        case 0x0439:    return 0xE9;
        case 0x043A:    return 0xEA;
        case 0x043B:    return 0xEB;
        case 0x043C:    return 0xEC;
        case 0x043D:    return 0xED;
        case 0x043E:    return 0xEE;
        case 0x043F:    return 0xEF;

        case 0x0440:    return 0xF0;
        case 0x0441:    return 0xF1;
        case 0x0442:    return 0xF2;
        case 0x0443:    return 0xF3;
        case 0x0444:    return 0xF4;
        case 0x0445:    return 0xF5;
        case 0x0446:    return 0xF6;
        case 0x0447:    return 0xF7;
        case 0x0448:    return 0xF8;
        case 0x0449:    return 0xF9;
        case 0x044A:    return 0xFA;
        case 0x044B:    return 0xFB;
        case 0x044C:    return 0xFC;
        case 0x044D:    return 0xFD;
        case 0x044E:    return 0xFE;
        case 0x044F:    return 0xFF;

        // cyrillic IO (upper/lower)
        case 0x0401:    return 0xA8;
        case 0x0451:    return 0xB8;

        // cyrillic ukrainian IE (upper/lower)
        case 0x0404:    return 0xAA;
        case 0x0454:    return 0xBA;

        // cyrillic YI (upper/lower)
        case 0x0407:    return 0xAF;
        case 0x0457:    return 0xBF;

        // cyrillic letter Byelorussian-Ukrainian I
        case 0x0406:    return 0xB2;
        case 0x0456:    return 0xB3;
    }

    LogErr(LOG, "can't convert a symbol (%C) from utf-16 into Windows-1251", src);
    
    // space
    return 0x20;
}

//---------------------------------------------------------
// Desc:  convert UTF-16 string into Windows-1251 string
// Args:  src    - original string
//        dst    - output converted string
//        maxLen - limit for dst
//---------------------------------------------------------
void Utf16ToWin1251(const wchar_t* src, uint8* dst, size_t maxLen)
{
    assert(src && src[0] != L'\0');
    assert(dst);
    assert(maxLen > 0 && maxLen > wcslen(src));

    const size_t len = wcslen(src);

    for (size_t i = 0; i < len; ++i)
        dst[i] = CharUtf16ToWin1251(src[i]);

    dst[len] = '\0';
}


//*****************************************************************************
//                           PUBLIC METHODS
//*****************************************************************************

//---------------------------------------------------------
// default constructor
//---------------------------------------------------------
TextMgr::TextMgr() :
    pFont_(nullptr),
    x_(0),
    y_(0),
    shadowOffX_(0),
    shadowOffY_(0),
    textFlags_(0),
    fadeForSec_(0.0f)
{
    memset(fonts_, 0, sizeof(fonts_));
    memset(locale_, 0, sizeof(locale_));

    textColor_.rgba = 0;
    shadowColor_.rgba = 0;
}

//---------------------------------------------------------
// init a font of a single type (we may have different fonts for different purposes)
//---------------------------------------------------------
void TextMgr::InitFont(
    const eFontType fontType, 
    const TexID fontTexId,
    const char* fontDataFilepath)
{
    assert(fontType >= 0 && fontType < NUM_FONT_TYPES);
    fonts_[fontType].Init(fontTexId, fontDataFilepath);
}

//---------------------------------------------------------
// update texts data (for instance: remained time for faded text)
//---------------------------------------------------------
void TextMgr::Update(const float dt)
{
    for (index i = 0; i < textsFadedBuf_.size(); ++i)
    {
        textsFadedBuf_[i].currTime -= dt;

        if (textsFadedBuf_[i].currTime <= 0.0f)
        {
            // swap & pop
            std::swap(textsFadedBuf_[i], textsFadedBuf_.back());
            textsFadedBuf_.pop_back();
        }
    }
}

//---------------------------------------------------------
// render some specific text onto the screen (for instance: faded text) 
//---------------------------------------------------------
void TextMgr::Render()
{
    //
    // PLAIN TEXT
    //
    for (const TextData& data : textsBuf_)
    {
        // setup text manager for drawing the next text
        SetPos(data.textPX, data.textPY);
        SetTextColor(data.textColor);

        // apply text modifiers
        PushFlags(data.textFlags);
        currFont_ = data.font;

        DrawText(data.text.c_str());

        PopFlags(data.textFlags);
    }
    textsBuf_.clear();
    
    //
    // SHADOWED TEXT
    //
    for (const TextShadowedData& data : textsShadowedBuf_)
    {
        // setup text manager for drawing the next text
        SetPos(data.textPX, data.textPY);
        SetTextColor(data.textColor);

        SetShadowColor(data.shadowColor);
        SetShadowOffset(data.shadowOffX, data.shadowOffY);

        // apply text modifiers
        PushFlags(data.textFlags);
        currFont_ = data.font;

        DrawTextShadowed(data.text.c_str());

        PopFlags(data.textFlags);
    }
    textsShadowedBuf_.clear();
    
    //
    // FADED TEXT
    //
    for (const TextFadedData& data : textsFadedBuf_)
    {
        // setup text manager for drawing the next text
        SetPos(data.textPX, data.textPY);
        SetTextColor(data.textColor);

        const uint8 alpha = (uint8)(255 * (data.currTime / data.lifeTime));
        textColor_.a = alpha;

        // apply text modifiers
        PushFlags(data.textFlags);
        currFont_ = data.font;

        // render text according to its type
        if (data.textFlags & TEXT_SHADOWED)
        {
            SetShadowColor(data.shadowColor);
            shadowColor_.a = alpha;

            SetShadowOffset(data.shadowOffX, data.shadowOffY);
            DrawTextShadowed(data.text.c_str());
        }
        else
        {
            DrawText(data.text.c_str());
        }

        PopFlags(data.textFlags);
    }
}

//---------------------------------------------------------
// clear the list of faded texts (force removing of all of them)
//---------------------------------------------------------
void TextMgr::ClearFadedTexts()
{
    textsFadedBuf_.clear();
}

//---------------------------------------------------------
// load all the text for input locale (or reset to english if failed)
//---------------------------------------------------------
bool TextMgr::SetLocale(const char* locale)
{
    assert(locale && locale[0] != '\0');

    // try to open a file with this localization
    char path[64];
    snprintf(path, sizeof(path), "assets/fonts/locales/%s.txt", locale);

    // set UTF-8 for proper reading of cyrillic chars from the file
    setlocale(LC_ALL, "en_US.UTF-8");

    FILE* fp = fopen(path, "r");
    if (!fp)
    {
        LogErr(LOG, "%s%s", ERR_OPEN_FILE, path);
        return false;
    }

    // store the current locale
    snprintf(locale_, sizeof(4), "%s", locale);
 
    //
    // read in all the pairs [key => "localized_str"]
    //
    wchar_t buf[128];
    char key[64];
    wchar_t value[64];

    // clear the previous locale
    keyToText_.clear();

    while (fgetws(buf, 128, fp))
    {
        if (buf[0] == L'{' || buf[0] == L'}')
            continue;

        // 1. skip tabulation
        // 2. read key
        // 3. skip quote (")
        // 4. read value (read all chars until quote ("))
        int count = swscanf(buf, L"\t%S \"%[^\"]", key, value);
        if (count != 2)
        {
            LogErr(LOG, "can't parse a string:\n %S", buf);
            continue;
        }

        // store the pair
        keyToText_.insert({ key, value });
    }

    fclose(fp);
    return true;
}

//---------------------------------------------------------
// setup position (top left corner) for the next sentence
//---------------------------------------------------------
void TextMgr::SetPos(const int x, const int y)
{
    x_ = x;
    y_ = y;
}

//---------------------------------------------------------
// setup a bitmask with text flags (look at the TextFlags enumeration)
//---------------------------------------------------------
void TextMgr::PushFlags(const uint16 mask) 
{
    textFlags_ |= mask;
}

void TextMgr::PopFlags(const uint16 mask)
{
    textFlags_ &= ~(mask);
}

//---------------------------------------------------------
// reset the bitmask with text flags (no text modifiers will be used)
//---------------------------------------------------------
void TextMgr::FlushFlags()
{
    textFlags_ = 0;
}

//---------------------------------------------------------
// setup text fading time
//---------------------------------------------------------
void TextMgr::SetFadeSec(const float sec)
{
    if (sec < 0.0f)
    {
        LogErr(LOG, "seconds count can't be negative! (%f)", sec);
        return;
    }

    fadeForSec_ = sec;
}

//---------------------------------------------------------
// setup offset of the shadow by X and Y relatively to the text position
//---------------------------------------------------------
void TextMgr::SetShadowOffset(const int x, const int y)
{
    shadowOffX_ = x;
    shadowOffY_ = y;
}

//---------------------------------------------------------
// setup foreground (symbols) color
//---------------------------------------------------------
void TextMgr::SetTextColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
    textColor_.rgba = RGBATOU32(r,g,b,a);
}

void TextMgr::SetTextColor(const Color c)
{
    textColor_ = c;
}

//---------------------------------------------------------
// setup text shadow's color
//---------------------------------------------------------
void TextMgr::SetShadowColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
    shadowColor_.rgba = RGBATOU32(r,g,b,a);
}

void TextMgr::SetShadowColor(const Color c)
{
    shadowColor_ = c;
}

//---------------------------------------------------------
// Desc:  push a text string for drawing onto the screen 
//        (we will render it when call the Render() method)
// NOTE:  if input str begins with '$' then it is a key to some localized string,
//        in another case we print this string as it is
//---------------------------------------------------------
void TextMgr::Draw(const char* str)
{
    if (!str || str[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_KEY);
        return;
    }

    // isn't a key
    if (str[0] != '$')
    {
        DrawText(str);
    }
    else
    {
        const auto& it = keyToText_.find(str);

        if (it == keyToText_.end())
            Draw(L"WTF, DUDE, NO VALUE BY KEY: %s", str);
        else
            Draw(it->second.c_str());
    }
}

//---------------------------------------------------------
// draw FORMATTED input string onto the screen
//---------------------------------------------------------
void TextMgr::Draw(const wchar_t* fmt, ...)
{
    assert(fmt && fmt[0] != L'\0');

    va_list args;
    va_start(args, fmt);

    wchar_t text[256];
    _vsnwprintf(text, ARRAY_SIZE(text), fmt, args);


    // store data for the shadowed, not faded text
    if ((textFlags_ & TEXT_SHADOWED) && !(textFlags_ & TEXT_FADED))
    {
        textsShadowedBuf_.push_back(TextShadowedData());
        TextShadowedData& data = textsShadowedBuf_.back();

        data.text        = text;
        data.textPX      = x_;
        data.textPY      = y_;
        data.textColor   = textColor_;
        data.shadowColor = shadowColor_;
        data.textFlags   = textFlags_;
        data.shadowOffX  = shadowOffX_;
        data.shadowOffY  = shadowOffY_;
        data.font        = currFont_;
    }

    // store data for the faded text
    else if (textFlags_ & TEXT_FADED)
    {
        textsFadedBuf_.push_back(TextFadedData());
        TextFadedData& data = textsFadedBuf_.back();

        data.text        = text;
        data.lifeTime    = fadeForSec_;
        data.currTime    = fadeForSec_;
        data.textPX      = x_;
        data.textPY      = y_;
        data.textColor   = textColor_;
        data.shadowColor = shadowColor_;
        data.textFlags   = textFlags_;
        data.shadowOffX  = shadowOffX_;
        data.shadowOffY  = shadowOffY_;
        data.font        = currFont_;
    }

    // store data for the plain text (not shadowed, not faded)
    else
    {
        textsBuf_.push_back(TextData());
        TextData& data = textsBuf_.back();

        data.text      = text;
        data.textPX    = x_;
        data.textPY    = y_;
        data.textColor = textColor_;
        data.textFlags = textFlags_;
        data.font      = currFont_;
    }

    va_end(args);
}

//---------------------------------------------------------
// get/set locales
//---------------------------------------------------------
const char** TextMgr::GetAvailLocales()    const { return AVAIL_LOCALES; }
const size_t TextMgr::GetNumAvailLocales() const { return ARRAY_SIZE(AVAIL_LOCALES); }
const char*  TextMgr::GetCurrLocale()      const { return locale_; }

//---------------------------------------------------------
// update input top-left coords according to the input flag (text modifier)
//---------------------------------------------------------
void TextMgr::ModifyTextPosAccordingToFlags(
    const uint8* text,
    const size_t textLen,
    const uint16 flags,
    int& top, 
    int& left) const
{
    assert(text);
    assert(textLen > 0);
    assert(flags != 0);

    // text centered horizontally
    if (textFlags_ & TEXT_CENTER_X)
    {
        const int wndMidX    = g_Render.GetWndSize().x >> 1;
        const int halfWidth  = pFont_->GetTextWidth(text, textLen) >> 1;
        left = wndMidX - halfWidth;
    }
    // text centered vertically
    if (textFlags_ & TEXT_CENTER_Y)
    {
        const int wndMidY    = g_Render.GetWndSize().y >> 1;
        const int halfHeight = pFont_->GetFontHeight() >> 1;
        top = wndMidY - halfHeight;
    }
}

//---------------------------------------------------------
// here we actually draw a text string onth the screen
//---------------------------------------------------------
void TextMgr::DrawText(const char* str)
{
    assert(str && str[0] != '\0');

    pFont_ = &fonts_[currFont_];
    assert(pFont_);

    Texture* pFontTex = g_AssetMgr.GetTex(pFont_->GetFontTexId());
    assert(pFontTex);

    // top-left position (in pixels) of the char
    int top = y_;                     
    int left = x_;

    const size_t len = strlen(str);

    //
    // if we have some specific modificators for the text
    //
    if (textFlags_)
        ModifyTextPosAccordingToFlags((const uint8*)str, len, textFlags_, top, left);

    //
    // draw a text onto the screen
    //
    const uint8   startChar = pFont_->GetStartChar();
    const uint16 cellHeight = pFont_->GetCellHeight();

    for (size_t i = 0; i < len; ++i)
    {
        const uint8 charWidth = pFont_->GetCharWidth(str[i]);
        const Recti   srcRect = pFont_->GetCharTexCoords(str[i]);
        const Recti   dstRect = { left, left + charWidth, top, top + cellHeight };

        left += charWidth;

        // draw a single character
        g_Render.DrawRect(pFontTex, srcRect, dstRect, textColor_, 0, NULL, 0.0f);
    }
}

//---------------------------------------------------------
// helper for drawing input text onto the screen
//---------------------------------------------------------
void TextMgr::DrawText(const wchar_t* str)
{
    assert(str && str[0] != L'\0');

    uint8 convStr[512];
    Utf16ToWin1251(str, convStr, sizeof(convStr));

    DrawText((const char*)convStr);
}

//---------------------------------------------------------
// helper for drawing input text + shadow onto the screen
//---------------------------------------------------------
void TextMgr::DrawTextShadowed(const wchar_t* str)
{
    assert(str && str[0] != L'\0');

    pFont_ = &fonts_[currFont_];

    Texture* pFontTex = g_AssetMgr.GetTex(pFont_->GetFontTexId());
    assert(pFontTex);

    int top = y_;                       // top position (in pixels) of the char
    int left = x_;                      // left position (in pixels) of the char
    const size_t len = wcslen(str);

    uint8 convStr[512];
    Utf16ToWin1251(str, convStr, sizeof(convStr));

    //
    // if we have some specific modificators for the text
    //
    if (textFlags_)
        ModifyTextPosAccordingToFlags(convStr, len, textFlags_, top, left);

    //
    // draw a text onto the screen
    //
    const uint8   startChar = pFont_->GetStartChar();
    const uint16 cellHeight = pFont_->GetCellHeight();

    for (size_t i = 0; i < len; ++i)
    {
        const uint8  charWidth = pFont_->GetCharWidth(convStr[i]);
        const Recti    srcRect = pFont_->GetCharTexCoords(convStr[i]);
        const Recti dstRectTxt = { left, left + charWidth, top, top + cellHeight };
        
        left += charWidth;

        // first, draw a shadow of the char...
        const Recti dstRectShad = {
            dstRectTxt.x0 + shadowOffX_,
            dstRectTxt.x1 + shadowOffX_,
            dstRectTxt.y0 + shadowOffY_,
            dstRectTxt.y1 + shadowOffY_,
        };
        g_Render.DrawRect(pFontTex, srcRect, dstRectShad, shadowColor_, 0, NULL, 0.0f);

        // second, draw a char by itself...
        g_Render.DrawRect(pFontTex, srcRect, dstRectTxt, textColor_, 0, NULL, 0.0f);
    }
}