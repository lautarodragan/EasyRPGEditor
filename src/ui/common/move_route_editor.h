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

#include <lcf/rpg/movecommand.h>
#include <QWidget>
#include <vector>

class ProjectData;
class QGridLayout;
class QGroupBox;
class QListWidget;
class QPushButton;
class QVBoxLayout;

/**
 * Edits a move route: the list of move commands and the buttons to add them,
 * as in RPG Maker. Used by the Move Event command and by the custom movement
 * pattern of event pages.
 */
class MoveRouteEditor : public QWidget
{
	Q_OBJECT

public:
	explicit MoveRouteEditor(QWidget* parent = nullptr);

	void setProject(ProjectData& project);

	/**
	 * @param route Commands to edit
	 * @param editable false to only show them, e.g. when they could not be read
	 */
	void setRoute(const std::vector<lcf::rpg::MoveCommand>& route, bool editable = true);
	const std::vector<lcf::rpg::MoveCommand>& route() const;

	/** Adds a widget below the list of commands, e.g. route options */
	void addBelowList(QWidget* widget);

signals:
	/** The user added, edited or deleted a command */
	void routeChanged();

private:
	void addCommandButtons();
	void refreshRoute(int select_row);
	void insertCommand(int code);
	void editCommand(int row);
	bool editOptions(lcf::rpg::MoveCommand& cmd);
	bool editSwitch(lcf::rpg::MoveCommand& cmd);
	bool editGraphic(lcf::rpg::MoveCommand& cmd);
	bool editSound(lcf::rpg::MoveCommand& cmd);
	void deleteCommand();
	void deleteAllCommands();

	ProjectData* m_project = nullptr;
	std::vector<lcf::rpg::MoveCommand> m_route;
	bool m_editable = true;

	QListWidget* m_list = nullptr;
	QPushButton* m_delete = nullptr;
	QPushButton* m_deleteAll = nullptr;
	QVBoxLayout* m_left = nullptr;
	QGroupBox* m_commands = nullptr;
	QGridLayout* m_commandsLayout = nullptr;
};
