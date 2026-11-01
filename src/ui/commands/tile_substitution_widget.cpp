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

#include "tile_substitution_widget.h"
#include "ui_tile_substitution_widget.h"
#include "param_bindings.h"

using namespace ParamBindings;

TileSubstitutionWidget::TileSubstitutionWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::TileSubstitutionWidget) {

	ui->setupUi(this);

	ui->groupLayerButtons->setId(ui->radioLower, 0);
	ui->groupLayerButtons->setId(ui->radioUpper, 1);
}

TileSubstitutionWidget::~TileSubstitutionWidget() {
	delete ui;
}

void TileSubstitutionWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: layer (0: lower, 1: upper), old tile, new tile
	ensureParameters(cmd, 3);
	EventCommandBaseWidget::setData(cmd);

	bindRadios(ui->groupLayerButtons, m_cmd, 0);
	updateRange();
	bindSpin(ui->spinOld, m_cmd, 1);
	bindSpin(ui->spinNew, m_cmd, 2);

	connect(ui->groupLayerButtons, &QButtonGroup::idToggled, this, [this] { updateRange(); });
}

void TileSubstitutionWidget::updateRange() {
	// The lower layer has 162 tiles (with the water and terrain tiles), the upper one 144
	int max = m_cmd->parameters[0] == 1 ? 143 : 161;
	ui->spinOld->setMaximum(max);
	ui->spinNew->setMaximum(max);
}
