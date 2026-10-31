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

#pragma once

#include <QList>
#include <QObject>
#include <QString>

class QAudioOutput;
class QMediaPlayer;
class QProcess;
class QTimer;

/**
 * Plays music and sounds for previews.
 *
 * Audio files are played with Qt Multimedia. Like EasyRPG Player, MIDI files
 * are not synthesized here: they are sent to an external synthesizer, such as
 * FluidSynth, through the ALSA sequencer using aplaymidi.
 */
class AudioPlayer : public QObject {
	Q_OBJECT

public:
	explicit AudioPlayer(QObject* parent = nullptr);
	~AudioPlayer() override;

	/**
	 * Starts playing a file, stopping the previous one.
	 *
	 * @param path File to play
	 * @param volume Volume in percent (0 - 100)
	 * @param tempo Tempo in percent (50 - 150), not supported for MIDI
	 * @param fade_in Fade in time in milliseconds, not supported for MIDI
	 */
	void play(const QString& path, int volume, int tempo, int fade_in);

	void stop();

	/** Applies a new volume to the current audio file */
	void setVolume(int volume);

	/** Applies a new tempo to the current audio file */
	void setTempo(int tempo);

	/** Whether to start over when the file ends, true by default */
	void setLooping(bool looping);

	static bool isMidi(const QString& path);

signals:
	/** Reports why a file cannot be played */
	void error(const QString& message);

private:
	void playMidi(const QString& path);
	void startPendingMidi();
	void startMidi();
	void startMidiProcess();
	void stopMidi();

	QMediaPlayer* m_player = nullptr;
	QAudioOutput* m_output = nullptr;
	QTimer* m_fade = nullptr;
	float m_volume = 1.0f;

	QProcess* m_midi = nullptr;
	// Songs turning their notes off, the next song waits for them
	QList<QProcess*> m_stopping;
	QProcess* m_port_lookup = nullptr;
	QString m_pending_midi;
	QString m_midi_port;
	QString m_midi_path;
	QString m_notes_off_path;
	bool m_looping = true;
};
