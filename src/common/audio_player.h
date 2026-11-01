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

#include <QObject>
#include <QString>

class QAudioOutput;
class QMediaPlayer;
class QTimer;

/**
 * Plays music and sounds for previews.
 *
 * Audio files are played with Qt Multimedia. MIDI files are synthesized with
 * FluidSynth inside the editor, using a SoundFont: the one chosen by the user,
 * else the one set in the EASYRPG_EDITOR_SOUNDFONT environment variable,
 * the one shipped with the editor, or FluidR3 as installed by the system.
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
	 * @param tempo Tempo in percent (50 - 150)
	 * @param fade_in Fade in time in milliseconds
	 */
	void play(const QString& path, int volume, int tempo, int fade_in);

	void stop();

	/** Applies a new volume to the current file */
	void setVolume(int volume);

	/** Applies a new tempo to the current file */
	void setTempo(int tempo);

	/** Applies a new balance to the current MIDI file (0: left, 50: center, 100: right) */
	void setBalance(int balance);

	/** Whether to start over when the file ends, true by default */
	void setLooping(bool looping);

	static bool isMidi(const QString& path);

	/** SoundFont chosen by the user, empty to find one automatically */
	static QString chosenSoundFont();

	/** Saves the choice and loads that SoundFont in the background, stopping the MIDI playing */
	static void setSoundFont(const QString& path);

	/** Starts loading the SoundFont in the background, if not done yet */
	static void loadSoundFont();

	static bool isSoundFontLoading();

	/** SoundFont in use, empty while loading or when none could be loaded */
	static QString soundFont();

	/** Why the chosen SoundFont, or any, could not be loaded */
	static QString soundFontError();

signals:
	/** Reports why a file cannot be played */
	void error(const QString& message);

	/** The SoundFont started or finished loading, emitted by every player */
	void soundFontChanged();

private:
	void playMidi(const QString& path);
	void waitForSoundFont();
	void startMidi();
	void stopMidi();
	void startFade(int fade_in);
	void applyVolume(float volume);

	QMediaPlayer* m_player = nullptr;
	QAudioOutput* m_output = nullptr;
	QTimer* m_fade = nullptr;
	float m_volume = 1.0f;
	float m_current_volume = 1.0f;
	int m_tempo = 100;
	bool m_looping = true;

	// MIDI
	void* m_midi_player = nullptr;
	int m_balance = 50;
	QString m_pending_midi;
	bool m_midi = false;
};
