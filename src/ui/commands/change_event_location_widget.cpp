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

#include "change_event_location_widget.h"
#include "ui_change_event_location_widget.h"
#include "ui/common/event_target_combobox.h"
#include "ui/common/map_location_dialog.h"
#include "core.h"
#include <algorithm>

namespace {
// Parameters: target, mode (0: constant, 1: variables), x, y
constexpr int param_count = 4;
}

ChangeEventLocationWidget::ChangeEventLocationWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ChangeEventLocationWidget) {

	ui->setupUi(this);

	ui->comboVarX->makeModel(project);
	ui->comboVarY->makeModel(project);
}

ChangeEventLocationWidget::~ChangeEventLocationWidget() {
	delete ui;
}

void ChangeEventLocationWidget::setData(lcf::rpg::EventCommand* cmd) {
	// RPG Maker 2003 adds a direction after these, it is kept as is
	if (cmd->parameters.size() < param_count) {
		std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
		params.resize(param_count, 0);
		cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	}

	EventCommandBaseWidget::setData(cmd);

	EventTargetComboBox::fill(ui->comboTarget, cmd->parameters[0], false);

	bool by_variable = cmd->parameters[1] != 0;
	ui->radioVariable->setChecked(by_variable);
	ui->radioConstant->setChecked(!by_variable);
	if (by_variable) {
		// Variable IDs start at 1, the combo boxes at 0
		ui->comboVarX->setCurrentIndex(std::max(cmd->parameters[2] - 1, 0));
		ui->comboVarY->setCurrentIndex(std::max(cmd->parameters[3] - 1, 0));
	} else {
		ui->spinX->setValue(cmd->parameters[2]);
		ui->spinY->setValue(cmd->parameters[3]);
	}
	updateEnabled();

	connect(ui->comboTarget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		m_cmd->parameters[0] = ui->comboTarget->itemData(i).toInt();
	});
	connect(ui->radioConstant, &QRadioButton::toggled, this, [this] { updateEnabled(); writeLocation(); });
	connect(ui->spinX, qOverload<int>(&QSpinBox::valueChanged), this, [this] { writeLocation(); });
	connect(ui->spinY, qOverload<int>(&QSpinBox::valueChanged), this, [this] { writeLocation(); });
	connect(ui->comboVarX->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writeLocation(); });
	connect(ui->comboVarY->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writeLocation(); });

	// Events can only move on the map they are on
	connect(ui->pushPick, &QPushButton::clicked, this, [this] {
		MapLocationDialog dialog(m_project, core().currentMapId(), ui->spinX->value(), ui->spinY->value(), false, this);
		if (dialog.exec() == QDialog::Accepted) {
			ui->spinX->setValue(dialog.x());
			ui->spinY->setValue(dialog.y());
		}
	});
}

void ChangeEventLocationWidget::writeLocation() {
	// X and Y share their parameters: coordinates or variable IDs, depending on the mode
	if (ui->radioConstant->isChecked()) {
		m_cmd->parameters[1] = 0;
		m_cmd->parameters[2] = ui->spinX->value();
		m_cmd->parameters[3] = ui->spinY->value();
	} else {
		m_cmd->parameters[1] = 1;
		m_cmd->parameters[2] = ui->comboVarX->currentIndex() + 1;
		m_cmd->parameters[3] = ui->comboVarY->currentIndex() + 1;
	}
}

void ChangeEventLocationWidget::updateEnabled() {
	bool constant = ui->radioConstant->isChecked();
	ui->spinX->setEnabled(constant);
	ui->spinY->setEnabled(constant);
	ui->pushPick->setEnabled(constant && core().currentMapId() != 0);
	ui->groupVariables->setEnabled(!constant);
}
