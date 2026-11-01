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

#include <QWidget>

#include "picker_child_widget.h"

namespace Ui {
class PickerAudioWidget;
}

namespace lcf::rpg {
	class Music;
	class Sound;
}

class AudioPlayer;

class PickerAudioWidget : public PickerChildWidget {
	Q_OBJECT

public:
	enum class Type {
		Music,
		Sound
	};

	explicit PickerAudioWidget(const lcf::rpg::Music& music, QWidget *parent = nullptr);
	explicit PickerAudioWidget(const lcf::rpg::Sound& sound, QWidget *parent = nullptr);
	~PickerAudioWidget();

	void fileChanged(const QString&) override;
	void fileActivated() override;

	int fadeInTime() const;
	int volume() const;
	int tempo() const;
	int balance() const;

private:
	void setupPlayback();
	void updateSoundFont();

	Ui::PickerAudioWidget *ui;
	AudioPlayer* m_player = nullptr;

	Type m_type;
	QString m_filename;

private slots:
	void on_sliderFadeIn_valueChanged(int value);
	void on_sliderVolume_valueChanged(int value);
	void on_sliderTempo_valueChanged(int value);
	void on_sliderBalance_valueChanged(int value);
	void on_pushPlay_clicked();
	void on_pushStop_clicked();
	void on_pushSoundFont_clicked();
	void on_pushSoundFontAuto_clicked();
};
