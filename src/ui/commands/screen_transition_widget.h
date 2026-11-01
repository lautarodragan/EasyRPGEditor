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
class ScreenTransitionWidget;
}

/** Erase Screen and Show Screen: a single parameter, the transition */
class ScreenTransitionWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	ScreenTransitionWidget(ProjectData& project, QWidget *parent, bool erase);
	~ScreenTransitionWidget() override;

	void setData(lcf::rpg::EventCommand* cmd) override;

private:
	Ui::ScreenTransitionWidget *ui;
	bool m_erase;
};

class EraseScreenWidget : public ScreenTransitionWidget
{
	Q_OBJECT

public:
	EraseScreenWidget(ProjectData& project, QWidget *parent) : ScreenTransitionWidget(project, parent, true) {}
};

class ShowScreenWidget : public ScreenTransitionWidget
{
	Q_OBJECT

public:
	ShowScreenWidget(ProjectData& project, QWidget *parent) : ScreenTransitionWidget(project, parent, false) {}
};
