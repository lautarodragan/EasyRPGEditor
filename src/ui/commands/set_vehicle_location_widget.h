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
class SetVehicleLocationWidget;
}

/**
 * Editor of Set Vehicle Location: a map and a location, fixed or from variables.
 */
class SetVehicleLocationWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	explicit SetVehicleLocationWidget(ProjectData& project, QWidget *parent);
	~SetVehicleLocationWidget() override;

	void setData(lcf::rpg::EventCommand* cmd) override;

private:
	void writeLocation();
	void updateLocation();
	void updateEnabled();

	int m_map = 0;
	int m_x = 0;
	int m_y = 0;

	Ui::SetVehicleLocationWidget *ui;
};
