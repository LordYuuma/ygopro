#ifndef SOUND_CANBERRA_H
#define SOUND_CANBERRA_H
#include "sound_backend.h"
#include <cstdint>
#include <array>
#include <string>

struct ca_context;

class SoundCanberra : public SoundBackend {
public:
	SoundCanberra();
	~SoundCanberra();
	void SetSoundVolume(double volume);
	void SetMusicVolume(double volume);
	bool PlayMusic(const std::string& name, bool loop);
	bool PlaySound(const std::string& name);
	void StopSounds();
	void StopMusic();
	void PauseMusic(bool pause);
	bool MusicPlaying();
	void Tick();
private:
	ca_context* ctx;
	std::string loop_music;
	const uint32_t MUSIC = 1;
	const uint32_t SOUND = 2;
	const std::array<const char*,3> channels = { NULL, "MUSIC", "SOUND" };
	double sound_db, music_db;
	bool Play(uint32_t channel, const std::string& name, double volume);
};

#endif //SOUND_CANBERRA_H
