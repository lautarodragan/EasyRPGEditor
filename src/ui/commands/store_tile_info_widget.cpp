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

#include "store_tile_info_widget.h"
#include "ui_store_tile_info_widget.h"
#include "param_bindings.h"
#include "core.h"
#include "ui/common/map_location_dialog.h"
#include <algorithm>

using namespace ParamBindings;

StoreTileInfoWidget::StoreTileInfoWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::StoreTileInfoWidget) {

	ui->setupUi(this);

	ui->comboVarX->makeModel(project);
	ui->comboVarY->makeModel(project);
	ui->comboTarget->makeModel(project);
}

StoreTileInfoWidget::~StoreTileInfoWidget() {
	delete ui;
}

void StoreTileInfoWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: mode (0: constant, 1: variables), x, y, variable
	ensureParameters(cmd, 4);
	EventCommandBaseWidget::setData(cmd);

	bool by_variable = m_cmd->parameters[0] != 0;
	ui->radioVariable->setChecked(by_variable);
	ui->radioConstant->setChecked(!by_variable);
	if (by_variable) {
		// Variable IDs start at 1, the combo boxes at 0
		ui->comboVarX->setCurrentIndex(std::max(m_cmd->parameters[1] - 1, 0));
		ui->comboVarY->setCurrentIndex(std::max(m_cmd->parameters[2] - 1, 0));
	} else {
		ui->spinX->setValue(m_cmd->parameters[1]);
		ui->spinY->setValue(m_cmd->parameters[2]);
	}
	bindDatabaseCombo(ui->comboTarget, m_cmd, 3);
	updateEnabled();

	connect(ui->radioConstant, &QRadioButton::toggled, this, [this] { updateEnabled(); writeLocation(); });
	connect(ui->spinX, qOverload<int>(&QSpinBox::valueChanged), this, [this] { writeLocation(); });
	connect(ui->spinY, qOverload<int>(&QSpinBox::valueChanged), this, [this] { writeLocation(); });
	connect(ui->comboVarX->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writeLocation(); });
	connect(ui->comboVarY->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writeLocation(); });

	// The location is on the map the event runs on
	connect(ui->pushPick, &QPushButton::clicked, this, [this] {
		MapLocationDialog dialog(m_project, core().currentMapId(), ui->spinX->value(), ui->spinY->value(), false, this);
		if (dialog.exec() == QDialog::Accepted) {
			ui->spinX->setValue(dialog.x());
			ui->spinY->setValue(dialog.y());
		}
	});
}

void StoreTileInfoWidget::writeLocation() {
	// X and Y share their parameters: coordinates or variable IDs, depending on the mode
	if (ui->radioConstant->isChecked()) {
		m_cmd->parameters[0] = 0;
		m_cmd->parameters[1] = ui->spinX->value();
		m_cmd->parameters[2] = ui->spinY->value();
	} else {
		m_cmd->parameters[0] = 1;
		m_cmd->parameters[1] = ui->comboVarX->currentIndex() + 1;
		m_cmd->parameters[2] = ui->comboVarY->currentIndex() + 1;
	}
}

void StoreTileInfoWidget::updateEnabled() {
	bool constant = ui->radioConstant->isChecked();
	ui->spinX->setEnabled(constant);
	ui->spinY->setEnabled(constant);
	ui->pushPick->setEnabled(constant && core().currentMapId() != 0);
	ui->groupVariables->setEnabled(!constant);
}
