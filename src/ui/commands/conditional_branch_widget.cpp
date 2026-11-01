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

#include "conditional_branch_widget.h"
#include "ui_conditional_branch_widget.h"
#include "ui/common/event_target_combobox.h"
#include "common/dbstring.h"
#include "model/event_command_list.h"
#include "model/project_data.h"
#include <QButtonGroup>
#include <algorithm>
#include <vector>

namespace {
using Cmd = lcf::rpg::EventCommand::Code;

// Parameters: type, four values depending on the type, has else branch
constexpr int param_count = 6;
constexpr int param_else = 5;

// Condition types, as Game_Interpreter::CommandConditionalBranch of the Player reads them
enum Type {
	TypeSwitch = 0,
	TypeVariable = 1,
	TypeTimer1 = 2,
	TypeGold = 3,
	TypeItem = 4,
	TypeActor = 5,
	TypeEvent = 6,
	TypeVehicle = 7,
	TypeActionKey = 8,
	TypeBgmLooped = 9,
	TypeTimer2 = 10, // RPG Maker 2003
	TypeOther = 11, // RPG Maker 2003 v1.11
};

// Checks on an actor (parameter 2 of TypeActor)
enum ActorCheck {
	ActorInParty = 0,
	ActorName = 1,
	ActorLevel = 2,
	ActorHp = 3,
	ActorSkill = 4,
	ActorEquipment = 5,
	ActorState = 6,
};

struct Branch {
	int else_index = -1;
	int end_index = -1;
};

// Finds the Else Branch and End Branch of the Conditional Branch at index.
// The commands inside the branches are indented deeper than the Conditional Branch.
Branch findBranch(const std::vector<lcf::rpg::EventCommand>& commands, int index) {
	Branch branch;
	int indent = commands[index].indent;
	for (int i = index + 1; i < static_cast<int>(commands.size()); ++i) {
		const auto& cmd = commands[i];
		if (cmd.indent < indent) {
			break;
		}
		if (cmd.indent > indent) {
			continue;
		}
		auto code = static_cast<Cmd>(cmd.code);
		if (code == Cmd::ElseBranch && branch.else_index == -1) {
			branch.else_index = i;
			continue;
		}
		if (code == Cmd::EndBranch) {
			branch.end_index = i;
		}
		break;
	}
	if (branch.end_index == -1) {
		branch.else_index = -1;
	}
	return branch;
}

lcf::rpg::EventCommand makeCommand(Cmd code, int indent) {
	lcf::rpg::EventCommand cmd;
	cmd.code = static_cast<int32_t>(code);
	cmd.indent = indent;
	return cmd;
}

// Combo boxes of database entries list IDs starting at 1
int idOf(RpgComboBoxBase* combo) {
	return combo->comboBox()->currentIndex() + 1;
}

void selectId(RpgComboBoxBase* combo, int id) {
	combo->comboBox()->setCurrentIndex(id - 1);
}
}

ConditionalBranchWidget::ConditionalBranchWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ConditionalBranchWidget) {

	ui->setupUi(this);

	ui->comboSwitch->makeModel(project);
	ui->comboVariable->makeModel(project);
	ui->comboVariableOperand->makeModel(project);
	ui->comboItem->makeModel(project);
	ui->comboActor->makeModel(project);
	ui->comboActorSkill->makeModel(project);
	ui->comboActorEquipment->makeModel(project);
	ui->comboActorState->makeModel(project);

	m_type_group = new QButtonGroup(this);
	m_type_group->addButton(ui->radioSwitch, TypeSwitch);
	m_type_group->addButton(ui->radioVariable, TypeVariable);
	m_type_group->addButton(ui->radioTimer1, TypeTimer1);
	m_type_group->addButton(ui->radioGold, TypeGold);
	m_type_group->addButton(ui->radioItem, TypeItem);
	m_type_group->addButton(ui->radioActor, TypeActor);
	m_type_group->addButton(ui->radioEvent, TypeEvent);
	m_type_group->addButton(ui->radioVehicle, TypeVehicle);
	m_type_group->addButton(ui->radioActionKey, TypeActionKey);
	m_type_group->addButton(ui->radioBgmLooped, TypeBgmLooped);
	m_type_group->addButton(ui->radioTimer2, TypeTimer2);
	m_type_group->addButton(ui->radioOther, TypeOther);

	m_actor_group = new QButtonGroup(this);
	m_actor_group->addButton(ui->radioActorInParty, ActorInParty);
	m_actor_group->addButton(ui->radioActorName, ActorName);
	m_actor_group->addButton(ui->radioActorLevel, ActorLevel);
	m_actor_group->addButton(ui->radioActorHp, ActorHp);
	m_actor_group->addButton(ui->radioActorSkill, ActorSkill);
	m_actor_group->addButton(ui->radioActorEquipment, ActorEquipment);
	m_actor_group->addButton(ui->radioActorState, ActorState);

	auto* operand_group = new QButtonGroup(this);
	operand_group->addButton(ui->radioVariableConstant);
	operand_group->addButton(ui->radioVariableVariable);

	// Timer 2 and the v1.11 conditions only exist in RPG Maker 2003
	bool is_2k3 = project.database().system.ldb_id == 2003;
	ui->radioTimer2->setVisible(is_2k3);
	ui->spinTimer2Min->setVisible(is_2k3);
	ui->spinTimer2Sec->setVisible(is_2k3);
	ui->comboTimer2Op->setVisible(is_2k3);
	ui->radioOther->setVisible(is_2k3);
	ui->comboOther->setVisible(is_2k3);
}

ConditionalBranchWidget::~ConditionalBranchWidget() {
	delete ui;
}

void ConditionalBranchWidget::setData(EventCommandList* commands) {
	m_commands = commands;

	// Edit a copy: the command list is shared with the event, Cancel must not change it
	m_edit = commands->command();
	if (m_edit.parameters.size() < param_count) {
		std::vector<int32_t> params(m_edit.parameters.begin(), m_edit.parameters.end());
		params.resize(param_count, 0);
		m_edit.parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	}
	EventCommandBaseWidget::setData(&m_edit);

	loadParameters();
	connectWidgets();
	updateEnabled();
	updateElseWarning();
}

void ConditionalBranchWidget::loadParameters() {
	const auto& p = m_edit.parameters;
	int type = p[0];

	// The widgets of the other types show defaults
	ui->radioVariableConstant->setChecked(true);
	ui->radioActorInParty->setChecked(true);
	ui->spinActorLevel->setValue(1);
	int event_id = EventTargetComboBox::hero;

	switch (type) {
		case TypeSwitch:
			selectId(ui->comboSwitch, p[1]);
			ui->comboSwitchState->setCurrentIndex(p[2] == 0 ? 0 : 1);
			break;
		case TypeVariable:
			selectId(ui->comboVariable, p[1]);
			if (p[2] == 0) {
				ui->spinVariableValue->setValue(p[3]);
			} else {
				ui->radioVariableVariable->setChecked(true);
				selectId(ui->comboVariableOperand, p[3]);
			}
			ui->comboVariableOp->setCurrentIndex(p[4]);
			break;
		case TypeTimer1:
			ui->spinTimer1Min->setValue(p[1] / 60);
			ui->spinTimer1Sec->setValue(p[1] % 60);
			ui->comboTimer1Op->setCurrentIndex(p[2] == 0 ? 0 : 1);
			break;
		case TypeTimer2:
			ui->spinTimer2Min->setValue(p[1] / 60);
			ui->spinTimer2Sec->setValue(p[1] % 60);
			ui->comboTimer2Op->setCurrentIndex(p[2] == 0 ? 0 : 1);
			break;
		case TypeGold:
			ui->spinGold->setValue(p[1]);
			ui->comboGoldOp->setCurrentIndex(p[2] == 0 ? 0 : 1);
			break;
		case TypeItem:
			selectId(ui->comboItem, p[1]);
			ui->comboItemState->setCurrentIndex(p[2] == 0 ? 0 : 1);
			break;
		case TypeActor:
			selectId(ui->comboActor, p[1]);
			if (auto* button = m_actor_group->button(p[2])) {
				button->setChecked(true);
			}
			switch (p[2]) {
				case ActorName: ui->lineActorName->setText(ToQString(m_edit.string)); break;
				case ActorLevel: ui->spinActorLevel->setValue(p[3]); break;
				case ActorHp: ui->spinActorHp->setValue(p[3]); break;
				case ActorSkill: selectId(ui->comboActorSkill, p[3]); break;
				case ActorEquipment: selectId(ui->comboActorEquipment, p[3]); break;
				case ActorState: selectId(ui->comboActorState, p[3]); break;
				default: break;
			}
			break;
		case TypeEvent:
			event_id = p[1];
			ui->comboEventDirection->setCurrentIndex(p[2]);
			break;
		case TypeVehicle:
			ui->comboVehicle->setCurrentIndex(p[1]);
			break;
		case TypeOther:
			ui->comboOther->setCurrentIndex(p[1]);
			break;
		default:
			break;
	}
	EventTargetComboBox::fill(ui->comboEvent, event_id, true);

	if (auto* button = m_type_group->button(type)) {
		button->setChecked(true);
	}
	// Show RPG Maker 2003 conditions used by a RPG Maker 2000 project
	if (type == TypeTimer2) {
		for (QWidget* widget : std::initializer_list<QWidget*>{ ui->radioTimer2, ui->spinTimer2Min, ui->spinTimer2Sec, ui->comboTimer2Op }) {
			widget->show();
		}
	} else if (type == TypeOther) {
		ui->radioOther->show();
		ui->comboOther->show();
	}

	switch (type) {
		case TypeGold: case TypeItem: case TypeActor:
			ui->tabWidget->setCurrentWidget(ui->tabParty);
			break;
		case TypeEvent: case TypeVehicle: case TypeActionKey: case TypeBgmLooped: case TypeOther:
			ui->tabWidget->setCurrentWidget(ui->tabOther);
			break;
		default:
			ui->tabWidget->setCurrentWidget(ui->tabValues);
			break;
	}

	if (m_type_group->button(type)) {
		ui->labelUnsupported->hide();
	} else {
		ui->labelUnsupported->setText(tr("This editor does not support the condition type %1. "
			"Its parameters are kept unless another condition is selected.").arg(type));
	}

	Branch branch = findBranch(m_commands->commands(), m_commands->index());
	ui->checkElse->setChecked(branch.end_index != -1 ? branch.else_index != -1 : p[param_else] != 0);
}

void ConditionalBranchWidget::connectWidgets() {
	auto changed = [this] {
		updateEnabled();
		writeParameters();
	};

	connect(m_type_group, &QButtonGroup::idToggled, this, [=](int, bool checked) {
		if (checked) {
			changed();
		}
	});
	connect(m_actor_group, &QButtonGroup::idToggled, this, [=](int, bool checked) {
		if (checked) {
			changed();
		}
	});
	connect(ui->radioVariableConstant, &QRadioButton::toggled, this, changed);

	for (auto* combo : findChildren<RpgComboBoxBase*>()) {
		connect(combo->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, changed);
	}
	for (auto* combo : { ui->comboSwitchState, ui->comboVariableOp, ui->comboTimer1Op, ui->comboTimer2Op,
			ui->comboGoldOp, ui->comboItemState, ui->comboEvent, ui->comboEventDirection, ui->comboVehicle, ui->comboOther }) {
		connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, changed);
	}
	for (auto* spin : findChildren<QSpinBox*>()) {
		connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, changed);
	}
	connect(ui->lineActorName, &QLineEdit::textEdited, this, changed);
	connect(ui->checkElse, &QCheckBox::toggled, this, [this] { updateElseWarning(); });
}

void ConditionalBranchWidget::writeParameters() {
	int type = m_type_group->checkedId();
	if (type == -1) {
		return;
	}

	auto& p = m_edit.parameters;

	// RPG Maker stores 0 in the parameters a condition does not use
	if (p[0] != type) {
		p[0] = type;
		std::fill(p.begin() + 1, p.begin() + param_else, 0);
		m_edit.string = {};
	}

	auto seconds = [](QSpinBox* min, QSpinBox* sec) {
		return min->value() * 60 + sec->value();
	};

	switch (type) {
		case TypeSwitch:
			p[1] = idOf(ui->comboSwitch);
			p[2] = ui->comboSwitchState->currentIndex();
			break;
		case TypeVariable:
			p[1] = idOf(ui->comboVariable);
			if (ui->radioVariableConstant->isChecked()) {
				p[2] = 0;
				p[3] = ui->spinVariableValue->value();
			} else {
				p[2] = 1;
				p[3] = idOf(ui->comboVariableOperand);
			}
			p[4] = ui->comboVariableOp->currentIndex();
			break;
		case TypeTimer1:
			p[1] = seconds(ui->spinTimer1Min, ui->spinTimer1Sec);
			p[2] = ui->comboTimer1Op->currentIndex();
			break;
		case TypeTimer2:
			p[1] = seconds(ui->spinTimer2Min, ui->spinTimer2Sec);
			p[2] = ui->comboTimer2Op->currentIndex();
			break;
		case TypeGold:
			p[1] = ui->spinGold->value();
			p[2] = ui->comboGoldOp->currentIndex();
			break;
		case TypeItem:
			p[1] = idOf(ui->comboItem);
			p[2] = ui->comboItemState->currentIndex();
			break;
		case TypeActor: {
			p[1] = idOf(ui->comboActor);
			int check = m_actor_group->checkedId();
			if (p[2] != check) {
				p[2] = check;
				p[3] = 0;
				m_edit.string = {};
			}
			switch (check) {
				case ActorName: m_edit.string = ToDBString(ui->lineActorName->text()); break;
				case ActorLevel: p[3] = ui->spinActorLevel->value(); break;
				case ActorHp: p[3] = ui->spinActorHp->value(); break;
				case ActorSkill: p[3] = idOf(ui->comboActorSkill); break;
				case ActorEquipment: p[3] = idOf(ui->comboActorEquipment); break;
				case ActorState: p[3] = idOf(ui->comboActorState); break;
				default: break;
			}
			break;
		}
		case TypeEvent:
			p[1] = ui->comboEvent->currentData().toInt();
			p[2] = ui->comboEventDirection->currentIndex();
			break;
		case TypeVehicle:
			p[1] = ui->comboVehicle->currentIndex();
			break;
		case TypeOther:
			p[1] = ui->comboOther->currentIndex();
			break;
		default:
			break;
	}
}

void ConditionalBranchWidget::updateEnabled() {
	int type = m_type_group->checkedId();

	auto enable = [](std::initializer_list<QWidget*> widgets, bool enabled) {
		for (auto* widget : widgets) {
			widget->setEnabled(enabled);
		}
	};

	enable({ ui->comboSwitch, ui->comboSwitchState }, type == TypeSwitch);
	enable({ ui->comboVariable, ui->comboVariableOp, ui->radioVariableConstant, ui->radioVariableVariable }, type == TypeVariable);
	ui->spinVariableValue->setEnabled(type == TypeVariable && ui->radioVariableConstant->isChecked());
	ui->comboVariableOperand->setEnabled(type == TypeVariable && ui->radioVariableVariable->isChecked());
	enable({ ui->spinTimer1Min, ui->spinTimer1Sec, ui->comboTimer1Op }, type == TypeTimer1);
	enable({ ui->spinTimer2Min, ui->spinTimer2Sec, ui->comboTimer2Op }, type == TypeTimer2);
	enable({ ui->spinGold, ui->comboGoldOp }, type == TypeGold);
	enable({ ui->comboItem, ui->comboItemState }, type == TypeItem);
	enable({ ui->comboActor, ui->widgetActor }, type == TypeActor);
	int check = m_actor_group->checkedId();
	ui->lineActorName->setEnabled(check == ActorName);
	ui->spinActorLevel->setEnabled(check == ActorLevel);
	ui->spinActorHp->setEnabled(check == ActorHp);
	ui->comboActorSkill->setEnabled(check == ActorSkill);
	ui->comboActorEquipment->setEnabled(check == ActorEquipment);
	ui->comboActorState->setEnabled(check == ActorState);
	enable({ ui->comboEvent, ui->comboEventDirection }, type == TypeEvent);
	ui->comboVehicle->setEnabled(type == TypeVehicle);
	ui->comboOther->setEnabled(type == TypeOther);
}

void ConditionalBranchWidget::updateElseWarning() {
	Branch branch = findBranch(m_commands->commands(), m_commands->index());
	// An empty else branch holds only the END of the branch
	bool loses_commands = !ui->checkElse->isChecked() && branch.else_index != -1
		&& branch.end_index - branch.else_index > 2;
	if (loses_commands) {
		ui->labelElseWarning->setText(tr("The commands of the else branch will be deleted."));
		ui->labelElseWarning->show();
	} else {
		ui->labelElseWarning->hide();
	}
}

void ConditionalBranchWidget::apply() {
	auto& commands = m_commands->commands();
	int index = m_commands->index();
	int indent = commands[index].indent;
	bool with_else = ui->checkElse->isChecked();

	m_edit.parameters[param_else] = with_else ? 1 : 0;
	m_edit.indent = indent;
	commands[index] = m_edit;

	Branch branch = findBranch(commands, index);
	if (branch.end_index == -1) {
		// The branch has no body yet, give it one
		std::vector<lcf::rpg::EventCommand> body;
		body.push_back(makeCommand(Cmd::END, indent + 1));
		if (with_else) {
			body.push_back(makeCommand(Cmd::ElseBranch, indent));
			body.push_back(makeCommand(Cmd::END, indent + 1));
		}
		body.push_back(makeCommand(Cmd::EndBranch, indent));
		commands.insert(commands.begin() + index + 1, body.begin(), body.end());
	} else if (with_else && branch.else_index == -1) {
		std::vector<lcf::rpg::EventCommand> body = {
			makeCommand(Cmd::ElseBranch, indent),
			makeCommand(Cmd::END, indent + 1)
		};
		commands.insert(commands.begin() + branch.end_index, body.begin(), body.end());
	} else if (!with_else && branch.else_index != -1) {
		commands.erase(commands.begin() + branch.else_index, commands.begin() + branch.end_index);
	}

	updateElseWarning();
}
