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

#include "simple_command_widgets.h"
#include "command_bindings.h"
#include "common/dbstring.h"
#include "model/project_data.h"
#include "ui/common/rpg_combobox.h"
#include "ui/common/rpg_spinbox.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QToolButton>
#include <QVBoxLayout>
#include <memory>

using namespace CommandBindings;

SimpleCommandWidget::SimpleCommandWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent) {
	auto* layout = new QVBoxLayout(this);
	m_form = new QFormLayout;
	layout->addLayout(m_form);
	layout->addStretch(1);
}

void SimpleCommandWidget::setData(lcf::rpg::EventCommand* cmd) {
	// The controls aren't connected by their names like in other editors
	m_cmd = cmd;
	if (m_bind) {
		m_bind(cmd);
	}
}

namespace {
// Radio buttons in a row, their IDs are the parameter values
QButtonGroup* radios(QWidget* owner, QFormLayout* form, const QString& label, std::initializer_list<QString> texts, int first_id = 0) {
	auto* group = new QButtonGroup(owner);
	auto* row = new QHBoxLayout;
	int id = first_id;
	for (const auto& text : texts) {
		auto* radio = new QRadioButton(text);
		group->addButton(radio, id++);
		row->addWidget(radio);
	}
	row->addStretch(1);
	form->addRow(label, row);
	return group;
}

template<typename T>
T* dbCombo(ProjectData& project, QWidget* owner) {
	auto* combo = new T(owner);
	combo->makeModel(project);
	return combo;
}

QComboBox* monsterCombo() {
	auto* combo = new QComboBox;
	addMonsters(combo);
	return combo;
}

QComboBox* battleCommandCombo(ProjectData& project) {
	auto* combo = new QComboBox;
	for (const auto& command : project.database().battlecommands.commands) {
		combo->addItem(QStringLiteral("%1: %2").arg(command.ID, 4, 10, QChar('0')).arg(ToQString(command.name)), command.ID);
	}
	return combo;
}

/**
 * An amount given by a mode (0 constant, 1 variable, 2 percentage) and a value
 * (the constant, variable ID or percentage).
 */
struct Operand {
	QButtonGroup* mode;
	QSpinBox* value;
	RpgComboBoxBase* variable;
	QSpinBox* percent = nullptr;

	Operand(ProjectData& project, QWidget* owner, QFormLayout* form, bool with_percent) {
		mode = new QButtonGroup(owner);
		auto* row = new QHBoxLayout;
		auto* constant = new QRadioButton(QObject::tr("Constant"));
		mode->addButton(constant, 0);
		value = new RpgSpinBox;
		value->setRange(0, 99999);
		row->addWidget(constant);
		row->addWidget(value, 1);
		auto* from_variable = new QRadioButton(QObject::tr("Variable"));
		mode->addButton(from_variable, 1);
		variable = dbCombo<VariableRpgComboBox>(project, owner);
		row->addWidget(from_variable);
		row->addWidget(variable, 2);
		if (with_percent) {
			auto* percent_radio = new QRadioButton(QObject::tr("Percentage"));
			mode->addButton(percent_radio, 2);
			percent = new RpgSpinBox;
			percent->setRange(0, 100);
			percent->setSuffix(QStringLiteral(" %"));
			row->addWidget(percent_radio);
			row->addWidget(percent, 1);
		}
		form->addRow(QObject::tr("Amount"), row);
	}

	void updateEnabled() {
		value->setEnabled(mode->checkedId() == 0);
		variable->setEnabled(mode->checkedId() == 1);
		if (percent) {
			percent->setEnabled(mode->checkedId() == 2);
		}
	}

	void bind(lcf::rpg::EventCommand* cmd, size_t mode_index, size_t value_index) {
		const int current = param(*cmd, mode_index);
		const int amount = param(*cmd, value_index);
		if (auto* button = mode->button(current)) {
			button->setChecked(true);
		}
		value->setValue(current == 0 ? amount : 0);
		variable->comboBox()->setCurrentIndex(current == 1 ? amount - 1 : 0);
		if (percent) {
			percent->setValue(current == 2 ? amount : 0);
		}

		// The mode decides which control writes the value
		auto write = [=, this] {
			const int id = mode->checkedId();
			if (id < 0) {
				return;
			}
			setParam(*cmd, mode_index, id);
			switch (id) {
				case 0: setParam(*cmd, value_index, value->value()); break;
				case 1: setParam(*cmd, value_index, variable->comboBox()->currentIndex() + 1); break;
				case 2: setParam(*cmd, value_index, percent->value()); break;
			}
			updateEnabled();
		};
		QObject::connect(mode, &QButtonGroup::idToggled, mode, [=](int, bool checked) {
			if (checked) {
				write();
			}
		});
		QObject::connect(value, qOverload<int>(&QSpinBox::valueChanged), mode, write);
		QObject::connect(variable->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), mode, write);
		if (percent) {
			QObject::connect(percent, qOverload<int>(&QSpinBox::valueChanged), mode, write);
		}
		updateEnabled();
	}
};

void addBackdrop(ProjectData& project, QWidget* owner, QFormLayout* form, QLineEdit*& edit) {
	auto* row = new QHBoxLayout;
	edit = new QLineEdit;
	edit->setReadOnly(true);
	auto* pick = new QToolButton;
	pick->setText(QStringLiteral("..."));
	row->addWidget(edit, 1);
	row->addWidget(pick);
	form->addRow(QObject::tr("Background"), row);
	QObject::connect(pick, &QToolButton::clicked, owner, [&project, edit, owner] {
		pickBackdrop(project, edit, owner);
	});
}
}

// Parameters as read by Game_Interpreter_Map::CommandEnterHeroName of the Player
EnterHeroNameWidget::EnterHeroNameWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* actor = dbCombo<ActorRpgComboBox>(project, this);
	m_form->addRow(tr("Actor"), actor);
	auto* charset = new QComboBox;
	charset->addItem(tr("Letters"), 0);
	charset->addItem(tr("Symbols"), 1);
	m_form->addRow(tr("Initial page"), charset);
	auto* show_name = new QCheckBox(tr("Show the current name"));
	m_form->addRow(QString(), show_name);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(actor, cmd, 0);
		bind(charset, cmd, 1);
		bind(show_name, cmd, 2);
	};
}

// Parameters as read by Game_Interpreter_Battle of the Player.
// Monsters are counted from 0 in the troop.
ChangeMonsterHPWidget::ChangeMonsterHPWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* monster = monsterCombo();
	m_form->addRow(tr("Monster"), monster);
	auto* operation = radios(this, m_form, tr("Operation"), { tr("Increase"), tr("Decrease") });
	auto operand = std::make_shared<Operand>(project, this, m_form, true);
	auto* lethal = new QCheckBox(tr("Can kill the monster"));
	m_form->addRow(QString(), lethal);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(monster, cmd, 0);
		bind(operation, cmd, 1);
		operand->bind(cmd, 2, 3);
		bind(lethal, cmd, 4);
	};
}

ChangeMonsterMPWidget::ChangeMonsterMPWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* monster = monsterCombo();
	m_form->addRow(tr("Monster"), monster);
	auto* operation = radios(this, m_form, tr("Operation"), { tr("Increase"), tr("Decrease") });
	auto operand = std::make_shared<Operand>(project, this, m_form, false);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(monster, cmd, 0);
		bind(operation, cmd, 1);
		operand->bind(cmd, 2, 3);
	};
}

ChangeMonsterConditionWidget::ChangeMonsterConditionWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* monster = monsterCombo();
	m_form->addRow(tr("Monster"), monster);
	auto* operation = radios(this, m_form, tr("Operation"), { tr("Inflict"), tr("Remove") });
	auto* state = dbCombo<StateRpgComboBox>(project, this);
	m_form->addRow(tr("Condition"), state);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(monster, cmd, 0);
		bind(operation, cmd, 1);
		bind(state, cmd, 2);
	};
}

ShowHiddenMonsterWidget::ShowHiddenMonsterWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* monster = monsterCombo();
	m_form->addRow(tr("Monster"), monster);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(monster, cmd, 0);
	};
}

ChangeBattleBGWidget::ChangeBattleBGWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	QLineEdit* name = nullptr;
	addBackdrop(project, this, m_form, name);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bindString(name, cmd);
	};
}

ShowBattleAnimationBattleWidget::ShowBattleAnimationBattleWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	const bool is_2k3 = project.database().system.ldb_id == 2003;
	auto* animation = dbCombo<AnimationRpgComboBox>(project, this);
	m_form->addRow(tr("Animation"), animation);
	auto* allies = new QCheckBox(tr("Target the party"));
	m_form->addRow(QString(), allies);
	allies->setVisible(is_2k3);
	auto* target = new QComboBox;
	m_form->addRow(tr("Target"), target);
	auto* wait = new QCheckBox(tr("Wait until done"));
	m_form->addRow(QString(), wait);

	// -1 targets everyone. Monsters are counted from 0, party members from 1.
	auto fill_targets = [target](bool party) {
		QSignalBlocker blocker(target);
		target->clear();
		if (party) {
			target->addItem(tr("Entire party"), -1);
			addNumbers(target, 1, 4, 0, tr("Party member %1"));
		} else {
			target->addItem(tr("All monsters"), -1);
			addMonsters(target);
		}
	};

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(animation, cmd, 0);
		const bool party = is_2k3 && param(*cmd, 3) != 0;
		fill_targets(party);
		bind(target, cmd, 1);
		bind(wait, cmd, 2);
		if (is_2k3) {
			bind(allies, cmd, 3, 0, [=] {
				// The targets differ, start with everyone
				fill_targets(allies->isChecked());
				target->setCurrentIndex(0);
				setParam(*cmd, 1, -1);
			});
		} else if (param(*cmd, 3) != 0) {
			// Shown when a RPG Maker 2000 project has the RPG Maker 2003 parameter
			allies->show();
			{
				QSignalBlocker blocker(allies);
				allies->setChecked(true);
			}
			allies->setEnabled(false);
		}
	};
}

// Parameters as read by Game_Interpreter_Battle::CommandEnableCombo of the Player
EnableComboWidget::EnableComboWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* actor = dbCombo<ActorRpgComboBox>(project, this);
	m_form->addRow(tr("Actor"), actor);
	auto* command = battleCommandCombo(project);
	m_form->addRow(tr("Battle command"), command);
	auto* times = new RpgSpinBox;
	times->setRange(1, 8);
	m_form->addRow(tr("Number of times"), times);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		bind(actor, cmd, 0);
		bind(command, cmd, 1, 1);
		bind(times, cmd, 2, 1);
	};
}

// Parameters as read by Game_Interpreter_Battle::CommandForceFlee of the Player
ForceFleeWidget::ForceFleeWidget(ProjectData& project, QWidget* parent) :
	SimpleCommandWidget(project, parent) {
	auto* who = radios(this, m_form, tr("Flees"), { tr("Party"), tr("All monsters"), tr("One monster") });
	auto* monster = monsterCombo();
	m_form->addRow(tr("Monster"), monster);
	auto* ignore = new QCheckBox(tr("Also in a pincers attack (party) or surround attack (monsters)"));
	m_form->addRow(QString(), ignore);

	m_bind = [=](lcf::rpg::EventCommand* cmd) {
		auto update = [=] {
			monster->setEnabled(who->checkedId() == 2);
		};
		bind(who, cmd, 0, 0, update);
		bind(monster, cmd, 1);
		bind(ignore, cmd, 2);
		update();
	};
}
