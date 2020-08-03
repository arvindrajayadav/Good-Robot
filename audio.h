#ifndef AUDIO_H
#define AUDIO_H


#if defined(__APPLE__)
#include <string>
#include "master.h"
#endif

enum SoundLoop
{
	LOOP_INCOMING,
	LOOP_MOVE,
	LOOP_ACCESS,
	LOOP_PRIMARY,
	LOOP_SECONDARY,
	LOOP_FORCEFIELD,
	LOOP_COUNT
};

const char* AudioCurrentSong();
bool        AudioValid(const std::string &effect);
void        AudioPlaySong(const char* name, bool crossfade = true);
void        AudioInit();
void        AudioLoad(char* file);
void        AudioLoop(SoundLoop id, const std::string &effect, float pitch, float gain);
void        AudioLoop(SoundLoop id, float pitch, float gain);
void        AudioPause();
void        AudioResume();
void        AudioPauseMusic(bool pause);
void				AudioMusicSupress ();
void        AudioPlay(const std::string &effect, float pitch = 1);
void        AudioPlay(const std::string &effect, GLvector2 position);
void        AudioStop();
void        AudioTerm();
void        AudioUpdate();

#endif // AUDIO_H