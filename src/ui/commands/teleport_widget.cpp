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

#include "teleport_widget.h"
#include "ui_teleport_widget.h"
#include "core.h"
#include "stringizer.h"
#include "model/project_data.h"
#include "ui/common/map_location_dialog.h"
#include <lcf/saveopt.h>

namespace {
// Parameters: map, x, y and, in RPG Maker 2003, the direction
constexpr int param_count = 3;
}

TeleportWidget::TeleportWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::TeleportWidget) {

	ui->setupUi(this);

	// 0 keeps the facing, 1 to 4 are up, right, down and left
	ui->comboDirection->addItem(tr("Retain Facing"));
	for (int dir = 0; dir < 4; ++dir) {
		ui->comboDirection->addItem(Stringizer::direction(dir));
	}
}

TeleportWidget::~TeleportWidget() {
	delete ui;
}

void TeleportWidget::setData(lcf::rpg::EventCommand* cmd) {
	bool rpg2k3 = lcf::GetEngineVersion(m_project.database()) == lcf::EngineVersion::e2k3;
	size_t count = rpg2k3 ? param_count + 1 : param_count;

	if (cmd->parameters.size() < count) {
		std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
		params.resize(count, 0);
		// A new teleport goes to the map being edited
		if (params[0] == 0) {
			params[0] = core().currentMapId();
		}
		cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	}

	EventCommandBaseWidget::setData(cmd);

	ui->groupDirection->setVisible(cmd->parameters.size() > param_count);
	if (cmd->parameters.size() > param_count) {
		ui->comboDirection->setCurrentIndex(std::clamp(cmd->parameters[3], 0, 4));
		connect(ui->comboDirection, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
			m_cmd->parameters[3] = index;
		});
	}

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
}

void TeleportWidget::updateLocation() {
	ui->lineLocation->setText(QString("%1:%2 (%3,%4)")
		.arg(m_cmd->parameters[0], 4, 10, QChar('0'))
		.arg(Stringizer::mapName(m_cmd->parameters[0]))
		.arg(m_cmd->parameters[1], 3, 10, QChar('0'))
		.arg(m_cmd->parameters[2], 3, 10, QChar('0')));
}
