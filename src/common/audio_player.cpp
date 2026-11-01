/*
 * This file is part of EasyRPG Editor.
 *
 * EasyRPG Editor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Editor. If not, see <http://www.gnu.org/licenses/>.
 */

#include "audio_player.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioSink>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QIODevice>
#include <QMediaPlayer>
#include <QTimer>
#include <QUrl>
#include <QtConcurrent>
#include <algorithm>
#include <fluidsynth.h>

class SynthDevice;

namespace {
constexpr int fade_step_ms = 50;
constexpr int sample_rate = 44100;

// One synthesizer for the whole editor: loading the SoundFont takes a while,
// so it is loaded once, in the background. Only one player plays MIDI at a time.
struct Synth {
	fluid_settings_t* settings = nullptr;
	fluid_synth_t* synth = nullptr;
	QFutureWatcher<QString>* loader = nullptr;
	bool ready = false;
	QString error;
	AudioPlayer* owner = nullptr;
	// Shared output: creating one per song added seconds before it sounded
	QAudioSink* sink = nullptr;
	SynthDevice* device = nullptr;
};

Synth& synth() {
	static Synth s;
	return s;
}

QString findSoundFont() {
	QStringList candidates;
	QString env = qEnvironmentVariable("EASYRPG_EDITOR_SOUNDFONT");
	if (!env.isEmpty()) {
		candidates << env;
	}
	// Shipped with the editor (AppImage, Flatpak and installs keep it next to bin)
	QDir shipped(QDir(QCoreApplication::applicationDirPath()).filePath("../share/easyrpg-editor/soundfonts"));
	for (const QString& name : shipped.entryList({ "*.sf2", "*.sf3" }, QDir::Files, QDir::Name)) {
		candidates << shipped.filePath(name);
	}
	// FluidR3 from the system: Arch (soundfont-fluid), Debian and Ubuntu (fluid-soundfont-gm)
	candidates << "/usr/share/soundfonts/FluidR3_GM.sf2"
		<< "/usr/share/sounds/sf2/FluidR3_GM.sf2"
		<< "/usr/share/soundfonts/default.sf2"
		<< "/usr/share/sounds/sf2/default-GM.sf2";

	for (const QString& path : candidates) {
		if (QFileInfo(path).isFile()) {
			return QFileInfo(path).absoluteFilePath();
		}
	}
	return {};
}

// Starts loading the SoundFont, unless already loaded or loading
void loadSynth() {
	auto& s = synth();
	if (s.loader) {
		return;
	}

	s.settings = new_fluid_settings();
	fluid_settings_setnum(s.settings, "synth.sample-rate", sample_rate);
	s.synth = new_fluid_synth(s.settings);

	s.loader = new QFutureWatcher<QString>(qApp);
	QObject::connect(s.loader, &QFutureWatcherBase::finished, [] {
		auto& s = synth();
		s.error = s.loader->result();
		s.ready = s.error.isEmpty();
	});
	// Looked up here: applicationDirPath() is not reliable in other threads
	QString path = findSoundFont();
	s.loader->setFuture(QtConcurrent::run([synth = s.synth, path]() -> QString {
		if (path.isEmpty()) {
			return QCoreApplication::translate("AudioPlayer",
				"No SoundFont found to play MIDI. Install FluidR3 (soundfont-fluid or fluid-soundfont-gm), or set EASYRPG_EDITOR_SOUNDFONT to a .sf2 or .sf3 file.");
		}
		if (fluid_synth_sfload(synth, QFile::encodeName(path).constData(), 1) == FLUID_FAILED) {
			return QCoreApplication::translate("AudioPlayer", "Cannot load the SoundFont %1").arg(path);
		}
		return {};
	}));
}
}

// Renders the synthesizer for the audio output, as 16 bit stereo
class SynthDevice : public QIODevice {
public:
	explicit SynthDevice(QObject* parent) : QIODevice(parent) {
		open(QIODevice::ReadOnly);
	}

	void setBalance(int balance) {
		// 50 is centered, the other side is lowered towards 0 and 100
		m_left = std::min(1.0f, (100 - balance) / 50.0f);
		m_right = std::min(1.0f, balance / 50.0f);
	}

	bool isSequential() const override { return true; }
	qint64 bytesAvailable() const override { return 1 << 16; }

protected:
	qint64 readData(char* data, qint64 max_size) override {
		int frames = static_cast<int>(max_size / 4);
		auto* samples = reinterpret_cast<int16_t*>(data);
		fluid_synth_write_s16(synth().synth, frames, samples, 0, 2, samples, 1, 2);
		if (m_left < 1.0f || m_right < 1.0f) {
			for (int i = 0; i < frames; ++i) {
				samples[i * 2] = static_cast<int16_t>(samples[i * 2] * m_left);
				samples[i * 2 + 1] = static_cast<int16_t>(samples[i * 2 + 1] * m_right);
			}
		}
		return frames * 4;
	}

	qint64 writeData(const char*, qint64) override { return -1; }

private:
	float m_left = 1.0f;
	float m_right = 1.0f;
};

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent) {
	m_output = new QAudioOutput(this);
	m_player = new QMediaPlayer(this);
	m_player->setAudioOutput(m_output);
	m_player->setLoops(QMediaPlayer::Infinite);

	connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString& message) {
		emit error(message);
	});

	m_fade = new QTimer(this);
	m_fade->setInterval(fade_step_ms);
}

AudioPlayer::~AudioPlayer() {
	stop();
}

void AudioPlayer::setLooping(bool looping) {
	m_looping = looping;
	m_player->setLoops(looping ? QMediaPlayer::Infinite : QMediaPlayer::Once);
}

bool AudioPlayer::isMidi(const QString& path) {
	QString suffix = QFileInfo(path).suffix().toLower();
	return suffix == "mid" || suffix == "midi";
}

void AudioPlayer::play(const QString& path, int volume, int tempo, int fade_in) {
	stop();

	m_volume = std::clamp(volume, 0, 100) / 100.0f;
	m_tempo = std::clamp(tempo, 50, 150);

	if (isMidi(path)) {
		m_midi = true;
		startFade(fade_in);
		playMidi(path);
		return;
	}

	m_midi = false;
	m_player->setSource(QUrl::fromLocalFile(path));
	m_player->setPlaybackRate(m_tempo / 100.0);
	startFade(fade_in);
	m_player->play();
}

void AudioPlayer::stop() {
	m_fade->stop();
	m_player->stop();
	m_player->setSource(QUrl());
	stopMidi();
}

void AudioPlayer::startFade(int fade_in) {
	m_fade->disconnect();
	if (fade_in <= 0) {
		applyVolume(m_volume);
		return;
	}

	applyVolume(0.0f);
	float step = m_volume * fade_step_ms / fade_in;
	connect(m_fade, &QTimer::timeout, this, [this, step] {
		applyVolume(std::min(m_current_volume + step, m_volume));
		if (m_current_volume >= m_volume) {
			m_fade->stop();
		}
	});
	m_fade->start();
}

void AudioPlayer::applyVolume(float volume) {
	m_current_volume = volume;
	m_output->setVolume(volume);
	if (synth().owner == this && synth().sink) {
		synth().sink->setVolume(volume);
	}
}

void AudioPlayer::setVolume(int volume) {
	m_volume = std::clamp(volume, 0, 100) / 100.0f;
	if (!m_fade->isActive()) {
		applyVolume(m_volume);
	}
}

void AudioPlayer::setTempo(int tempo) {
	m_tempo = std::clamp(tempo, 50, 150);
	m_player->setPlaybackRate(m_tempo / 100.0);
	if (m_midi_player) {
		fluid_player_set_tempo(static_cast<fluid_player_t*>(m_midi_player), FLUID_PLAYER_TEMPO_INTERNAL, m_tempo / 100.0);
	}
}

void AudioPlayer::setBalance(int balance) {
	m_balance = std::clamp(balance, 0, 100);
	if (synth().owner == this && synth().device) {
		synth().device->setBalance(m_balance);
	}
}

void AudioPlayer::playMidi(const QString& path) {
	m_pending_midi = path;

	auto& s = synth();
	if (s.ready) {
		startMidi();
		return;
	}
	if (!s.error.isEmpty()) {
		m_pending_midi.clear();
		emit error(s.error);
		return;
	}

	// Still loading: play once the SoundFont is ready, unless stopped meanwhile
	loadSynth();
	connect(s.loader, &QFutureWatcherBase::finished, this, [this] {
		if (m_pending_midi.isEmpty()) {
			return;
		}
		if (synth().ready) {
			startMidi();
		} else {
			m_pending_midi.clear();
			emit error(synth().error);
		}
	}, Qt::SingleShotConnection);
}

void AudioPlayer::startMidi() {
	auto& s = synth();
	if (s.owner && s.owner != this) {
		s.owner->stopMidi();
	}
	s.owner = this;

	QString path = m_pending_midi;
	m_pending_midi.clear();

	// Forget the instruments and controllers of the previous song
	fluid_synth_system_reset(s.synth);

	auto* player = new_fluid_player(s.synth);
	if (fluid_player_add(player, QFile::encodeName(path).constData()) == FLUID_FAILED) {
		delete_fluid_player(player);
		s.owner = nullptr;
		emit error(tr("Cannot play %1").arg(QFileInfo(path).fileName()));
		return;
	}
	fluid_player_set_loop(player, m_looping ? -1 : 1);
	fluid_player_set_tempo(player, FLUID_PLAYER_TEMPO_INTERNAL, m_tempo / 100.0);
	fluid_player_play(player);
	m_midi_player = player;

	if (!s.sink) {
		QAudioFormat format;
		format.setSampleRate(sample_rate);
		format.setChannelCount(2);
		format.setSampleFormat(QAudioFormat::Int16);
		s.device = new SynthDevice(qApp);
		s.sink = new QAudioSink(format, qApp);
		// A small buffer, so volume changes and stops are heard right away
		s.sink->setBufferSize(format.bytesForDuration(100'000));
	}
	s.device->setBalance(m_balance);
	s.sink->setVolume(m_current_volume);
	// Kept running between songs, rendering silence: restarting it adds a delay
	if (s.sink->state() == QAudio::StoppedState) {
		s.sink->start(s.device);
	}
}

void AudioPlayer::stopMidi() {
	m_pending_midi.clear();

	auto& s = synth();
	if (m_midi_player) {
		auto* player = static_cast<fluid_player_t*>(m_midi_player);
		fluid_player_stop(player);
		delete_fluid_player(player);
		m_midi_player = nullptr;
		fluid_synth_all_sounds_off(s.synth, -1);
	}
	if (s.owner == this) {
		s.owner = nullptr;
	}
}
