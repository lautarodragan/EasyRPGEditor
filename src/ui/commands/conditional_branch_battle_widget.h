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
#include <vector>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QRadioButton;
class QSpinBox;
class RpgComboBoxBase;

/**
 * Edits a Conditional Branch of a troop page, which has its own conditions
 * (actors and monsters that can act, the target of the player, ...).
 * Adds or removes the Else Branch like ConditionalBranchWidget.
 */
class ConditionalBranchBattleWidget : public BlockCommandWidget
{
	Q_OBJECT

public:
	ConditionalBranchBattleWidget(ProjectData& project, QWidget* parent);

protected:
	void load() override;

private:
	void writeParameters();
	void updateEnabled();

	QButtonGroup* m_type_group;
	RpgComboBoxBase* m_switch;
	QComboBox* m_switch_state;
	RpgComboBoxBase* m_variable;
	QComboBox* m_variable_op;
	QRadioButton* m_operand_constant;
	QRadioButton* m_operand_variable;
	QSpinBox* m_value;
	RpgComboBoxBase* m_value_variable;
	RpgComboBoxBase* m_actor_act;
	QComboBox* m_monster_act;
	QComboBox* m_monster_target;
	RpgComboBoxBase* m_actor_command;
	QComboBox* m_command;
	QCheckBox* m_else;
	QLabel* m_unsupported;
	std::vector<QWidget*> m_rpg2k3_widgets;
};
