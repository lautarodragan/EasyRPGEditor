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

#include "change_screen_transitions_widget.h"
#include "ui_change_screen_transitions_widget.h"
#include "param_bindings.h"
#include "stringizer.h"
#include <QSignalBlocker>

using namespace ParamBindings;

ChangeScreenTransitionsWidget::ChangeScreenTransitionsWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ChangeScreenTransitionsWidget) {

	ui->setupUi(this);

	for (int type = 0; type < Stringizer::transitionTypeCount; ++type) {
		ui->comboType->addItem(Stringizer::transitionTypeName(type), type);
	}
}

ChangeScreenTransitionsWidget::~ChangeScreenTransitionsWidget() {
	delete ui;
}

void ChangeScreenTransitionsWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: type (even ones erase, odd ones show), transition
	ensureParameters(cmd, 2);
	EventCommandBaseWidget::setData(cmd);

	bindComboData(ui->comboType, m_cmd, 0);
	fillTransitions();
	connect(ui->comboType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { fillTransitions(); });
	connect(ui->comboTransition, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		if (i >= 0) {
			m_cmd->parameters[1] = ui->comboTransition->itemData(i).toInt();
		}
	});
}

void ChangeScreenTransitionsWidget::fillTransitions() {
	// Erasing and showing name the same transition IDs differently
	bool erase = m_cmd->parameters[0] % 2 == 0;
	QSignalBlocker blocker(ui->comboTransition);
	ui->comboTransition->clear();
	// transitionCount is "None": the screen changes instantly
	for (int id = 0; id <= Stringizer::transitionCount; ++id) {
		ui->comboTransition->addItem(erase ? Stringizer::eraseTransitionName(id) : Stringizer::showTransitionName(id), id);
	}
	selectData(ui->comboTransition, m_cmd->parameters[1]);
}
