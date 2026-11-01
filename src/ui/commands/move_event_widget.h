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
#include <lcf/rpg/movecommand.h>
#include <string>
#include <vector>

namespace Ui {
class MoveEventWidget;
}

class MoveEventWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	explicit MoveEventWidget(ProjectData& project, QWidget *parent);
	~MoveEventWidget() override;

	void setData(lcf::rpg::EventCommand* cmd) override;

private:
	void addCommandButtons();
	void populateTargets();
	void refreshRoute(int select_row);
	void insertCommand(int code);
	void editCommand(int row);
	bool editOptions(lcf::rpg::MoveCommand& cmd);
	bool editSwitch(lcf::rpg::MoveCommand& cmd);
	bool editGraphic(lcf::rpg::MoveCommand& cmd);
	bool editSound(lcf::rpg::MoveCommand& cmd);
	void deleteCommand();
	void deleteAllCommands();
	void writeRoute();

	Ui::MoveEventWidget *ui;
	std::string m_encoding;
	std::vector<lcf::rpg::MoveCommand> m_route;
	bool m_route_valid = true;
};
