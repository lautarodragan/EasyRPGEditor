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

#include "move_event_widget.h"
#include "ui_move_event_widget.h"
#include "ui/common/event_target_combobox.h"
#include "ui/common/move_route_editor.h"
#include "model/move_route_codec.h"
#include "model/project.h"
#include "model/project_data.h"
#include <QAbstractButton>
#include <algorithm>

MoveEventWidget::MoveEventWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::MoveEventWidget) {

	ui->setupUi(this);

	for (int i = 1; i <= 8; ++i) {
		auto* radio = findChild<QAbstractButton*>(QString("radioFrequency%1").arg(i));
		ui->groupFrequency_arg1->setId(radio, i);
	}

	m_encoding = m_project.project().encoding().toStdString();

	ui->routeEditor->setProject(m_project);
	ui->routeEditor->addBelowList(ui->groupOptions);
	connect(ui->routeEditor, &MoveRouteEditor::routeChanged, this, [this] {
		MoveRouteCodec::Encode(m_cmd->parameters, ui->routeEditor->route(), m_encoding);
	});
}

MoveEventWidget::~MoveEventWidget() {
	delete ui;
}

void MoveEventWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Ensure frequency, repeat and skip exist before the base class binds them,
	// and that the frequency has a radio button (1 to 8)
	std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
	if (params.size() < MoveRouteCodec::first_command) {
		params.resize(MoveRouteCodec::first_command, 0);
	}
	params[1] = std::clamp(params[1], 1, 8);
	cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());

	EventCommandBaseWidget::setData(cmd);

	std::vector<lcf::rpg::MoveCommand> route;
	// A route that can't be read is shown but not edited, keeping the data untouched
	bool valid = MoveRouteCodec::Decode(cmd->parameters, route, m_encoding);
	ui->routeEditor->setRoute(route, valid);

	populateTargets();
}

void MoveEventWidget::populateTargets() {
	EventTargetComboBox::fill(ui->comboTarget, m_cmd->parameters[0], true);

	connect(ui->comboTarget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		m_cmd->parameters[0] = ui->comboTarget->itemData(i).toInt();
	});
}

