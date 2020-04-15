#include "sound_canberra.h"
#include "logging.h"
#include <stdexcept>
#include <fmt/format.h>
#include <queue>
#include <cmath>
#include <canberra.h>

SoundCanberra::SoundCanberra()
{
	ca_context_create(&ctx);
	/* Note: each of those backends come with their own share of problems.
	 * "alsa", "pulse", and "oss" only support a subset of wav and vorbis.
	 * "gstreamer" supports a wide array of formats, but libcanberra has
	 * no volume control for it, also it spawns a new bin per active sound,
	 * making external volume control a pain as well.
	 * To set a specific one, use the CANBERRA_DRIVER environment variable.
	 */
	std::queue<std::string> backends({"alsa", "pulse", "oss", "gstreamer" });
	ca_context_change_props(ctx, CA_PROP_APPLICATION_NAME, "edopro", CA_PROP_APPLICATION_ID, "io.github.edo9300.edopro", CA_PROP_APPLICATION_ICON_NAME, "edopro", NULL);
	while (ca_context_open(ctx) != 0) {
		auto backend = backends.front();
		ca_context_set_driver(ctx, backend.c_str());
		backends.pop();
		if (backends.empty())
			throw std::runtime_error("no available backend");
	}
}

SoundCanberra::~SoundCanberra()
{
	ca_context_cancel(ctx, MUSIC);
	ca_context_cancel(ctx, SOUND);
	ca_context_destroy(ctx);
}

void SoundCanberra::SetSoundVolume(double volume)
{
	sound_db = 10*std::log(volume);
	ca_context_cancel(ctx, SOUND);
}

void SoundCanberra::SetMusicVolume(double volume)
{
	music_db = 10*std::log(volume);
	ca_context_cancel(ctx, MUSIC);
}

bool SoundCanberra::Play(uint32_t id, const std::string& name, double volume)
{
	ca_proplist* props;
	ca_proplist_create(&props);
	ca_proplist_sets(props, CA_PROP_MEDIA_FILENAME, name.c_str());
	ca_proplist_setf(props, CA_PROP_CANBERRA_VOLUME, "%g", volume);
	int err = ca_context_play_full(ctx, id, props, NULL, NULL);
	ca_proplist_destroy(props);
	if (err)
		ygo::ErrorLog(fmt::format("While trying to play {} on {}: {}", name, std::string(channels[id]), std::string(strerror(err))));
	return !err;
}

bool SoundCanberra::PlayMusic(const std::string& name, bool loop)
{
	StopMusic();
	if (loop)
		loop_music = name;
	else
		loop_music = "";
	return Play(MUSIC, name, music_db);
}

bool SoundCanberra::PlaySound(const std::string& name)
{
	return Play(SOUND, name, sound_db);
}

void SoundCanberra::StopMusic()
{
	loop_music = "";
	ca_context_cancel(ctx, MUSIC);
}

void SoundCanberra::StopSounds()
{
	ca_context_cancel(ctx, SOUND);
}

void SoundCanberra::PauseMusic(bool pause)
{
	if (pause)
		StopMusic();
}

bool SoundCanberra::MusicPlaying()
{
	if (loop_music != "")
		return true;
	else  {
		int is_playing;
		return ca_context_playing(ctx, MUSIC, &is_playing) == 0
			&& is_playing;
	}
}

void SoundCanberra::Tick()
{
	if (loop_music != "")
	{
		int is_playing;
		if (ca_context_playing(ctx, MUSIC, &is_playing) == 0 &&
			!is_playing)
			Play(MUSIC, loop_music, music_db);
	}
}
