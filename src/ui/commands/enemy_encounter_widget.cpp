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

#include "enemy_encounter_widget.h"
#include "command_bindings.h"
#include "model/project_data.h"
#include "ui/common/rpg_combobox.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>

using namespace CommandBindings;

namespace {
// Parameters as read by Game_Interpreter_Map::CommandEnemyEncounter of the Player
enum Param {
	ParamTroopMode = 0, // 0 troop, 1 troop from a variable
	ParamTroop = 1,
	ParamBackground = 2, // 0 map, 1 specific (string), 2 terrain (RPG Maker 2003)
	ParamEscape = 3, // 0 disallowed, 1 ends the event, 2 escape handler
	ParamDefeat = 4, // 0 game over, 1 defeat handler
	ParamFirstStrike = 5,
	// RPG Maker 2003
	ParamCondition = 6, // lcf::rpg::System::BattleCondition
	ParamFormation = 7, // lcf::rpg::System::BattleFormation, with a specific background
	ParamTerrain = 8,
};

QRadioButton* addRadio(QButtonGroup* group, QGridLayout* layout, const QString& text, int id, int row) {
	auto* radio = new QRadioButton(text);
	group->addButton(radio, id);
	layout->addWidget(radio, row, 0);
	return radio;
}
}

EnemyEncounterWidget::EnemyEncounterWidget(ProjectData& project, QWidget* parent) :
	BlockCommandWidget(project, parent) {

	m_2k3 = project.database().system.ldb_id == 2003;

	auto* layout = new QVBoxLayout(this);

	// Troop
	auto* troop_box = new QGroupBox(tr("Troop"));
	auto* troop_layout = new QGridLayout(troop_box);
	m_troop_group = new QButtonGroup(this);
	addRadio(m_troop_group, troop_layout, tr("Specific troop"), 0, 0);
	addRadio(m_troop_group, troop_layout, tr("Troop ID from variable"), 1, 1);
	auto* troop = new TroopRpgComboBox(troop_box);
	troop->makeModel(project);
	m_troop = troop;
	auto* troop_variable = new VariableRpgComboBox(troop_box);
	troop_variable->makeModel(project);
	m_troop_variable = troop_variable;
	troop_layout->addWidget(m_troop, 0, 1);
	troop_layout->addWidget(m_troop_variable, 1, 1);
	troop_layout->setColumnStretch(1, 1);
	layout->addWidget(troop_box);

	// Background
	auto* bg_box = new QGroupBox(tr("Battle background"));
	auto* bg_layout = new QGridLayout(bg_box);
	m_bg_group = new QButtonGroup(this);
	addRadio(m_bg_group, bg_layout, tr("Same as the map"), 0, 0);
	addRadio(m_bg_group, bg_layout, tr("Specific background"), 1, 1);
	auto* terrain_radio = addRadio(m_bg_group, bg_layout, tr("Background of terrain"), 2, 3);
	auto* bg_row = new QHBoxLayout;
	m_bg_name = new QLineEdit;
	m_bg_name->setReadOnly(true);
	m_bg_pick = new QToolButton;
	m_bg_pick->setText(QStringLiteral("..."));
	bg_row->addWidget(m_bg_name);
	bg_row->addWidget(m_bg_pick);
	bg_layout->addLayout(bg_row, 1, 1);
	m_formation = new QComboBox;
	m_formation->addItem(tr("Formation of the terrain"), 0);
	m_formation->addItem(tr("Loose formation"), 1);
	m_formation->addItem(tr("Tight formation"), 2);
	bg_layout->addWidget(m_formation, 2, 1);
	auto* terrain = new TerrainRpgComboBox(bg_box);
	terrain->makeModel(project);
	m_terrain = terrain;
	bg_layout->addWidget(m_terrain, 3, 1);
	bg_layout->setColumnStretch(1, 1);
	layout->addWidget(bg_box);
	terrain_radio->setVisible(m_2k3);
	m_terrain->setVisible(m_2k3);
	m_formation->setVisible(m_2k3);

	// Escape and defeat
	auto* handlers = new QHBoxLayout;
	auto* escape_box = new QGroupBox(tr("Escape"));
	auto* escape_layout = new QGridLayout(escape_box);
	m_escape_group = new QButtonGroup(this);
	addRadio(m_escape_group, escape_layout, tr("Disallow"), 0, 0);
	addRadio(m_escape_group, escape_layout, tr("End event processing"), 1, 1);
	addRadio(m_escape_group, escape_layout, tr("Execute custom handler"), 2, 2);
	handlers->addWidget(escape_box);
	auto* defeat_box = new QGroupBox(tr("Defeat"));
	auto* defeat_layout = new QGridLayout(defeat_box);
	m_defeat_group = new QButtonGroup(this);
	addRadio(m_defeat_group, defeat_layout, tr("Game over"), 0, 0);
	addRadio(m_defeat_group, defeat_layout, tr("Execute custom handler"), 1, 1);
	defeat_layout->setRowStretch(2, 1);
	handlers->addWidget(defeat_box);
	layout->addLayout(handlers);

	// Special conditions
	auto* options = new QHBoxLayout;
	m_first_strike = new QCheckBox(tr("First strike"));
	options->addWidget(m_first_strike);
	m_condition = new QComboBox;
	m_condition->addItem(tr("No special condition"), 0);
	m_condition->addItem(tr("Initiative"), 1);
	m_condition->addItem(tr("Back attack"), 2);
	m_condition->addItem(tr("Surround attack"), 3);
	m_condition->addItem(tr("Pincers attack"), 4);
	m_condition->setVisible(m_2k3);
	options->addWidget(m_condition, 1);
	layout->addLayout(options);

	layout->addWidget(warningLabel());
	layout->addStretch(1);

	connect(m_bg_pick, &QToolButton::clicked, this, [this] {
		pickBackdrop(m_project, m_bg_name, this);
	});
}

void EnemyEncounterWidget::load() {
	auto* troop = static_cast<RpgComboBoxBase*>(m_troop);
	auto* troop_variable = static_cast<RpgComboBoxBase*>(m_troop_variable);

	// The troop: the radio button picks the combo box that writes the ID
	const bool from_variable = param(m_edit, ParamTroopMode) != 0;
	if (auto* button = m_troop_group->button(from_variable ? 1 : 0)) {
		button->setChecked(true);
	}
	const int troop_id = param(m_edit, ParamTroop, 1);
	troop->comboBox()->setCurrentIndex(from_variable ? 0 : troop_id - 1);
	troop_variable->comboBox()->setCurrentIndex(from_variable ? troop_id - 1 : 0);
	auto write_troop = [this, troop, troop_variable] {
		const bool variable = m_troop_group->checkedId() == 1;
		auto* combo = variable ? troop_variable : troop;
		if (combo->comboBox()->currentIndex() < 0) {
			return;
		}
		setParam(m_edit, ParamTroopMode, variable ? 1 : 0);
		setParam(m_edit, ParamTroop, combo->comboBox()->currentIndex() + 1);
		updateEnabled();
	};
	connect(m_troop_group, &QButtonGroup::idToggled, this, [=](int, bool checked) {
		if (checked) {
			write_troop();
		}
	});
	connect(troop->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, write_troop);
	connect(troop_variable->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, write_troop);

	// Background
	if (param(m_edit, ParamBackground) == 2) {
		m_bg_group->button(2)->show();
		m_terrain->show();
	}
	bind(m_bg_group, &m_edit, ParamBackground, 0, [this] { updateEnabled(); });
	bindString(m_bg_name, &m_edit);
	bind(static_cast<RpgComboBoxBase*>(m_terrain), &m_edit, ParamTerrain, 1);
	bind(m_formation, &m_edit, ParamFormation, 0);

	// Handlers
	bind(m_escape_group, &m_edit, ParamEscape, 1, [this] { branchesChanged(); });
	bind(m_defeat_group, &m_edit, ParamDefeat, 0, [this] { branchesChanged(); });

	bind(m_first_strike, &m_edit, ParamFirstStrike);
	bind(m_condition, &m_edit, ParamCondition);

	updateEnabled();
}

void EnemyEncounterWidget::updateEnabled() {
	m_troop->setEnabled(m_troop_group->checkedId() == 0);
	m_troop_variable->setEnabled(m_troop_group->checkedId() == 1);
	const int bg = m_bg_group->checkedId();
	m_bg_name->setEnabled(bg == 1);
	m_bg_pick->setEnabled(bg == 1);
	m_formation->setEnabled(bg == 1);
	m_terrain->setEnabled(bg == 2);
}
