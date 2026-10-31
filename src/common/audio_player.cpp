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
#include <QAudioOutput>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>
#include <algorithm>

namespace {
constexpr int fade_step_ms = 50;

// A MIDI file that sends "All Sound Off" and "All Notes Off" on every channel.
// Killing aplaymidi leaves notes playing in the synthesizer otherwise.
QByteArray allNotesOffMidi() {
	QByteArray track;
	for (char channel = 0; channel < 16; ++channel) {
		for (char controller : { char(120), char(123) }) {
			track.append(char(0));
			track.append(char(0xB0 | channel));
			track.append(controller);
			track.append(char(0));
		}
	}
	// End of track
	track.append(QByteArray("\x00\xFF\x2F\x00", 4));

	QByteArray midi("MThd\x00\x00\x00\x06\x00\x00\x00\x01\x00\x60", 14);
	midi.append("MTrk");
	quint32 size = static_cast<quint32>(track.size());
	for (int shift = 24; shift >= 0; shift -= 8) {
		midi.append(char((size >> shift) & 0xFF));
	}
	midi.append(track);
	return midi;
}
}

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

	if (isMidi(path)) {
		playMidi(path);
		return;
	}

	m_volume = std::clamp(volume, 0, 100) / 100.0f;
	m_player->setSource(QUrl::fromLocalFile(path));
	m_player->setPlaybackRate(std::clamp(tempo, 50, 150) / 100.0);

	m_fade->disconnect();
	if (fade_in > 0) {
		m_output->setVolume(0.0f);
		float step = m_volume * fade_step_ms / fade_in;
		connect(m_fade, &QTimer::timeout, this, [this, step] {
			float volume = std::min(m_output->volume() + step, m_volume);
			m_output->setVolume(volume);
			if (volume >= m_volume) {
				m_fade->stop();
			}
		});
		m_fade->start();
	} else {
		m_output->setVolume(m_volume);
	}

	m_player->play();
}

void AudioPlayer::stop() {
	m_fade->stop();
	m_player->stop();
	m_player->setSource(QUrl());
	stopMidi();
}

void AudioPlayer::setVolume(int volume) {
	m_volume = std::clamp(volume, 0, 100) / 100.0f;
	if (!m_fade->isActive()) {
		m_output->setVolume(m_volume);
	}
}

void AudioPlayer::setTempo(int tempo) {
	m_player->setPlaybackRate(std::clamp(tempo, 50, 150) / 100.0);
}

void AudioPlayer::playMidi(const QString& path) {
	if (QStandardPaths::findExecutable("aplaymidi").isEmpty()) {
		emit error(tr("Playing MIDI needs aplaymidi (from alsa-utils) and a MIDI synthesizer such as FluidSynth."));
		return;
	}

	m_midi_port = findMidiPort();
	if (m_midi_port.isEmpty()) {
		emit error(tr("No MIDI synthesizer found. Start one, for example: fluidsynth -a pipewire -m alsa_seq -si <soundfont.sf2>"));
		return;
	}

	m_midi_path = path;
	m_midi = new QProcess(this);
	// Loop like the game does: start over when the song ends
	connect(m_midi, &QProcess::finished, this, [this](int, QProcess::ExitStatus status) {
		if (m_midi && m_looping && status == QProcess::NormalExit) {
			m_midi->start("aplaymidi", { "-p", m_midi_port, m_midi_path });
		}
	});
	m_midi->start("aplaymidi", { "-p", m_midi_port, path });
}

void AudioPlayer::stopMidi() {
	if (!m_midi) {
		return;
	}

	QProcess* midi = m_midi;
	m_midi = nullptr;
	midi->disconnect();
	midi->kill();
	midi->waitForFinished(1000);
	midi->deleteLater();

	QTemporaryFile notes_off(QDir::tempPath() + "/easyrpg-editor-XXXXXX.mid");
	if (notes_off.open()) {
		notes_off.write(allNotesOffMidi());
		notes_off.flush();
		QProcess::execute("aplaymidi", { "-p", m_midi_port, notes_off.fileName() });
	}
}

QString AudioPlayer::findMidiPort() {
	// Output of "aplaymidi -l":
	//  Port    Client name                      Port name
	//  14:0    Midi Through                     Midi Through Port-0
	// 128:0    FLUID Synth (1234)               Synth input port (1234:0)
	QProcess list;
	list.start("aplaymidi", { "-l" });
	if (!list.waitForFinished(2000)) {
		return {};
	}

	static const QRegularExpression port_line(R"(^\s*(\d+:\d+)\s+(.*)$)");
	for (const QString& line : QString::fromLocal8Bit(list.readAllStandardOutput()).split('\n')) {
		auto match = port_line.match(line);
		if (match.hasMatch() && !match.captured(2).contains("Midi Through")) {
			return match.captured(1);
		}
	}
	return {};
}
