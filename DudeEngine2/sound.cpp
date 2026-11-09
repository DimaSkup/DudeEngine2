/**********************************************************************************\

    ******    **    **  ******    ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    **    **  **    **  **    **  ********
    **    **  **    **  **    **  **
    **    **  **    **  **    **  **
    ******     ******   ******    ********

    Filename:   sound.cpp
    Desc:       implementation of a wrapper for sound stuff
    Created:    13.07.2026
\**********************************************************************************/
#include "sound.h"
#include "log.h"
#include "consts.h"
#include "mem_helpers.h"

#include <miniaudio.h>
#include <assert.h>


// sound engine
static ma_engine s_SoundEngine;
static bool s_bEngineInit = false;

//---------------------------------------------------------
// called only once to init the sound engine for later using for all the particular sounds
//---------------------------------------------------------
bool Sound::InitSoundEngine()
{
    if (s_bEngineInit == true)
        return true;

    if (ma_engine_init(NULL, &s_SoundEngine) != MA_SUCCESS)
    {
        LogErr(LOG, "failed to init the sound engine");
        return false;
    }

    s_bEngineInit = true;
    
    LogMsg(LOG, "sound engine is initialized");
    return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
Sound::~Sound()
{
    LogMsg(LOG, "unload the sound");
    ma_sound_uninit(pSound_);
    SafeDelete(pSound_);
}

//---------------------------------------------------------
// Desc:  init a sound loading it from a file
// Args:  id   - a unique identifier for this sound object
//        path - a path to the sound file
//---------------------------------------------------------
bool Sound::LoadFromFile(SoundID id, const char* path) 
{
    if (!path || path[0] == '\0')
    {
        LogErr(LOG, ERR_EMPTY_PATH);
        return false;
    }

    if (pSound_)
    {
        LogErr(LOG, "the sound object is already initialized, you can't load another sound file: %s", path);
        return false;
    }

    pSound_ = new ma_sound();

    ma_result r = ma_sound_init_from_file(&s_SoundEngine, 
                                          path, 
                                          0, //MA_SOUND_FLAG_DECODE, 
                                          NULL, 
                                          NULL, 
                                          pSound_);
    if (r != MA_SUCCESS)
    {
        LogErr(LOG, "can't init (err code: %d) a sound from file: %s", (int)r, path);
    }

    id_ = id;
    return true;
}

//---------------------------------------------------------
// play this sound
//---------------------------------------------------------
void Sound::Play(void) 
{
    assert(pSound_);
    if (ma_sound_start(pSound_) != MA_SUCCESS)
    {
        LogErr(LOG, "can't play a sound (id: %d)", (int)id_);
    }
}

//---------------------------------------------------------
// stop this sound
//---------------------------------------------------------
void Sound::Stop(void)
{
    assert(pSound_);
    ma_sound_stop(pSound_);
}

//---------------------------------------------------------
// set/get a volume for this sound
//---------------------------------------------------------
void Sound::SetVolume(const float volume)
{
    assert(pSound_);
    ma_sound_set_volume(pSound_, volume);
}

float Sound::GetVolume() const
{
    assert(pSound_);
    return ma_sound_get_volume(pSound_);
}

//---------------------------------------------------------
// enable/disable looping for this sound
//---------------------------------------------------------
void Sound::SetLooping(bool bLooping) 
{
    assert(pSound_);
    ma_sound_set_looping(pSound_, bLooping);
}

