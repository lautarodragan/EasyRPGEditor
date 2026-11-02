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

#include "teleport_targets_widget.h"
#include "ui_teleport_targets_widget.h"
#include "param_bindings.h"
#include "stringizer.h"
#include "ui/common/map_location_dialog.h"

using namespace ParamBindings;

namespace {
// Parameters: add (0) or remove, map, x, y, switch enabled, switch
constexpr int param_count = 6;
}

TeleportTargetsWidget::TeleportTargetsWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::TeleportTargetsWidget) {

	ui->setupUi(this);

	ui->groupOperation->setId(ui->radioAdd, 0);
	ui->groupOperation->setId(ui->radioRemove, 1);

	ui->comboSwitch->makeModel(project);
}

TeleportTargetsWidget::~TeleportTargetsWidget() {
	delete ui;
}

void TeleportTargetsWidget::setData(lcf::rpg::EventCommand* cmd) {
	ensureParameters(cmd, param_count);
	EventCommandBaseWidget::setData(cmd);

	// The Player removes the target for any value other than 0
	(m_cmd->parameters[0] == 0 ? ui->radioAdd : ui->radioRemove)->setChecked(true);
	connect(ui->groupOperation, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
		if (checked) {
			m_cmd->parameters[0] = id;
		}
	});
	bindCheck(ui->checkSwitch, m_cmd, 4);
	bindDatabaseCombo(ui->comboSwitch, m_cmd, 5);

	connect(ui->groupOperation, &QButtonGroup::idToggled, this, [this](int, bool checked) {
		if (checked) {
			updateLocation();
			updateEnabled();
		}
	});
	connect(ui->checkSwitch, &QCheckBox::toggled, this, [this](bool checked) {
		// A switch must be picked when it is turned on
		if (checked && ui->comboSwitch->currentIndex() < 0 && ui->comboSwitch->comboBox()->count() > 0) {
			ui->comboSwitch->setCurrentIndex(0);
		}
		updateEnabled();
	});

	connect(ui->pushPick, &QPushButton::clicked, this, [this] {
		MapLocationDialog dialog(m_project, m_cmd->parameters[1], m_cmd->parameters[2], m_cmd->parameters[3], true, this);
		if (dialog.exec() == QDialog::Accepted) {
			m_cmd->parameters[1] = dialog.mapId();
			// Removing only needs the map
			if (m_cmd->parameters[0] == 0) {
				m_cmd->parameters[2] = dialog.x();
				m_cmd->parameters[3] = dialog.y();
			}
			updateLocation();
		}
	});

	updateLocation();
	updateEnabled();
}

void TeleportTargetsWidget::updateLocation() {
	QString text = QString("%1:%2")
		.arg(m_cmd->parameters[1], 4, 10, QChar('0'))
		.arg(Stringizer::mapName(m_cmd->parameters[1]));
	if (m_cmd->parameters[0] == 0) {
		text += QString(" (%1,%2)")
			.arg(m_cmd->parameters[2], 3, 10, QChar('0'))
			.arg(m_cmd->parameters[3], 3, 10, QChar('0'));
	}
	ui->lineLocation->setText(text);
}

void TeleportTargetsWidget::updateEnabled() {
	// The switch is only used when adding a target
	bool adding = m_cmd->parameters[0] == 0;
	ui->groupSwitch->setEnabled(adding);
	ui->comboSwitch->setEnabled(adding && ui->checkSwitch->isChecked());
}
