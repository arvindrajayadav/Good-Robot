/*-----------------------------------------------------------------------------

  Projectile.cpp

  A c-style module that manages the loading and playback of sound effects.
  Built on OpenAL.

  Good Robot
  (c) 2015 Pyrodactyl

  -------------------------------------------------------------------------------

  -----------------------------------------------------------------------------*/

#include "master.h"

//Not sure if this is correct way to do this, but defining this symbol will
//make OpenAL shut its stupid gob about deprecated wave-loading.
#define MIDL_PASS


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include "audio.h"
#include "env.h"
#include "file.h"
#include "game.h"
#include "ini.h"
#include "player.h"
#include "random.h"
#include "resource.h"
#include "stb_vorbis.h"
#include "system.h"
#include "loaders.h"

#define MODULATE            1
//Max number of sounds that may play at once. If we run out, we clobber
//ongoing sounds, starting oldest-first.
#define MAX_CHANNELS        16
//To keep sounds from becoming repetitive, we modulate each instance by a random
//ammount. This range specified by how far from 1.0 these values may deviate.
#define MODULATE_RANGE      0.33f
//How many different modulate values we store. We don't need many. No need to
//hit the RNG for a fresh value every time.
#define MODULATE_VALUES     (modulate.size ())
//When a particular sound is made, this is how many ms to wait before the same
//sound can be triggered again. Keeps spammy sounds from hogging all the channels
//and prevents volume bursts from duplicate overlapping samples.
#define SOUND_COOLDOWN      100
//Distance at which sounds fall off.
#define MAX_DISTANCE				14.0f

struct AudioEntry
{
	std::string   index;
	std::string   filename;
	bool          pitchmod;
};

struct AudioData
{
	bool        modulate;
	unsigned    buffer;
	string      filename;
	int         cooldown;
};

struct LoopChannel
{
	std::string effect;
	bool        playing;
	unsigned    channel;
};

struct AudioJob
{
	string    filename;
	bool      locked;
	int       buffer_id;
	int       channel;
};

struct oggFile
{
	string    name;
	bool      ready;
	int       stream_size;
	int       channels;
	int       rate;
	short*    buffer;
};

static vector<float>                          modulate;
static unsigned                               channel_music;
static unsigned                               buffer_music;
static unsigned                               channel[MAX_CHANNELS];
static int                                    current_channel;
static int                                    current_mod;
static vector<AudioEntry>                     file_list;
static unordered_map<std::string, AudioData>  library;
static int                                    song_transition_start;
static vector<LoopChannel>                    loop;
static LoopChannel                            looping[LOOP_COUNT];
static bool                                   music_playing;
static bool                                   loops_paused;
static bool  																	music_supress;
static float																	music_supress_gain;
static oggFile                                ogg_file;
static AudioJob                               audio_job;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static float audio_modulate()
{
#if MODULATE
	current_mod++;
	current_mod %= MODULATE_VALUES;
	return modulate[current_mod];
#else
	return 1.0f;
#endif
}

static void play(const std::string &effect, GLvector2 position, float pitch)
{
	if (GameTick() < library[effect].cooldown)
		return;

	if (library.count(effect) <= 0)
		return;

	float pos[3];
	library[effect].cooldown = GameTick() + SOUND_COOLDOWN;
	pos[0] = position.x;
	pos[1] = position.y;
	pos[2] = -1.0f;
	alSourceStop(channel[current_channel]);
	alSourcefv(channel[current_channel], AL_POSITION, pos);
	//if (library[effect].modulate)
	if (0)
		alSourcef(channel[current_channel], AL_PITCH, audio_modulate() * pitch);
	else
		alSourcef(channel[current_channel], AL_PITCH, pitch);
	alSourcei(channel[current_channel], AL_BUFFER, library[effect].buffer);
	alSourcePlay(channel[current_channel]);
	current_channel++;
	current_channel %= MAX_CHANNELS;
}

static bool audio_load(const char* file, int buffer, bool music = false)
{
	const char* filename;
	string      location;
	int         channels;
	bool        is_wave;

	if (music)
		location = ResourceLocation(file, RESOURCE_MUSIC);
	else
		location = ResourceLocation(file, RESOURCE_SOUND);
	filename = location.c_str();

	is_wave = false;
	if (strstr(file, ".wav"))
		is_wave = true;
	if (is_wave) {
		int       bits;
		ALenum    formato;
		ALsizei   size;
		ALvoid*   dati;
		ALsizei   freq;
#if defined(__APPLE__)
		alutLoadWAVFile((ALbyte*)filename, &formato, &dati, &size, &freq);
#else
		ALboolean loop;

		//"Strongly deprecated!" But IT WORKS.
		alutLoadWAVFile((ALbyte*)filename, &formato, &dati, &size, &freq, &loop);
#endif
		alBufferData(buffer, formato, dati, size, freq);
		//Buffer = alutCreateBufferFromFile (file); Doesn't F'ing work!
		alGetBufferi(buffer, AL_CHANNELS, &channels);
		alGetBufferi(buffer, AL_FREQUENCY, &freq);
		alGetBufferi(buffer, AL_BITS, &bits);
		alGetBufferi(buffer, AL_SIZE, &size);
		alutUnloadWAV(formato, dati, size, freq);
	}
	else {
		long      fsize;
		long      stream_size;
		int       rate;
		uchar*    stream;
		short*    ogg_output;
		unsigned  format;

		stream = (uchar*)FileContentsBinary(filename, &fsize);
		if (!stream) {
			Console("File not found: %s", filename);
			return false;
		}
		stream_size = stb_vorbis_decode_memory(stream, fsize, &channels, &rate, &ogg_output);
		format = AL_FORMAT_MONO16;
		if (channels == 2)
			format = AL_FORMAT_STEREO16;
		//I'm not sure why we multiply stream size by two, but if we don't
		//then we only hear half the sounds. Conjecture: Vorbis gives us shorts
		//and OpenAL takes chars?
		alBufferData(buffer, format, ogg_output, stream_size * 2 * channels, rate);
		free(stream);
		free(ogg_output);
	}
	return true;
}

static int audio_load_thread(void* data)
{
	AudioJob*     aj = (AudioJob*)data;

	aj->locked = true;
	alSourceStop(aj->channel);
	alSourcei(aj->channel, AL_BUFFER, 0);
	audio_load(aj->filename.c_str(), aj->buffer_id, true);
	alSourcei(aj->channel, AL_BUFFER, aj->buffer_id);
	alSourcei(aj->channel, AL_LOOPING, true);
	alSourcePlay(aj->channel);
	aj->locked = false;
	return 0;
}

int create_channel()
{
	float     zero[] = { 0.0, 0.0, 0.0 };
	unsigned  id;

	alGenSources(1, &id);
	alSourcef (id, AL_PITCH, 1.0f);
	alSourcef (id, AL_GAIN, 1.0f);
	alSourcefv(id, AL_POSITION, zero);
	alSourcefv(id, AL_VELOCITY, zero);
	alSourcei (id, AL_LOOPING, false);
	alSourcei (id, AL_PLAYING, false);
	alSourcef (id, AL_MAX_DISTANCE, MAX_DISTANCE);
	alSourcef (id, AL_REFERENCE_DISTANCE, 3.0f);
	alSourcef (id, AL_ROLLOFF_FACTOR, 1.0f);
	return id;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void AudioFileListInit()
{
	file_list.clear();

	using namespace pyrodactyl;
	XMLDoc sound_list("core/sounds/sounds.xml");
	if (sound_list.ready())
	{
		rapidxml::xml_node<char> *node = sound_list.Doc()->first_node("sounds");
		for (auto n = node->first_node(); n != NULL; n = n->next_sibling())
		{
			std::string filename;
			if (LoadStr(filename, "file", n))
			{
				AudioEntry a;

				std::size_t found = filename.find(".");
				if (found != std::string::npos)
					a.index = filename.substr(0, found);
				else
					a.index = filename;

				a.filename = filename;
				LoadBool(a.pitchmod, "pitchmod", n);

				file_list.push_back(a);
			}
		}
	}
}

bool AudioValid(const std::string &effect)
{
	return (library.count(effect) > 0);
}

void AudioUpdate()
{
	int     delta;
	float   music_gain;
	float   fade;

	fade = 1.0f;
	alListenerf(AL_GAIN, EnvValuef(ENV_VOLUME_FX));
	if (song_transition_start) {
		delta = SystemTick() - song_transition_start;
		fade = 1.0f - ((float)delta / 2000.0f);
		if (fade <= 0.0f) {
			SystemThread("audio_load_thread", audio_load_thread, (void*)&audio_job);
			song_transition_start = 0;
		}
	}
	//Game paused, so pause audio loops.
	if (!loops_paused && !GameActive()) {
		loops_paused = true;
		for (unsigned i = 0; i < LOOP_COUNT; i++) {
			if (looping[i].playing)
				alSourcePause(looping[i].channel);
		}
	}
	//Game resumed, so resume loops.
	if (loops_paused && GameActive()) {
		loops_paused = false;
		for (unsigned i = 0; i < LOOP_COUNT; i++) {
			if (looping[i].playing)
				alSourcePlay(looping[i].channel);
		}
	}
	music_gain = EnvValuef(ENV_VOLUME_MUSIC);
	if (music_supress) 
		music_supress_gain -= 0.01f;
	else
		music_supress_gain += 0.01f;
	music_supress_gain = clamp (music_supress_gain, 0.0f, 1.0f);
	music_supress = false;
	alSourcef (channel_music, AL_GAIN, fade * music_gain * music_supress_gain);
}

void AudioInit()
{
	float           listen_pos[] = { 0.0, 0.0, 0.0 };
	float           listen_vel[] = { 0.0, 0.0, 0.0 };
	float           listen_angle[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };
	ALCdevice*      device;
	ALCcontext*     context;
	iniFile         ini;
	vector<string>  pitchmod_list;

	ini.Open(ResourceLocation(GAMEPLAY_FILE, RESOURCE_DATA));
	pitchmod_list = StringSplit(ini.StringGet("Audio", "Pitchmod"), ", ");
	for (unsigned i = 0; i < pitchmod_list.size(); i++) {
		modulate.push_back(StringToFloat(pitchmod_list[i]));
	}
	if (modulate.empty())
		modulate.push_back(1.0f);

	device = alcOpenDevice(NULL);
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	alGetError();
	alListenerfv(AL_POSITION, listen_pos);
	alListenerfv(AL_VELOCITY, listen_vel);
	alListenerfv(AL_ORIENTATION, listen_angle);
	alDistanceModel (AL_LINEAR_DISTANCE_CLAMPED);
	//Set up channels.
	channel_music = create_channel();
	alGenBuffers(1, &buffer_music);
	for (int i = 0; i < MAX_CHANNELS; i++)
		channel[i] = create_channel();

	//Load the sound file list
	AudioFileListInit();

	//Load all the audio files
	for (unsigned i = 0; i < file_list.size(); i++){
		std::string entry = file_list[i].index;

		AudioData a;
		library[entry] = a;

		library[entry].filename = file_list[i].filename;
		library[entry].modulate = file_list[i].pitchmod;
		library[entry].cooldown = 0;
		alGenBuffers(1, &library[entry].buffer);
		audio_load(file_list[i].filename.c_str(), library[entry].buffer);
	}
	//Set up the loops for continuous background sounds.
	for (unsigned i = 0; i < LOOP_COUNT; i++) {
		looping[i].channel = create_channel();
		looping[i].playing = false;
	}
	looping[LOOP_INCOMING].effect = "alarm";
	looping[LOOP_MOVE].effect = "move";
	looping[LOOP_ACCESS].effect = "vibration";
	looping[LOOP_FORCEFIELD].effect = "forcefield";
	alSourcei(channel_music, AL_BUFFER, 0);
	Console("AudioInit: %u files loaded.", file_list.size());
}

void AudioPlay(const std::string &effect, GLvector2 position)
{
	GLvector2   offset;

	offset = position - PlayerPosition();
	play(effect, offset, 1.0f);
}

void AudioPlay(const std::string &effect, float pitch)
{
	play(effect, GLvector2(), pitch);
}

void AudioPauseMusic(bool pause)
{
	if (pause) {
		if (music_playing) {
			music_playing = false;
			alSourcePause(channel_music);
		}
	}	else {
		if (!music_playing) {
			music_playing = true;
			alSourcePlay(channel_music);
		}
	}
}

const char* AudioCurrentSong()
{
	return audio_job.filename.c_str();
}

void AudioPlaySong(const char* name, bool crossfade)
{
	if (!stricmp(name, audio_job.filename.c_str()))
		return;
	if (crossfade)
		song_transition_start = SystemTick();
	else
		song_transition_start = 1;
	audio_job.filename = name;
	audio_job.channel = channel_music;
	audio_job.buffer_id = buffer_music;
	Console("AudioPlaySong: Queuing '%s'", name);
}

void AudioStop()
{
	alSourceStop(channel_music);
}

void AudioPause()
{
	alSourcePause(channel_music);
}

void AudioResume()
{
	alSourcePlay(channel_music);
}

void AudioLoop(SoundLoop id, const std::string &effect, float pitch, float gain)
{
	if (looping[id].effect != effect) {
		looping[id].effect = effect;
		alSourceStop(looping[id].channel);
		looping[id].playing = false;
		if (library.count(effect) > 0) {
			float   pos[] = { 0, 0, 0 };
			alSourcefv(looping[id].channel, AL_POSITION, pos);
			alSourcei(looping[id].channel, AL_LOOPING, true);
			alSourcei(looping[id].channel, AL_BUFFER, library[effect].buffer);
			alSourcePlay(looping[id].channel);
			looping[id].playing = true;
		}
	}
	alSourcef(looping[id].channel, AL_PITCH, pitch);
	alSourcef(looping[id].channel, AL_GAIN, gain);
}

void AudioMusicSupress ()
{
	music_supress = true;
}

void AudioLoop(SoundLoop id, float pitch, float gain)
{
	if (!looping[id].playing) {
		float   pos[] = { 0, 0, 0 };
		alSourcefv(looping[id].channel, AL_POSITION, pos);
		alSourcei(looping[id].channel, AL_LOOPING, true);
		alSourcei(looping[id].channel, AL_BUFFER, library[looping[id].effect].buffer);
		alSourcePlay(looping[id].channel);
		looping[id].playing = true;
	}
	alSourcef(looping[id].channel, AL_PITCH, pitch);
	alSourcef(looping[id].channel, AL_GAIN, gain);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void AudioTerm()
{
	//alDeleteSources(1, &source);
	alutExit();
}