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

#include "player_visibility_widget.h"
#include "ui_player_visibility_widget.h"
#include "param_bindings.h"

using namespace ParamBindings;

PlayerVisibilityWidget::PlayerVisibilityWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::PlayerVisibilityWidget) {

	ui->setupUi(this);

	ui->groupVisible->setId(ui->radioHide, 0);
	ui->groupVisible->setId(ui->radioShow, 1);
}

PlayerVisibilityWidget::~PlayerVisibilityWidget() {
	delete ui;
}

void PlayerVisibilityWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: 0 hides the hero, 1 shows it
	ensureParameters(cmd, 1);
	EventCommandBaseWidget::setData(cmd);

	bindRadios(ui->groupVisible, m_cmd, 0);
}
