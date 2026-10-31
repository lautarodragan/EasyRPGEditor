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

#include "wait_widget.h"
#include "ui_wait_widget.h"

WaitWidget::WaitWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::WaitWidget) {

	ui->setupUi(this);

	ui->groupMode_arg1->setId(ui->radioTime, 0);
	ui->groupMode_arg1->setId(ui->radioKey, 1);
}

WaitWidget::~WaitWidget() {
	delete ui;
}

void WaitWidget::onParameterChanged(int index, int new_value) {
	// Wait mode: The duration only applies to "Wait for Time"
	if (index == 1) {
		ui->spinTime_arg0->setEnabled(new_value == 0);
	}
}
