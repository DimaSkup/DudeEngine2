/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   sound.h
    Desc:       a wrapper for sound stuff
    Created:    13.07.2026
\**********************************************************************************/
#pragma once

#include "types.h"

//---------------------------------------------------------
// forward declaration (pointer use only)
//---------------------------------------------------------
struct ma_sound;

class Sound
{
public:
    ~Sound();

    static bool InitSoundEngine(void);

    bool        LoadFromFile(SoundID id, const char* path);
    void        Play(void);
    void        Stop(void);

    void        SetVolume(const float volume);
    void        SetLooping(bool bLooping);

    float       GetVolume(void) const;

private:
    SoundID   id_;
    ma_sound* pSound_;
};
