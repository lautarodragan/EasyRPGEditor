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

#include <lcf/rpg/moveroute.h>
#include <QDialog>

class MoveRouteEditor;
class ProjectData;
class QCheckBox;

/**
 * Edits the custom movement pattern of an event page ("Define Pattern").
 * The route is written back on OK.
 */
class MoveRouteDialog : public QDialog
{
	Q_OBJECT

public:
	MoveRouteDialog(ProjectData& project, lcf::rpg::MoveRoute& route, QWidget* parent = nullptr);

	void accept() override;

private:
	lcf::rpg::MoveRoute& m_route;
	MoveRouteEditor* m_editor = nullptr;
	QCheckBox* m_repeat = nullptr;
	QCheckBox* m_skip = nullptr;
};
