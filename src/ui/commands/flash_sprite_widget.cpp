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

#include "flash_sprite_widget.h"
#include "ui_flash_sprite_widget.h"
#include "param_bindings.h"
#include "ui/common/event_target_combobox.h"
#include <QSlider>

using namespace ParamBindings;

FlashSpriteWidget::FlashSpriteWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::FlashSpriteWidget) {

	ui->setupUi(this);

	for (auto [slider, spin] : { std::pair{ui->sliderRed, ui->spinRed}, {ui->sliderGreen, ui->spinGreen},
			{ui->sliderBlue, ui->spinBlue}, {ui->sliderStrength, ui->spinStrength} }) {
		connect(slider, &QSlider::valueChanged, spin, &QSpinBox::setValue);
		connect(spin, qOverload<int>(&QSpinBox::valueChanged), slider, &QSlider::setValue);
	}
}

FlashSpriteWidget::~FlashSpriteWidget() {
	delete ui;
}

void FlashSpriteWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: character, red, green, blue, strength, duration in tenths of a second, wait
	// (a seventh one of Maniac Patch is kept)
	ensureParameters(cmd, 7);
	EventCommandBaseWidget::setData(cmd);

	EventTargetComboBox::fill(ui->comboTarget, m_cmd->parameters[0], true);
	connect(ui->comboTarget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		m_cmd->parameters[0] = ui->comboTarget->itemData(i).toInt();
	});

	bindSpin(ui->spinRed, m_cmd, 1);
	bindSpin(ui->spinGreen, m_cmd, 2);
	bindSpin(ui->spinBlue, m_cmd, 3);
	bindSpin(ui->spinStrength, m_cmd, 4);
	bindSpin(ui->spinTime, m_cmd, 5);
	bindCheck(ui->checkWait, m_cmd, 6);
}
