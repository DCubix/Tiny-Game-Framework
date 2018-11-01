//
// Created by Twister on 05/10/2018.
//

#ifndef TGF_SOUND_H
#define TGF_SOUND_H

#include "Types.h"
#include "SDL2/SDL.h"

#define AUS_SAMPLERATE 22050
#define AUS_ONE_NOTE_SPEED (AUS_SAMPLERATE / 16)
#define AUS_CHANNEL_COUNT 4

#define NPITCH(x) (440.0f * std::pow(2.0f, float((x) - 33) / 12.0f))

namespace tgf {
	class Oscillator {
	public:
		enum WaveForm {
			Triangle = 0,
			Square,
			Pulse,
			Noise
		};

		Oscillator() = default;
		Oscillator(float freq, float vol);

		float frequency() const { return m_frequency; }
		void frequency(float f) { m_frequency = f; }
		float volume() const { return m_volume; }
		void volume(float v) { m_volume = v; }
		WaveForm waveForm() const { return m_waveForm; }
		void waveForm(WaveForm wf) { m_waveForm = wf; }

		float sample();
		void stop();

	private:
		float m_phase, m_frequency, m_volume;
		float m_lastNoise;
		WaveForm m_waveForm;
	};

	class Sound {
	public:
		Sound() = default;
		Sound(const Str& notes, const Str& wfs, const Str& volumes, const Str& effects, float speed);

		void setNotes(const Str& v);
		void setWaveForms(const Str& v);
		void setVolumes(const Str& v);
		void setEffects(const Str& v);
		void set(const Str& notes, const Str& wfs, const Str& volumes, const Str& effects);

		u32 noteCount() const { return u32(m_notes.size()); }
		Vec<i32> notes() const { return m_notes; }
		Vec<i32> waveForms() const { return m_waveForms; }
		Vec<i32> volumes() const { return m_volumes; }
		Vec<i32> effects() const { return m_effects; }

		float speed() const { return m_speed; }
		void setSpeed(float v) { m_speed = v; }

	private:
		Vec<i32> m_notes, m_waveForms, m_volumes, m_effects;
		float m_speed;
	};

	class Channel {
		friend class AudioSystem;
	public:
		Channel();
		virtual ~Channel() = default;

		void play(const Vec<Sound*>& soundList, bool loop);
		void stop();

		float output() { update(); return m_oscillator->sample() * 0.5f; }
		bool playing() const { return m_playing; }

	protected:
		void playSound();
		void update();

	private:
		Ptr<Oscillator> m_oscillator;

		bool m_playing, m_loops;
		Vec<Sound*> m_soundList;
		u32 m_soundIndex, m_pos;

		float m_time, m_oneTime, m_oneNoteTime, m_totalNoteTime,
			m_pitch, m_effectTime, m_effectPitch, m_effectVol;
		i32 m_note, m_effect;
	};

	class AudioSystem {
	public:
		AudioSystem();
		virtual ~AudioSystem() { SDL_CloseAudioDevice(m_device); }

		void play(u32 channel, const Vec<Sound*>& sounds, bool loop);
		void playSingle(u32 channel, Sound* sound, bool loop);
		void stop(u32 channel);

		Array<Ptr<Channel>, AUS_CHANNEL_COUNT>& channels() { return m_channels; }
	private:
		Array<Ptr<Channel>, AUS_CHANNEL_COUNT> m_channels;

		SDL_AudioDeviceID m_device;
		SDL_AudioSpec m_spec;
	};
}


#endif //TGF_SOUND_H
