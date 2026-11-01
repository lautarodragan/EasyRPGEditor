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

#include "block_command_widget.h"

class QButtonGroup;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QToolButton;
class QWidget;

/**
 * Edits an Enemy Encounter: troop, battle background, escape and defeat
 * handling (with their handler branches) and the special conditions.
 */
class EnemyEncounterWidget : public BlockCommandWidget
{
	Q_OBJECT

public:
	EnemyEncounterWidget(ProjectData& project, QWidget* parent);

protected:
	void load() override;

private:
	void updateEnabled();

	bool m_2k3;
	QButtonGroup* m_troop_group;
	QWidget* m_troop;
	QWidget* m_troop_variable;
	QButtonGroup* m_bg_group;
	QLineEdit* m_bg_name;
	QToolButton* m_bg_pick;
	QComboBox* m_formation;
	QWidget* m_terrain;
	QButtonGroup* m_escape_group;
	QButtonGroup* m_defeat_group;
	QCheckBox* m_first_strike;
	QComboBox* m_condition;
};
