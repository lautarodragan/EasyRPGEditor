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

#include "actor_commands_widget.h"
#include "common/dbstring.h"
#include "model/project_data.h"
#include "ui/common/rpg_spinbox.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <algorithm>

namespace {
// Values of parameter 0
enum Target {
	TargetParty = 0,
	TargetActor = 1,
	TargetVariable = 2,
};

using Entries = std::vector<std::pair<int, QString>>;

template <typename T, typename Pred>
Entries entriesOf(const std::vector<T>& list, Pred pred) {
	Entries entries;
	for (const auto& item : list) {
		if (pred(item)) {
			entries.emplace_back(item.ID, ToQString(item.name));
		}
	}
	return entries;
}

template <typename T>
Entries entriesOf(const std::vector<T>& list) {
	return entriesOf(list, [](const T&) { return true; });
}

QString entryText(int id, const QString& name) {
	return QString("%1: %2").arg(id, 4, 10, QChar('0')).arg(name);
}
}

int ActorCommandWidget::Operand::type() const {
	if (radio_constant->isChecked()) {
		return 0;
	}
	if (radio_variable->isChecked()) {
		return 1;
	}
	return -1;
}

int ActorCommandWidget::Operand::value() const {
	return type() == 1 ? comboId(combo_variable) : constant_value();
}

ActorCommandWidget::ActorCommandWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent) {

	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(0, 0, 0, 0);
}

void ActorCommandWidget::setData(lcf::rpg::EventCommand* cmd) {
	EventCommandBaseWidget::setData(cmd);

	addTarget();
	build();
	m_layout->addStretch();
}

int ActorCommandWidget::param(int index) const {
	return index < static_cast<int>(m_cmd->parameters.size()) ? m_cmd->parameters[index] : 0;
}

void ActorCommandWidget::setParam(int index, int value) {
	resizeCommandList(index);
	m_cmd->parameters[index] = value;
}

int ActorCommandWidget::comboId(QComboBox* combo) {
	return combo->currentData().toInt();
}

void ActorCommandWidget::selectId(QComboBox* combo, int id) {
	combo->setCurrentIndex(combo->findData(id));
}

QGroupBox* ActorCommandWidget::addGroup(const QString& title) {
	auto* group = new QGroupBox(title, this);
	m_layout->addWidget(group);
	return group;
}

ActorCommandWidget::Entries ActorCommandWidget::variables() const {
	return entriesOf(m_project.database().variables);
}

QComboBox* ActorCommandWidget::makeCombo(const Entries& entries, int current) {
	auto* combo = new QComboBox(this);
	for (const auto& [id, name] : entries) {
		combo->addItem(entryText(id, name), id);
	}
	if (combo->findData(current) == -1) {
		// Not in the list (deleted or of another type), keep it selectable
		combo->addItem(entryText(current, QString("<%1?>").arg(current)), current);
	}
	selectId(combo, current);
	return combo;
}

QComboBox* ActorCommandWidget::makeValueCombo(const QStringList& names, int current) {
	auto* combo = new QComboBox(this);
	for (int i = 0; i < names.size(); ++i) {
		combo->addItem(names[i], i);
	}
	if (combo->findData(current) == -1) {
		// Value of another engine version or patch
		combo->addItem(tr("Other (%1)").arg(current), current);
	}
	selectId(combo, current);
	return combo;
}

QSpinBox* ActorCommandWidget::makeSpin(int max, int value) {
	auto* spin = new RpgSpinBox(this);
	// Values out of the range of RPG Maker are kept
	spin->setRange(std::min(0, value), std::max(max, value));
	spin->setValue(value);
	return spin;
}

void ActorCommandWidget::bindCombo(QComboBox* combo, int index, std::function<void(int)> changed) {
	connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
		setParam(index, comboId(combo));
		if (changed) {
			changed(comboId(combo));
		}
	});
}

void ActorCommandWidget::bindCheck(QCheckBox* check, int index) {
	check->setChecked(param(index) != 0);
	connect(check, &QCheckBox::toggled, this, [=, this](bool checked) {
		setParam(index, checked ? 1 : 0);
	});
}

void ActorCommandWidget::addTarget() {
	auto* group = addGroup(tr("Target"));
	auto* grid = new QGridLayout(group);

	int mode = param(0);
	int id = param(1);

	auto* radio_party = new QRadioButton(tr("Entire party"), group);
	auto* radio_actor = new QRadioButton(tr("Actor:"), group);
	auto* radio_variable = new QRadioButton(tr("Actor ID from variable:"), group);
	auto* buttons = new QButtonGroup(group);
	buttons->addButton(radio_party, TargetParty);
	buttons->addButton(radio_actor, TargetActor);
	buttons->addButton(radio_variable, TargetVariable);

	// The combo box of the other mode shows the first entry
	auto* combo_actor = makeCombo(entriesOf(m_project.database().actors), mode == TargetActor ? id : 1);
	auto* combo_variable = makeCombo(variables(), mode == TargetVariable ? id : 1);

	grid->addWidget(radio_party, 0, 0);
	grid->addWidget(radio_actor, 1, 0);
	grid->addWidget(combo_actor, 1, 1);
	grid->addWidget(radio_variable, 2, 0);
	grid->addWidget(combo_variable, 2, 1);
	grid->setColumnStretch(1, 1);

	if (auto* button = buttons->button(mode)) {
		button->setChecked(true);
	}

	auto update = [=] {
		combo_actor->setEnabled(radio_actor->isChecked());
		combo_variable->setEnabled(radio_variable->isChecked());
	};
	update();

	connect(buttons, &QButtonGroup::idToggled, this, [=, this](int new_mode, bool checked) {
		if (!checked) {
			return;
		}
		setParam(0, new_mode);
		// RPG Maker stores 0 as the ID of the entire party
		setParam(1, new_mode == TargetActor ? comboId(combo_actor)
			: new_mode == TargetVariable ? comboId(combo_variable) : 0);
		update();
	});
	connect(combo_actor, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
		if (radio_actor->isChecked()) {
			setParam(1, comboId(combo_actor));
		}
	});
	connect(combo_variable, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
		if (radio_variable->isChecked()) {
			setParam(1, comboId(combo_variable));
		}
	});
}

QButtonGroup* ActorCommandWidget::addChoice(const QString& title, int index, const QStringList& labels,
		std::function<void(int)> changed) {
	auto* group = addGroup(title);
	auto* layout = new QHBoxLayout(group);
	auto* buttons = new QButtonGroup(group);
	for (int i = 0; i < labels.size(); ++i) {
		auto* radio = new QRadioButton(labels[i], group);
		buttons->addButton(radio, i);
		layout->addWidget(radio);
	}
	layout->addStretch();

	if (auto* button = buttons->button(param(index))) {
		button->setChecked(true);
	}

	connect(buttons, &QButtonGroup::idToggled, this, [=, this](int id, bool checked) {
		if (checked) {
			setParam(index, id);
			if (changed) {
				changed(id);
			}
		}
	});
	return buttons;
}

ActorCommandWidget::Operand& ActorCommandWidget::addOperand(QGroupBox* group, int idx_type, int idx_value,
		QWidget* constant, std::function<int()> constant_value, bool load) {
	m_operands.push_back(std::make_unique<Operand>());
	auto& op = *m_operands.back();
	op.idx_type = idx_type;
	op.idx_value = idx_value;
	op.constant_value = std::move(constant_value);
	op.constant = constant;

	int type = load ? param(idx_type) : 0;
	op.radio_constant = new QRadioButton(tr("Constant:"), group);
	op.radio_variable = new QRadioButton(tr("Variable:"), group);
	op.combo_variable = makeCombo(variables(), load && type == 1 ? param(idx_value) : 1);
	auto* buttons = new QButtonGroup(group);
	buttons->addButton(op.radio_constant, 0);
	buttons->addButton(op.radio_variable, 1);
	// Other types (of patches) leave both unchecked
	if (auto* button = buttons->button(type)) {
		button->setChecked(true);
	}

	auto* grid = qobject_cast<QGridLayout*>(group->layout());
	if (!grid) {
		grid = new QGridLayout(group);
	}
	int row = grid->rowCount();
	grid->addWidget(op.radio_constant, row, 0);
	grid->addWidget(constant, row, 1);
	grid->addWidget(op.radio_variable, row + 1, 0);
	grid->addWidget(op.combo_variable, row + 1, 1);
	grid->setColumnStretch(1, 1);

	auto* ptr = &op;
	connect(buttons, &QButtonGroup::idToggled, this, [=, this](int, bool checked) {
		if (checked) {
			writeOperand(*ptr);
			updateOperand(*ptr, ptr->radio_constant->isEnabled());
		}
	});
	connect(op.combo_variable, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
		if (ptr->type() == 1) {
			writeOperand(*ptr);
		}
	});
	updateOperand(op);
	return op;
}

void ActorCommandWidget::writeOperand(const Operand& operand) {
	int type = operand.type();
	if (type == -1) {
		return;
	}
	setParam(operand.idx_type, type);
	setParam(operand.idx_value, operand.value());
}

void ActorCommandWidget::updateOperand(const Operand& operand, bool enabled) {
	operand.radio_constant->setEnabled(enabled);
	operand.radio_variable->setEnabled(enabled);
	operand.constant->setEnabled(enabled && operand.type() == 0);
	operand.combo_variable->setEnabled(enabled && operand.type() == 1);
}

// Change HP

void ChangeHPWidget::build() {
	auto* check = new QCheckBox(tr("Can kill (HP can reach 0)"), this);
	addChoice(tr("Operation"), 2, { tr("Increase"), tr("Decrease") }, [check](int decrease) {
		// Only decreasing can kill
		check->setEnabled(decrease != 0);
	});

	auto* group = addGroup(tr("Amount"));
	auto* spin = makeSpin(9999, param(3) == 0 ? param(4) : 0);
	auto& op = addOperand(group, 3, 4, spin, [spin] { return spin->value(); });
	connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, [this, &op] { writeOperand(op); });

	m_layout->addWidget(check);
	bindCheck(check, 5);
	check->setEnabled(param(2) != 0);
}

// Change SP

void ChangeSPWidget::build() {
	addChoice(tr("Operation"), 2, { tr("Increase"), tr("Decrease") });

	auto* group = addGroup(tr("Amount"));
	auto* spin = makeSpin(999, param(3) == 0 ? param(4) : 0);
	auto& op = addOperand(group, 3, 4, spin, [spin] { return spin->value(); });
	connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, [this, &op] { writeOperand(op); });
}

// Change Parameters

void ChangeParametersWidget::build() {
	addChoice(tr("Operation"), 2, { tr("Increase"), tr("Decrease") });

	auto* group = addGroup(tr("Parameter"));
	auto* layout = new QHBoxLayout(group);
	auto* combo = makeValueCombo({ tr("Max HP"), tr("Max SP"), tr("Attack"), tr("Defense"), tr("Spirit"), tr("Agility") }, param(3));
	layout->addWidget(combo);
	bindCombo(combo, 3);

	group = addGroup(tr("Amount"));
	auto* spin = makeSpin(9999, param(4) == 0 ? param(5) : 0);
	auto& op = addOperand(group, 4, 5, spin, [spin] { return spin->value(); });
	connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, [this, &op] { writeOperand(op); });
}

// Change Skills

void ChangeSkillsWidget::build() {
	addChoice(tr("Operation"), 2, { tr("Learn"), tr("Forget") });

	auto* group = addGroup(tr("Skill"));
	auto* combo = makeCombo(entriesOf(m_project.database().skills), param(3) == 0 ? param(4) : 1);
	auto& op = addOperand(group, 3, 4, combo, [combo] { return comboId(combo); });
	connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, &op] { writeOperand(op); });
}

// Change Equipment

void ChangeEquipmentWidget::build() {
	const bool unequip = param(2) != 0;

	auto* group = addGroup(tr("Operation"));
	auto* grid = new QGridLayout(group);
	auto* radio_equip = new QRadioButton(tr("Equip"), group);
	auto* radio_unequip = new QRadioButton(tr("Unequip:"), group);
	auto* buttons = new QButtonGroup(group);
	buttons->addButton(radio_equip, 0);
	buttons->addButton(radio_unequip, 1);
	grid->addWidget(radio_equip, 0, 0, 1, 2);

	// Only weapons, shields, armors, helmets and accessories can be equipped
	auto equipment = entriesOf(m_project.database().items, [](const lcf::rpg::Item& item) {
		return item.type >= lcf::rpg::Item::Type_weapon && item.type <= lcf::rpg::Item::Type_accessory;
	});
	int item_id = !unequip && param(3) == 0 ? param(4) : (equipment.empty() ? 1 : equipment.front().first);
	auto* combo_item = makeCombo(equipment, item_id);
	if (!unequip && param(3) == 0) {
		// An item that is no equipment shows its name
		const auto& items = m_project.database().items;
		int i = combo_item->findData(item_id);
		if (item_id >= 1 && item_id <= static_cast<int>(items.size())
				&& combo_item->itemText(i).endsWith("?>")) {
			combo_item->setItemText(i, entryText(item_id, ToQString(items[item_id - 1].name)));
		}
	}
	// When unequipping, parameter 3 is the slot
	auto& op = addOperand(group, 3, 4, combo_item, [combo_item] { return comboId(combo_item); }, !unequip);

	auto* combo_slot = makeValueCombo({ tr("Weapon"), tr("Shield"), tr("Armor"), tr("Helmet"), tr("Accessory"), tr("All") },
		unequip ? param(3) : 0);
	int row = grid->rowCount();
	grid->addWidget(radio_unequip, row, 0);
	grid->addWidget(combo_slot, row, 1);

	buttons->button(unequip ? 1 : 0)->setChecked(true);

	auto update = [=, this, &op] {
		updateOperand(op, radio_equip->isChecked());
		combo_slot->setEnabled(radio_unequip->isChecked());
	};
	update();

	connect(buttons, &QButtonGroup::idToggled, this, [=, this, &op](int id, bool checked) {
		if (!checked) {
			return;
		}
		setParam(2, id);
		if (id == 0) {
			writeOperand(op);
		} else {
			setParam(3, comboId(combo_slot));
			setParam(4, 0);
		}
		update();
	});
	connect(combo_item, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, &op] { writeOperand(op); });
	connect(combo_slot, qOverload<int>(&QComboBox::currentIndexChanged), this, [=, this] {
		setParam(3, comboId(combo_slot));
	});
}

// Change Condition

void ChangeConditionWidget::build() {
	addChoice(tr("Operation"), 2, { tr("Inflict"), tr("Remove") });

	auto* group = addGroup(tr("Condition"));
	auto* layout = new QHBoxLayout(group);
	auto* combo = makeCombo(entriesOf(m_project.database().states), param(3));
	layout->addWidget(combo);
	bindCombo(combo, 3);
}

// Change Class

void ChangeClassWidget::build() {
	auto* group = addGroup(tr("Class"));
	auto* grid = new QGridLayout(group);

	Entries classes = { { 0, tr("None") } };
	auto list = entriesOf(m_project.database().classes);
	classes.insert(classes.end(), list.begin(), list.end());
	auto* combo_class = makeCombo(classes, param(2));
	combo_class->setItemText(0, tr("None"));
	grid->addWidget(combo_class, 0, 0, 1, 2);
	bindCombo(combo_class, 2);

	auto* check_level = new QCheckBox(tr("Reset the level to 1"), group);
	grid->addWidget(check_level, 1, 0, 1, 2);
	bindCheck(check_level, 3);

	auto add_mode = [&](int row, const QString& label, int index, const QStringList& names) {
		grid->addWidget(new QLabel(label, group), row, 0);
		auto* combo = makeValueCombo(names, param(index));
		grid->addWidget(combo, row, 1);
		bindCombo(combo, index);
	};
	add_mode(2, tr("Skills:"), 4, { tr("Don't change"), tr("Replace with the skills of the class"),
		tr("Add the skills of the class") });
	add_mode(3, tr("Parameters:"), 5, { tr("Don't change"), tr("Halve"),
		tr("Base values of the class at level 1"), tr("Base values of the class at the current level") });
	grid->setColumnStretch(1, 1);

	auto* check_message = new QCheckBox(tr("Show a message for the level and learned skills"), this);
	m_layout->addWidget(check_message);
	bindCheck(check_message, 6);
}

// Change Battle Commands

void ChangeBattleCommandsWidget::build() {
	auto* group = addGroup(tr("Battle command"));
	auto* layout = new QHBoxLayout(group);
	Entries commands = { { 0, tr("All (only to remove)") } };
	auto list = entriesOf(m_project.database().battlecommands.commands);
	commands.insert(commands.end(), list.begin(), list.end());
	auto* combo = makeCombo(commands, param(2));
	combo->setItemText(0, tr("All (only to remove)"));
	layout->addWidget(combo);
	bindCombo(combo, 2);

	addChoice(tr("Operation"), 3, { tr("Remove"), tr("Add") }, [this, combo](int add) {
		if (add && param(2) == 0 && combo->count() > 1) {
			// All commands can only be removed
			combo->setCurrentIndex(1);
		}
	});
}
