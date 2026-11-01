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

#include "fade_out_bgm_widget.h"
#include "ui_fade_out_bgm_widget.h"

FadeOutBgmWidget::FadeOutBgmWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::FadeOutBgmWidget) {

	ui->setupUi(this);
}

FadeOutBgmWidget::~FadeOutBgmWidget() {
	delete ui;
}

void FadeOutBgmWidget::setData(lcf::rpg::EventCommand* cmd) {
	EventCommandBaseWidget::setData(cmd);
	resizeCommandList(0);

	// Stored in milliseconds, RPG Maker edits 1 to 10 whole seconds.
	// Written only when changed, to keep other values (like 0) as they are.
	ui->spinTime->setValue(m_cmd->parameters[0] / 1000);
	connect(ui->spinTime, qOverload<int>(&QSpinBox::valueChanged), this, [this](int seconds) {
		m_cmd->parameters[0] = seconds * 1000;
	});
}
