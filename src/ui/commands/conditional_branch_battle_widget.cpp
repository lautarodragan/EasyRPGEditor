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

#include "conditional_branch_battle_widget.h"
#include "command_bindings.h"
#include "common/dbstring.h"
#include "model/event_command_list.h"
#include "model/event_command_templates.h"
#include "model/project_data.h"
#include "ui/common/rpg_combobox.h"
#include "ui/common/rpg_spinbox.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <algorithm>

using namespace CommandBindings;

namespace {
using Cmd = lcf::rpg::EventCommand::Code;

// Parameters: type, up to four values depending on the type, has else branch
constexpr int param_else = 5;

// Condition types, as Game_Interpreter_Battle::CommandConditionalBranchBattle of the Player reads them
enum Type {
	TypeSwitch = 0, // switch, 0 ON / 1 OFF
	TypeVariable = 1, // variable, 0 constant / 1 variable, value, comparison
	TypeActorCanAct = 2, // actor
	TypeMonsterCanAct = 3, // troop member from 0
	TypeMonsterTarget = 4, // troop member from 0, RPG Maker 2003
	TypeActorCommand = 5, // actor, battle command, RPG Maker 2003
};

int idOf(RpgComboBoxBase* combo) {
	return combo->comboBox()->currentIndex() + 1;
}

void selectId(RpgComboBoxBase* combo, int id) {
	combo->comboBox()->setCurrentIndex(id >= 1 && id <= combo->comboBox()->count() ? id - 1 : -1);
}

QHBoxLayout* row(std::initializer_list<QWidget*> widgets) {
	auto* layout = new QHBoxLayout;
	for (auto* widget : widgets) {
		layout->addWidget(widget, 1);
	}
	return layout;
}
}

ConditionalBranchBattleWidget::ConditionalBranchBattleWidget(ProjectData& project, QWidget* parent) :
	BlockCommandWidget(project, parent) {

	const bool is_2k3 = project.database().system.ldb_id == 2003;

	auto* layout = new QVBoxLayout(this);
	auto* grid = new QGridLayout;
	grid->setColumnStretch(1, 1);
	layout->addLayout(grid);
	m_type_group = new QButtonGroup(this);

	auto add_row = [&](int type, const QString& text, QLayout* controls) {
		auto* radio = new QRadioButton(text);
		m_type_group->addButton(radio, type);
		grid->addWidget(radio, type, 0, Qt::AlignTop);
		grid->addLayout(controls, type, 1);
		return radio;
	};

	auto* sw = new SwitchRpgComboBox(this);
	sw->makeModel(project);
	m_switch = sw;
	m_switch_state = new QComboBox;
	m_switch_state->addItems({ tr("is ON"), tr("is OFF") });
	add_row(TypeSwitch, tr("Switch"), row({ m_switch, m_switch_state }));

	auto* variable = new VariableRpgComboBox(this);
	variable->makeModel(project);
	m_variable = variable;
	m_variable_op = new QComboBox;
	m_variable_op->addItems({ tr("equal to"), tr("greater than or equal to"), tr("less than or equal to"),
		tr("greater than"), tr("less than"), tr("not equal to") });
	m_operand_constant = new QRadioButton(tr("Constant"));
	m_value = new RpgSpinBox;
	m_value->setRange(-9999999, 9999999);
	m_operand_variable = new QRadioButton(tr("Variable"));
	auto* value_variable = new VariableRpgComboBox(this);
	value_variable->makeModel(project);
	m_value_variable = value_variable;
	auto* operand_group = new QButtonGroup(this);
	operand_group->addButton(m_operand_constant);
	operand_group->addButton(m_operand_variable);
	auto* variable_rows = new QVBoxLayout;
	variable_rows->addLayout(row({ m_variable, m_variable_op }));
	auto* operand_row = new QHBoxLayout;
	operand_row->addWidget(m_operand_constant);
	operand_row->addWidget(m_value, 1);
	operand_row->addWidget(m_operand_variable);
	operand_row->addWidget(m_value_variable, 2);
	variable_rows->addLayout(operand_row);
	add_row(TypeVariable, tr("Variable"), variable_rows);

	auto* actor_act = new ActorRpgComboBox(this);
	actor_act->makeModel(project);
	m_actor_act = actor_act;
	add_row(TypeActorCanAct, tr("Actor can act"), row({ m_actor_act }));

	m_monster_act = new QComboBox;
	addMonsters(m_monster_act);
	add_row(TypeMonsterCanAct, tr("Monster can act"), row({ m_monster_act }));

	m_monster_target = new QComboBox;
	addMonsters(m_monster_target);
	auto* target_radio = add_row(TypeMonsterTarget, tr("Monster is targeted"), row({ m_monster_target }));

	auto* actor_command = new ActorRpgComboBox(this);
	actor_command->makeModel(project);
	m_actor_command = actor_command;
	m_command = new QComboBox;
	for (const auto& command : project.database().battlecommands.commands) {
		m_command->addItem(QStringLiteral("%1: %2").arg(command.ID, 4, 10, QChar('0')).arg(ToQString(command.name)), command.ID);
	}
	auto* command_radio = add_row(TypeActorCommand, tr("Actor uses command"), row({ m_actor_command, m_command }));

	// RPG Maker 2000 has only the first four conditions
	m_rpg2k3_widgets = { target_radio, m_monster_target, command_radio, m_actor_command, m_command };
	for (auto* widget : m_rpg2k3_widgets) {
		widget->setVisible(is_2k3);
	}

	m_else = new QCheckBox(tr("Execute custom handler when the condition is not met (else branch)"));
	layout->addWidget(m_else);
	m_unsupported = new QLabel;
	m_unsupported->setWordWrap(true);
	m_unsupported->hide();
	layout->addWidget(m_unsupported);
	layout->addWidget(warningLabel());
	layout->addStretch(1);
}

void ConditionalBranchBattleWidget::load() {
	const int type = param(m_edit, 0);
	const int p1 = param(m_edit, 1);
	const int p2 = param(m_edit, 2);
	const int p3 = param(m_edit, 3);
	const int p4 = param(m_edit, 4);

	// The widgets of the other types show defaults
	m_operand_constant->setChecked(true);
	selectData(m_monster_act, 0);
	selectData(m_monster_target, 0);

	switch (type) {
		case TypeSwitch:
			selectId(m_switch, p1);
			m_switch_state->setCurrentIndex(p2 == 0 ? 0 : 1);
			break;
		case TypeVariable:
			selectId(m_variable, p1);
			if (p2 == 0) {
				m_value->setValue(p3);
			} else {
				m_operand_variable->setChecked(true);
				selectId(m_value_variable, p3);
			}
			m_variable_op->setCurrentIndex(p4 >= 0 && p4 < m_variable_op->count() ? p4 : 0);
			break;
		case TypeActorCanAct:
			selectId(m_actor_act, p1);
			break;
		case TypeMonsterCanAct:
			selectData(m_monster_act, p1);
			break;
		case TypeMonsterTarget:
			selectData(m_monster_target, p1);
			break;
		case TypeActorCommand:
			selectId(m_actor_command, p1);
			selectData(m_command, p2);
			break;
		default:
			break;
	}

	if (auto* button = m_type_group->button(type)) {
		button->setChecked(true);
		if (type >= TypeMonsterTarget) {
			// Show RPG Maker 2003 conditions used by a RPG Maker 2000 project
			for (auto* widget : m_rpg2k3_widgets) {
				widget->show();
			}
		}
	} else {
		m_unsupported->setText(tr("This editor does not support the condition type %1. "
			"Its parameters are kept unless another condition is selected.").arg(type));
		m_unsupported->show();
	}

	// The else branch follows the block, or the parameter when the block is missing
	const auto block = EventCommandTemplates::parseBlock(m_commands->commands(), m_commands->index());
	if (block.end_line != -1) {
		m_else->setChecked(!block.branches.empty());
	} else {
		m_else->setChecked(param(m_edit, param_else) != 0);
		// Give the branch its block
		branchesChanged();
	}

	auto changed = [this] {
		updateEnabled();
		writeParameters();
	};
	connect(m_type_group, &QButtonGroup::idToggled, this, [=](int, bool checked) {
		if (checked) {
			changed();
		}
	});
	connect(m_operand_constant, &QRadioButton::toggled, this, changed);
	for (auto* combo : { m_switch, m_variable, m_value_variable, m_actor_act, m_actor_command }) {
		connect(combo->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, changed);
	}
	for (auto* combo : { m_switch_state, m_variable_op, m_monster_act, m_monster_target, m_command }) {
		connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, changed);
	}
	connect(m_value, qOverload<int>(&QSpinBox::valueChanged), this, changed);
	connect(m_else, &QCheckBox::toggled, this, [this](bool checked) {
		setParam(m_edit, param_else, checked ? 1 : 0);
		branchesChanged();
	});

	updateEnabled();
}

void ConditionalBranchBattleWidget::writeParameters() {
	const int type = m_type_group->checkedId();
	if (type == -1) {
		return;
	}

	// RPG Maker writes the type, four values and the else flag
	setParam(m_edit, param_else, param(m_edit, param_else));
	auto& p = m_edit.parameters;

	// RPG Maker stores 0 in the parameters a condition does not use
	if (p[0] != type) {
		p[0] = type;
		std::fill(p.begin() + 1, p.begin() + param_else, 0);
	}

	switch (type) {
		case TypeSwitch:
			p[1] = idOf(m_switch);
			p[2] = m_switch_state->currentIndex();
			break;
		case TypeVariable:
			p[1] = idOf(m_variable);
			if (m_operand_constant->isChecked()) {
				p[2] = 0;
				p[3] = m_value->value();
			} else {
				p[2] = 1;
				p[3] = idOf(m_value_variable);
			}
			p[4] = m_variable_op->currentIndex();
			break;
		case TypeActorCanAct:
			p[1] = idOf(m_actor_act);
			break;
		case TypeMonsterCanAct:
			p[1] = m_monster_act->currentData().toInt();
			break;
		case TypeMonsterTarget:
			p[1] = m_monster_target->currentData().toInt();
			break;
		case TypeActorCommand:
			p[1] = idOf(m_actor_command);
			p[2] = m_command->currentData().toInt();
			break;
		default:
			break;
	}
}

void ConditionalBranchBattleWidget::updateEnabled() {
	const int type = m_type_group->checkedId();

	auto enable = [](std::initializer_list<QWidget*> widgets, bool enabled) {
		for (auto* widget : widgets) {
			widget->setEnabled(enabled);
		}
	};

	enable({ m_switch, m_switch_state }, type == TypeSwitch);
	enable({ m_variable, m_variable_op, m_operand_constant, m_operand_variable }, type == TypeVariable);
	m_value->setEnabled(type == TypeVariable && m_operand_constant->isChecked());
	m_value_variable->setEnabled(type == TypeVariable && m_operand_variable->isChecked());
	m_actor_act->setEnabled(type == TypeActorCanAct);
	m_monster_act->setEnabled(type == TypeMonsterCanAct);
	m_monster_target->setEnabled(type == TypeMonsterTarget);
	enable({ m_actor_command, m_command }, type == TypeActorCommand);
}
