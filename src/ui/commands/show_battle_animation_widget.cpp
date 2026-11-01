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

#include "show_battle_animation_widget.h"
#include "ui_show_battle_animation_widget.h"
#include "ui/common/event_target_combobox.h"
#include <QSignalBlocker>
#include <vector>

namespace {
// Parameters of the map command: animation, target, wait until done, global (whole screen)
const std::vector<int32_t> defaults = { 1, EventTargetComboBox::hero, 1, 0 };
}

ShowBattleAnimationWidget::ShowBattleAnimationWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ShowBattleAnimationWidget) {

	ui->setupUi(this);

	ui->comboAnimation->makeModel(project);
}

ShowBattleAnimationWidget::~ShowBattleAnimationWidget() {
	delete ui;
}

void ShowBattleAnimationWidget::setData(lcf::rpg::EventCommand* cmd) {
	if (cmd->parameters.size() < defaults.size()) {
		std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
		for (size_t i = params.size(); i < defaults.size(); ++i) {
			params.push_back(defaults[i]);
		}
		cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	}

	EventCommandBaseWidget::setData(cmd);

	{
		QSignalBlocker blocker(ui->comboAnimation->comboBox());
		// Animation IDs start at 1, the combo box at 0
		ui->comboAnimation->setCurrentIndex(cmd->parameters[0] - 1);
	}
	connect(ui->comboAnimation->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		if (i >= 0) {
			m_cmd->parameters[0] = i + 1;
		}
	});

	{
		QSignalBlocker blocker(ui->comboTarget);
		EventTargetComboBox::fill(ui->comboTarget, cmd->parameters[1], true);
	}
	connect(ui->comboTarget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		m_cmd->parameters[1] = ui->comboTarget->itemData(i).toInt();
	});

	{
		QSignalBlocker b1(ui->checkWait), b2(ui->checkGlobal);
		ui->checkWait->setChecked(cmd->parameters[2] != 0);
		ui->checkGlobal->setChecked(cmd->parameters[3] != 0);
	}
	connect(ui->checkWait, &QCheckBox::toggled, this, [this](bool checked) {
		m_cmd->parameters[2] = checked ? 1 : 0;
	});
	connect(ui->checkGlobal, &QCheckBox::toggled, this, [this](bool checked) {
		m_cmd->parameters[3] = checked ? 1 : 0;
	});
}
