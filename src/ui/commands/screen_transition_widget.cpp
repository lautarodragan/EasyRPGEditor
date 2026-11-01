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

#include "screen_transition_widget.h"
#include "ui_screen_transition_widget.h"
#include "stringizer.h"

ScreenTransitionWidget::ScreenTransitionWidget(ProjectData& project, QWidget *parent, bool erase) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ScreenTransitionWidget),
	m_erase(erase) {

	ui->setupUi(this);

	if (!erase) {
		ui->groupTransition->setTitle(tr("Show transition"));
	}
}

ScreenTransitionWidget::~ScreenTransitionWidget() {
	delete ui;
}

void ScreenTransitionWidget::setData(lcf::rpg::EventCommand* cmd) {
	EventCommandBaseWidget::setData(cmd);
	resizeCommandList(0);

	auto name = [this](int id) {
		return m_erase ? Stringizer::eraseTransitionName(id) : Stringizer::showTransitionName(id);
	};

	// -1 is the transition chosen in the database or by Change Screen Transitions
	ui->comboTransition->addItem(tr("Default (same as Teleport)"), -1);
	for (int id = 0; id < Stringizer::transitionCount; ++id) {
		ui->comboTransition->addItem(name(id), id);
	}

	int current = m_cmd->parameters[0];
	int index = ui->comboTransition->findData(current);
	if (index == -1) {
		// Not a transition of RPG Maker: kept as is until another is chosen
		ui->comboTransition->addItem(tr("Unknown (%1)").arg(current), current);
		index = ui->comboTransition->count() - 1;
	}
	ui->comboTransition->setCurrentIndex(index);

	connect(ui->comboTransition, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		m_cmd->parameters[0] = ui->comboTransition->itemData(i).toInt();
	});
}
