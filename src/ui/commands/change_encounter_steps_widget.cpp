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

#include "change_encounter_steps_widget.h"
#include "ui_change_encounter_steps_widget.h"
#include "param_bindings.h"

using namespace ParamBindings;

ChangeEncounterStepsWidget::ChangeEncounterStepsWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ChangeEncounterStepsWidget) {

	ui->setupUi(this);
}

ChangeEncounterStepsWidget::~ChangeEncounterStepsWidget() {
	delete ui;
}

void ChangeEncounterStepsWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: steps
	ensureParameters(cmd, 1);
	EventCommandBaseWidget::setData(cmd);

	bindSpin(ui->spinSteps, m_cmd, 0);
}
