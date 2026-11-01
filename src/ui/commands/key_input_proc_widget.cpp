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

#include "key_input_proc_widget.h"
#include "command_bindings.h"
#include "model/project_data.h"
#include "ui/common/rpg_combobox.h"
#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace CommandBindings;

namespace {
// Parameters as read by Game_Interpreter::CommandKeyInputProc of the Player.
// Common: 0 variable, 1 wait for a key, 3 decision, 4 cancel.
constexpr size_t param_variable = 0;
constexpr size_t param_wait = 1;
constexpr size_t param_all_directions = 2; // old versions: the four directions
constexpr size_t param_decision = 3;
constexpr size_t param_cancel = 4;

constexpr int none = -1;

// Where the keys are stored, none when the version has no such parameter
struct Layout {
	int all_directions = none;
	int shift = none;
	int down = none;
	int left = none;
	int right = none;
	int up = none;
	int numbers = none;
	int operators = none;
	int time_variable = none;
	int timed = none;
};

Layout layoutFor(bool rpg2k3, size_t size) {
	Layout layout;
	// RPG Maker 2000 v1.50 and newer, also used by RPG Maker 2003 for 10 parameters
	auto rpg2k_150 = [&] {
		layout.shift = 5;
		layout.down = 6;
		layout.left = 7;
		layout.right = 8;
		layout.up = 9;
	};
	if (!rpg2k3) {
		if (size < 6) {
			layout.all_directions = param_all_directions;
		} else {
			rpg2k_150();
		}
	} else if (size == 10) {
		rpg2k_150();
	} else {
		layout.numbers = 5;
		layout.operators = 6;
		layout.time_variable = 7;
		layout.timed = 8;
		if (size > 10) {
			// RPG Maker 2003 v1.05 and newer
			layout.shift = 9;
			layout.down = 10;
			layout.left = 11;
			layout.right = 12;
			layout.up = 13;
		} else {
			layout.all_directions = param_all_directions;
		}
	}
	return layout;
}
}

KeyInputProcWidget::KeyInputProcWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent) {

	m_2k3 = project.database().system.ldb_id == 2003;

	auto* layout = new QVBoxLayout(this);
	auto* form = new QFormLayout;
	auto* variable = new VariableRpgComboBox(this);
	variable->makeModel(project);
	m_variable = variable;
	form->addRow(tr("Store key code in"), m_variable);
	layout->addLayout(form);

	m_wait = new QCheckBox(tr("Wait until a key is pressed"));
	layout->addWidget(m_wait);

	auto* keys_box = new QGroupBox(tr("Keys"));
	auto* keys = new QGridLayout(keys_box);
	m_down = new QCheckBox(tr("Down (1)"));
	m_left = new QCheckBox(tr("Left (2)"));
	m_right = new QCheckBox(tr("Right (3)"));
	m_up = new QCheckBox(tr("Up (4)"));
	m_directions = new QCheckBox(tr("Directions (1-4)"));
	m_decision = new QCheckBox(tr("Decision (5)"));
	m_cancel = new QCheckBox(tr("Cancel (6)"));
	m_shift = new QCheckBox(tr("Shift (7)"));
	m_numbers = new QCheckBox(tr("Numbers 0-9 (10-19)"));
	m_operators = new QCheckBox(tr("Symbols + - * / . (20-24)"));
	keys->addWidget(m_down, 0, 0);
	keys->addWidget(m_left, 1, 0);
	keys->addWidget(m_right, 2, 0);
	keys->addWidget(m_up, 3, 0);
	keys->addWidget(m_directions, 4, 0);
	keys->addWidget(m_decision, 0, 1);
	keys->addWidget(m_cancel, 1, 1);
	keys->addWidget(m_shift, 2, 1);
	keys->addWidget(m_numbers, 3, 1);
	keys->addWidget(m_operators, 4, 1);
	layout->addWidget(keys_box);

	auto* time_row = new QHBoxLayout;
	m_timed = new QCheckBox(tr("Store the time until the key press (1/10 s) in"));
	auto* time_variable = new VariableRpgComboBox(this);
	time_variable->makeModel(project);
	m_time_variable = time_variable;
	time_row->addWidget(m_timed);
	time_row->addWidget(m_time_variable, 1);
	layout->addLayout(time_row);
	layout->addStretch(1);
}

void KeyInputProcWidget::setData(lcf::rpg::EventCommand* cmd) {
	m_cmd = cmd;

	const Layout keys = layoutFor(m_2k3, cmd->parameters.size());
	auto changed = [this] { updateEnabled(); };

	bind(m_variable, cmd, param_variable);
	bind(m_wait, cmd, param_wait, 0, changed);
	bind(m_decision, cmd, param_decision);
	bind(m_cancel, cmd, param_cancel);

	auto bind_key = [&](QCheckBox* check, int index) {
		check->setVisible(index != none);
		if (index != none) {
			bind(check, cmd, index);
		}
	};
	bind_key(m_directions, keys.all_directions);
	bind_key(m_shift, keys.shift);
	bind_key(m_down, keys.down);
	bind_key(m_left, keys.left);
	bind_key(m_right, keys.right);
	bind_key(m_up, keys.up);
	bind_key(m_numbers, keys.numbers);
	bind_key(m_operators, keys.operators);
	bind_key(m_timed, keys.timed);
	m_time_variable->setVisible(keys.time_variable != none);
	if (keys.time_variable != none) {
		bind(m_time_variable, cmd, keys.time_variable, 1);
	}
	if (keys.timed != none) {
		connect(m_timed, &QCheckBox::toggled, this, changed);
	}

	updateEnabled();
}

void KeyInputProcWidget::updateEnabled() {
	// RPG Maker 2003 measures the time while waiting for the key
	m_timed->setEnabled(m_wait->isChecked());
	m_time_variable->setEnabled(m_wait->isChecked() && m_timed->isChecked());
}
