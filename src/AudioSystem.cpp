//
// Created by Twister on 05/10/2018.
//

#include "AudioSystem.h"

#include "Log.h"
#include <algorithm>
#include <cmath>

namespace tgf {

	Sound::Sound(const Str& notes, const Str& wfs, const Str& volumes, const Str& effects, float speed) {
		setNotes(notes);
		setWaveForms(wfs);
		setVolumes(volumes);
		setEffects(effects);
		m_speed = speed;
	}

	void Sound::setNotes(const Str& v) {
		m_notes.clear();

		Str data = v;
		std::transform(data.begin(), data.end(), data.begin(), ::toupper);
		data.erase(std::remove_if(data.begin(), data.end(), ::isspace), data.end());

		StrReader sr(data);
		char c = '\0';
		while (sr.hasNext()) {
			c = sr.next();

			i32 param = -1;
			switch (c) {
				case 'C': param = 0; break;
				case 'D': param = 2; break;
				case 'E': param = 4; break;
				case 'F': param = 5; break;
				case 'G': param = 7; break;
				case 'A': param = 9; break;
				case 'B': param = 11; break;
				default: break;
			}

			if (param != -1) {
				c = sr.next();
				if (isdigit(c)) {
					int oct = (c - '0');
					if (oct >= 0 && oct <= 5) {
						param += oct * 12;
						c = sr.peek();
					} else LogE("Invalid value. \"", c, "\"");
				} else LogE("Invalid parameter. \"", c, "\"");

				if (c == '#' || c == '-') {
					param += c == '#' ? 1 : -1;
					c = sr.next();
				}
			} else if (c == '!') {
				param = -1;
			} else {
				LogE("Invalid parameter. \"", c, "\"");
			}

			m_notes.push_back(param);
		}
	}

	void Sound::setWaveForms(const Str& v) {
		m_waveForms.clear();

		Str data = v;
		std::transform(data.begin(), data.end(), data.begin(), ::toupper);
		data.erase(std::remove_if(data.begin(), data.end(), ::isspace), data.end());

		StrReader sr(data);
		char c = sr.next();
		while (c != '\0') {
			i32 param = -1;
			switch (c) {
				case 'T': param = 0; break;
				case 'S': param = 1; break;
				case 'P': param = 2; break;
				case 'N': param = 3; break;
				default: break;
			}

			if (param != -1) {
				m_waveForms.push_back(param);
			} else {
				LogE("Invalid parameter. \"", c, "\"");
			}

			c = sr.next();
		}
	}

	void Sound::setVolumes(const Str& v) {
		m_volumes.clear();

		Str data = v;
		std::transform(data.begin(), data.end(), data.begin(), ::toupper);
		data.erase(std::remove_if(data.begin(), data.end(), ::isspace), data.end());

		StrReader sr(data);
		char c = sr.next();
		while (c != '\0') {
			i32 param = -1;
			if (isdigit(c)) {
				param = (c - '0');
				if (param >= 0 && param <= 7)
					m_volumes.push_back(param);
			} else {
				LogE("Invalid parameter. \"", c, "\"");
			}
			c = sr.next();
		}
	}

	void Sound::setEffects(const Str& v) {
		m_effects.clear();

		Str data = v;
		std::transform(data.begin(), data.end(), data.begin(), ::toupper);
		data.erase(std::remove_if(data.begin(), data.end(), ::isspace), data.end());

		StrReader sr(data);
		char c = sr.next();
		while (c != '\0') {
			i32 param = -1;
			switch (c) {
				case 'N': param = 0; break;
				case 'S': param = 1; break;
				case 'V': param = 2; break;
				case 'F': param = 3; break;
				default: break;
			}

			if (param != -1) {
				m_effects.push_back(param);
			} else {
				LogE("Invalid parameter. \"", c, "\"");
			}

			c = sr.next();
		}
	}

	void Sound::set(const Str& notes, const Str& wfs, const Str& volumes, const Str& effects) {
		setNotes(notes);
		setWaveForms(wfs);
		setVolumes(volumes);
		setEffects(effects);
	}

	Oscillator::Oscillator(float freq, float vol) {
		m_frequency = freq;
		m_volume = vol;
		m_waveForm = WaveForm::Triangle;
		m_phase = 0;
	}

	float Oscillator::sample() {
		float p = m_phase;

		float v = 0.0f;
		switch (m_waveForm) {
			case Triangle: {
				if (p < PI) {
					v = -1.0f + (2.0f / PI) * p;
				} else {
					v = 3.0f - (2.0f / PI) * p;
				}
			} break;
			case Square: v = p < PI ? 1.0f : -1.0f; break;
			case Pulse: v = p < PI * 0.2f ? 1.0f : -1.0f; break;
			case Noise: {
				float dt = std::fmod(p, m_frequency);
				if (dt <= 1.0f) {
					m_lastNoise = (float(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
				}
				v = m_lastNoise * 0.5f;
			} break;
		}

		m_phase += (PI * 2.0f * m_frequency) / AUS_SAMPLERATE;
		if (m_phase > PI * 2.0f) {
			m_phase -= PI * 2.0f;
		}

		return v * m_volume;
	}

	void Oscillator::stop() {
		m_frequency = 0;
		m_volume = 0;
		m_phase = 0;
	}

	Channel::Channel() {
		m_oscillator = std::make_unique<Oscillator>(0, 0);
		m_playing = false;
		m_loops = false;
		m_effectPitch = 0;
		m_effectTime = 0;
		m_effectVol = 0;
		m_soundIndex = 0;
		m_time = 0;
		m_oneNoteTime = 0;
		m_totalNoteTime = 0;

	}

	void Channel::playSound() {
		Sound* snd = m_soundList[m_soundIndex];
		m_oneNoteTime = snd->speed();
		m_totalNoteTime = m_oneNoteTime * snd->noteCount();
		m_time = 0;
		m_pos = 0;
		m_oneTime = m_oneNoteTime;
	}

	void Channel::play(const Vec<Sound*>& soundList, bool loop) {
		m_playing = true;
		m_loops = loop;
		m_soundList = soundList;
		m_soundIndex = 0;
		playSound();
	}

	void Channel::stop() {
		m_playing = false;
		m_pitch = 0;
		m_oscillator->stop();
	}

	static float lfo(float t) {
		return std::sin(t * 48) * 0.5f + 0.5f;
	}

	void Channel::update() {
		if (!m_playing) return;

		// Next note
		if (m_oneTime >= m_oneNoteTime) {
			Sound* snd = m_soundList[m_soundIndex];
			m_note = snd->notes()[m_pos % snd->notes().size()];
			float volume = float(snd->volumes()[m_pos % snd->volumes().size()]) / 7.0f;

			if (m_note >= 0 && volume > 0.0f) {
				float lastPitch = m_pitch;
				Oscillator::WaveForm wf = (Oscillator::WaveForm) snd->waveForms()[m_pos % snd->waveForms().size()];
				m_pitch = NPITCH(m_note);
				m_effect = snd->effects()[m_pos % snd->effects().size()];

				m_oscillator->frequency(m_pitch);
				m_oscillator->volume(volume);
				m_oscillator->waveForm(wf);

				switch (m_effect) {
					default: break;
					case 1:
						m_effectTime = m_time;
						m_effectPitch = lastPitch > 0 ? lastPitch : m_pitch;
						break; // Slide
					case 2:
						m_effectTime = m_time;
						m_effectPitch = NPITCH(m_note - 1) - m_pitch;
						break; // Vibrato
					case 3:
						m_effectTime = m_time;
						m_effectVol = volume;
						break; // Fade-out
				}
			} else {
				m_oscillator->stop();
			}
			m_pos++;
			m_oneTime = 0;
		}
		m_oneTime += 1.0f / float(AUS_SAMPLERATE);

		// Play note
		if (m_note >= 0) {
			switch (m_effect) {
				default: break;
				case 1: {
					float a = (m_time - m_effectTime) / m_oneNoteTime;
					float p = (1.0f - a) * m_effectPitch + m_pitch * a;
					m_oscillator->frequency(p);
				} break;
				case 2: {
					float p = m_pitch + lfo(m_time) * m_effectPitch;
					m_oscillator->frequency(p);
				} break;
				case 3: {
					m_oscillator->volume(
							m_effectVol * (1.0f - ((m_time - m_effectTime) / m_oneNoteTime))
					);
				} break;
			}
		}

		m_time += 1.0f / float(AUS_SAMPLERATE);

		if (m_time >= m_totalNoteTime) {
			m_soundIndex++;
			if (m_soundIndex < m_soundList.size()) {
				playSound();
			} else if (m_loops) {
				m_soundIndex = 0;
				playSound();
			} else {
				stop();
			}
			m_time = 0;
		}
	}

	static void audioCallback(void* ud, Uint8* stream, int length) {
		auto* aus = static_cast<AudioSystem*>(ud);
		int flen = length / int(sizeof(float));
		auto* fstream = reinterpret_cast<float*>(stream);

		for (int i = 0; i < flen; i++) {
			float out = 0.0f;
			for (auto&& ch : aus->channels()) {
				out += ch->output();
			}
			fstream[i] = out;
		}
	}

	AudioSystem::AudioSystem() {
		SDL_AudioSpec spec;
		spec.freq = AUS_SAMPLERATE;
		spec.samples = 2048;
		spec.channels = 1;
		spec.callback = audioCallback;
		spec.userdata = this;
		spec.format = AUDIO_F32;

		if ((m_device = SDL_OpenAudioDevice(nullptr, 0, &spec, &m_spec, 0)) < 0) {
			LogE(SDL_GetError());
			return;
		}

		for (u32 i = 0; i < AUS_CHANNEL_COUNT; i++) {
			m_channels[i] = std::make_unique<Channel>();
		}

		SDL_PauseAudioDevice(m_device, 0);
	}

	void AudioSystem::play(u32 channel, const Vec<Sound*>& sounds, bool loop) {
		m_channels[channel]->play(sounds, loop);
	}

	void AudioSystem::playSingle(u32 channel, Sound* sound, bool loop) {
		Vec<Sound*> vec;
		vec.reserve(1);
		vec.push_back(sound);
		play(channel, vec, loop);
	}

	void AudioSystem::stop(u32 channel) {
		m_channels[channel]->stop();
	}

}