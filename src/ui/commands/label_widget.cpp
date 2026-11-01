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

#include "label_widget.h"
#include "ui_label_widget.h"
#include "param_bindings.h"

using namespace ParamBindings;

LabelWidget::LabelWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::LabelWidget) {

	ui->setupUi(this);
}

LabelWidget::~LabelWidget() {
	delete ui;
}

void LabelWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: label number
	ensureParameters(cmd, 1);
	EventCommandBaseWidget::setData(cmd);

	bindSpin(ui->spinLabel, m_cmd, 0);
}
