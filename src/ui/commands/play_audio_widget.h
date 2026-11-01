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

#include "ui/event/event_command_base_widget.h"

namespace Ui {
class PlayAudioWidget;
}

class QSpinBox;

/**
 * Play BGM and Play Sound. The string is the file name, the parameters are
 * fade-in time (in ms, only for BGM), volume, tempo and balance.
 */
class PlayAudioWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	PlayAudioWidget(ProjectData& project, QWidget *parent, bool music, bool system = false);
	~PlayAudioWidget() override;

	void setData(lcf::rpg::EventCommand* cmd) override;

private:
	void bindSpin(QSpinBox* spin, int index, int factor = 1);
	void pickFile();

	Ui::PlayAudioWidget *ui;
	bool m_music;
	// Change System BGM and SFX: the first parameter is the music or sound effect changed
	bool m_system;
};

class PlayBgmWidget : public PlayAudioWidget
{
	Q_OBJECT

public:
	PlayBgmWidget(ProjectData& project, QWidget *parent) : PlayAudioWidget(project, parent, true) {}
};

class PlaySoundWidget : public PlayAudioWidget
{
	Q_OBJECT

public:
	PlaySoundWidget(ProjectData& project, QWidget *parent) : PlayAudioWidget(project, parent, false) {}
};

class ChangeSystemBgmWidget : public PlayAudioWidget
{
	Q_OBJECT

public:
	ChangeSystemBgmWidget(ProjectData& project, QWidget *parent) : PlayAudioWidget(project, parent, true, true) {}
};

class ChangeSystemSfxWidget : public PlayAudioWidget
{
	Q_OBJECT

public:
	ChangeSystemSfxWidget(ProjectData& project, QWidget *parent) : PlayAudioWidget(project, parent, false, true) {}
};
