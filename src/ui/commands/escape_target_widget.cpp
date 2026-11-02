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

#include "escape_target_widget.h"
#include "ui_escape_target_widget.h"
#include "param_bindings.h"
#include "stringizer.h"
#include "ui/common/map_location_dialog.h"

using namespace ParamBindings;

namespace {
// Parameters: map, x, y, switch enabled, switch
constexpr int param_count = 5;
}

EscapeTargetWidget::EscapeTargetWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::EscapeTargetWidget) {

	ui->setupUi(this);

	ui->comboSwitch->makeModel(project);
}

EscapeTargetWidget::~EscapeTargetWidget() {
	delete ui;
}

void EscapeTargetWidget::setData(lcf::rpg::EventCommand* cmd) {
	ensureParameters(cmd, param_count);
	EventCommandBaseWidget::setData(cmd);

	bindCheck(ui->checkSwitch, m_cmd, 3);
	bindDatabaseCombo(ui->comboSwitch, m_cmd, 4);

	connect(ui->checkSwitch, &QCheckBox::toggled, this, [this](bool checked) {
		// A switch must be picked when it is turned on
		if (checked && ui->comboSwitch->currentIndex() < 0 && ui->comboSwitch->comboBox()->count() > 0) {
			ui->comboSwitch->setCurrentIndex(0);
		}
		updateEnabled();
	});

	connect(ui->pushPick, &QPushButton::clicked, this, [this] {
		MapLocationDialog dialog(m_project, m_cmd->parameters[0], m_cmd->parameters[1], m_cmd->parameters[2], true, this);
		if (dialog.exec() == QDialog::Accepted) {
			m_cmd->parameters[0] = dialog.mapId();
			m_cmd->parameters[1] = dialog.x();
			m_cmd->parameters[2] = dialog.y();
			updateLocation();
		}
	});

	updateLocation();
	updateEnabled();
}

void EscapeTargetWidget::updateLocation() {
	ui->lineLocation->setText(QString("%1:%2 (%3,%4)")
		.arg(m_cmd->parameters[0], 4, 10, QChar('0'))
		.arg(Stringizer::mapName(m_cmd->parameters[0]))
		.arg(m_cmd->parameters[1], 3, 10, QChar('0'))
		.arg(m_cmd->parameters[2], 3, 10, QChar('0')));
}

void EscapeTargetWidget::updateEnabled() {
	ui->comboSwitch->setEnabled(ui->checkSwitch->isChecked());
}
