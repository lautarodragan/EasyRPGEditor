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
#include <QFile>
#include <QTimer>
#include <QUrl>
#include <algorithm>

#ifdef Q_OS_LINUX
#include <csignal>
#include <sys/prctl.h>
#include <unistd.h>
#endif

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

	// A QProcess kills its process when deleted, which would skip turning the
	// notes off. Closing is the only place where waiting for that is fine.
	for (QProcess* midi : m_stopping) {
		midi->disconnect();
		if (!midi->waitForFinished(3000)) {
			midi->kill();
			midi->waitForFinished(1000);
		}
	}
	if (m_port_lookup) {
		m_port_lookup->disconnect();
		m_port_lookup->kill();
		m_port_lookup->waitForFinished(1000);
	}
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
	m_pending_midi.clear();
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

	// Written once and kept, the shell below may need it after the editor is gone
	m_notes_off_path = QDir::temp().filePath("easyrpg-editor-all-notes-off.mid");
	QFile notes_off(m_notes_off_path);
	if (!notes_off.exists() && notes_off.open(QIODevice::WriteOnly)) {
		notes_off.write(allNotesOffMidi());
	}

	// Nothing here waits: the song starts once the previous one turned its
	// notes off and the synthesizer port is known. A newer request replaces it.
	m_pending_midi = path;
	startPendingMidi();
}

void AudioPlayer::startPendingMidi() {
	if (m_pending_midi.isEmpty() || !m_stopping.isEmpty() || m_port_lookup) {
		return;
	}

	// Output of "aplaymidi -l":
	//  Port    Client name                      Port name
	//  14:0    Midi Through                     Midi Through Port-0
	// 128:0    FLUID Synth (1234)               Synth input port (1234:0)
	m_port_lookup = new QProcess(this);
	connect(m_port_lookup, &QProcess::finished, this, [this] {
		QString output = QString::fromLocal8Bit(m_port_lookup->readAllStandardOutput());
		m_port_lookup->deleteLater();
		m_port_lookup = nullptr;

		m_midi_port.clear();
		static const QRegularExpression port_line(R"(^\s*(\d+:\d+)\s+(.*)$)");
		for (const QString& line : output.split('\n')) {
			auto match = port_line.match(line);
			if (match.hasMatch() && !match.captured(2).contains("Midi Through")) {
				m_midi_port = match.captured(1);
				break;
			}
		}

		if (m_pending_midi.isEmpty()) {
			// Stopped meanwhile
			return;
		}
		if (m_midi_port.isEmpty()) {
			m_pending_midi.clear();
			emit error(tr("No MIDI synthesizer found. Start one, for example: fluidsynth -a pipewire -m alsa_seq -si <soundfont.sf2>"));
			return;
		}
		// A song may have been stopped while looking up the port
		if (m_stopping.isEmpty()) {
			startMidi();
		}
	});
	m_port_lookup->start("aplaymidi", { "-l" });
}

void AudioPlayer::startMidi() {
	m_midi_path = m_pending_midi;
	m_pending_midi.clear();
	m_midi = new QProcess(this);

#ifdef Q_OS_LINUX
	m_midi->setChildProcessModifier([] {
		// Ctrl+C in a terminal signals the whole process group: the editor
		// quits normally, but the shell must not be killed directly
		setsid();
		// Stop the music when the editor ends, even when it crashes
		prctl(PR_SET_PDEATHSIG, SIGTERM);
	});
#endif

	// Loop like the game does: start over when the song ends
	connect(m_midi, &QProcess::finished, this, [this](int exit_code, QProcess::ExitStatus status) {
		if (m_midi && m_looping && status == QProcess::NormalExit && exit_code == 0) {
			startMidiProcess();
		}
	});
	startMidiProcess();
}

void AudioPlayer::startMidiProcess() {
	// aplaymidi does not handle signals: killing it leaves notes playing in
	// the synthesizer. This shell stops it on SIGTERM and turns all notes off.
	static const QString script = QStringLiteral(
		"trap 'kill $pid 2>/dev/null; wait $pid; aplaymidi -p \"$1\" \"$3\"; exit 143' TERM\n"
		"aplaymidi -p \"$1\" \"$2\" & pid=$!\n"
		"wait $pid\n");
	m_midi->start("sh", { "-c", script, "sh", m_midi_port, m_midi_path, m_notes_off_path });
}

void AudioPlayer::stopMidi() {
	if (!m_midi) {
		return;
	}

	// SIGTERM, so the shell turns all notes off. This takes a moment, the next
	// song starts once it is done so it does not get its notes turned off.
	QProcess* midi = m_midi;
	m_midi = nullptr;
	midi->disconnect();
	m_stopping.append(midi);
	connect(midi, &QProcess::finished, this, [this, midi] {
		m_stopping.removeOne(midi);
		midi->deleteLater();
		startPendingMidi();
	});
	midi->terminate();

	QTimer::singleShot(3000, midi, [midi] {
		if (midi->state() != QProcess::NotRunning) {
			midi->kill();
		}
	});
}
