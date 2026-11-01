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

#include "set_vehicle_location_widget.h"
#include "ui_set_vehicle_location_widget.h"
#include "param_bindings.h"
#include "core.h"
#include "stringizer.h"
#include "ui/common/map_location_dialog.h"
#include <algorithm>

using namespace ParamBindings;

SetVehicleLocationWidget::SetVehicleLocationWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::SetVehicleLocationWidget) {

	ui->setupUi(this);

	ui->comboVehicle->addItem(tr("Boat"), 0);
	ui->comboVehicle->addItem(tr("Ship"), 1);
	ui->comboVehicle->addItem(tr("Airship"), 2);

	ui->comboVarMap->makeModel(project);
	ui->comboVarX->makeModel(project);
	ui->comboVarY->makeModel(project);
}

SetVehicleLocationWidget::~SetVehicleLocationWidget() {
	delete ui;
}

void SetVehicleLocationWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: vehicle (0: boat, 1: ship, 2: airship), mode (0: constant, 1: variables), map, x, y
	ensureParameters(cmd, 5);
	EventCommandBaseWidget::setData(cmd);

	bindComboData(ui->comboVehicle, m_cmd, 0);

	bool by_variable = m_cmd->parameters[1] != 0;
	ui->radioVariable->setChecked(by_variable);
	ui->radioConstant->setChecked(!by_variable);
	if (by_variable) {
		// Variable IDs start at 1, the combo boxes at 0
		ui->comboVarMap->setCurrentIndex(std::max(m_cmd->parameters[2] - 1, 0));
		ui->comboVarX->setCurrentIndex(std::max(m_cmd->parameters[3] - 1, 0));
		ui->comboVarY->setCurrentIndex(std::max(m_cmd->parameters[4] - 1, 0));
		m_map = std::max(core().currentMapId(), 1);
	} else {
		m_map = m_cmd->parameters[2];
		m_x = m_cmd->parameters[3];
		m_y = m_cmd->parameters[4];
	}
	updateLocation();
	updateEnabled();

	connect(ui->radioConstant, &QRadioButton::toggled, this, [this] { updateEnabled(); writeLocation(); });
	for (auto* combo : { ui->comboVarMap, ui->comboVarX, ui->comboVarY }) {
		connect(combo->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writeLocation(); });
	}

	connect(ui->pushPick, &QPushButton::clicked, this, [this] {
		MapLocationDialog dialog(m_project, m_map, m_x, m_y, true, this);
		if (dialog.exec() == QDialog::Accepted) {
			m_map = dialog.mapId();
			m_x = dialog.x();
			m_y = dialog.y();
			updateLocation();
			writeLocation();
		}
	});
}

void SetVehicleLocationWidget::writeLocation() {
	// Map, X and Y share their parameters: values or variable IDs, depending on the mode
	if (ui->radioConstant->isChecked()) {
		m_cmd->parameters[1] = 0;
		m_cmd->parameters[2] = m_map;
		m_cmd->parameters[3] = m_x;
		m_cmd->parameters[4] = m_y;
	} else {
		m_cmd->parameters[1] = 1;
		m_cmd->parameters[2] = ui->comboVarMap->currentIndex() + 1;
		m_cmd->parameters[3] = ui->comboVarX->currentIndex() + 1;
		m_cmd->parameters[4] = ui->comboVarY->currentIndex() + 1;
	}
}

void SetVehicleLocationWidget::updateLocation() {
	ui->lineLocation->setText(QString("%1:%2 (%3,%4)")
		.arg(m_map, 4, 10, QChar('0'))
		.arg(Stringizer::mapName(m_map))
		.arg(m_x, 3, 10, QChar('0'))
		.arg(m_y, 3, 10, QChar('0')));
}

void SetVehicleLocationWidget::updateEnabled() {
	bool constant = ui->radioConstant->isChecked();
	ui->lineLocation->setEnabled(constant);
	ui->pushPick->setEnabled(constant);
	ui->groupVariables->setEnabled(!constant);
}
