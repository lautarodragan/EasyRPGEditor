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

#include "call_event_widget.h"
#include "ui_call_event_widget.h"
#include "ui/common/event_target_combobox.h"
#include <QButtonGroup>
#include <algorithm>

namespace {
// Parameters: mode, ID, page.
// The ID and page are values or variable IDs, depending on the mode.
enum Mode {
	CommonEvent = 0,
	MapEvent = 1,
	MapEventVariables = 2,
	// Maniac Patch: common event ID from a variable, or from the variable a variable points to
	CommonEventVariable = 3,
	CommonEventVariableIndirect = 4
};
constexpr int param_count = 3;
}

CallEventWidget::CallEventWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::CallEventWidget) {

	ui->setupUi(this);

	ui->comboCommonEvent->makeModel(project);
	ui->comboEventVar->makeModel(project);
	ui->comboPageVar->makeModel(project);
	ui->comboCommonEventVar->makeModel(project);

	ui->groupMode->setId(ui->radioCommonEvent, CommonEvent);
	ui->groupMode->setId(ui->radioMapEvent, MapEvent);
	ui->groupMode->setId(ui->radioVariables, MapEventVariables);
	ui->groupMode->setId(ui->radioCommonEventVar, CommonEventVariable);
	ui->groupMode->setId(ui->radioCommonEventVarIndirect, CommonEventVariableIndirect);
}

CallEventWidget::~CallEventWidget() {
	delete ui;
}

void CallEventWidget::setData(lcf::rpg::EventCommand* cmd) {
	EventCommandBaseWidget::setData(cmd);
	resizeCommandList(param_count - 1);

	int mode = m_cmd->parameters[0];
	int id = m_cmd->parameters[1];
	int page = m_cmd->parameters[2];

	// The Maniac Patch modes are only offered when the command uses one
	bool maniac = mode == CommonEventVariable || mode == CommonEventVariableIndirect;
	ui->radioCommonEventVar->setVisible(maniac);
	ui->radioCommonEventVarIndirect->setVisible(maniac);
	ui->comboCommonEventVar->setVisible(maniac);

	// Variable and database IDs start at 1, the combo boxes at 0
	auto index = [](int id) { return std::max(id - 1, 0); };
	ui->comboCommonEvent->setCurrentIndex(mode == CommonEvent ? index(id) : 0);
	EventTargetComboBox::fill(ui->comboEvent, mode == MapEvent ? id : EventTargetComboBox::this_event, false);
	ui->spinPage->setValue(mode == MapEvent ? page : 1);
	ui->comboEventVar->setCurrentIndex(mode == MapEventVariables ? index(id) : 0);
	ui->comboPageVar->setCurrentIndex(mode == MapEventVariables ? index(page) : 0);
	ui->comboCommonEventVar->setCurrentIndex(maniac ? index(id) : 0);

	if (auto* button = ui->groupMode->button(mode)) {
		button->setChecked(true);
	}
	updateEnabled();

	// Connected after the values are set: an unchanged command is kept as is
	connect(ui->groupMode, &QButtonGroup::idToggled, this, [this](int, bool checked) {
		if (checked) {
			updateEnabled();
			writeParameters();
		}
	});
	connect(ui->comboCommonEvent->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, &CallEventWidget::writeParameters);
	connect(ui->comboEvent, qOverload<int>(&QComboBox::currentIndexChanged), this, &CallEventWidget::writeParameters);
	connect(ui->spinPage, qOverload<int>(&QSpinBox::valueChanged), this, &CallEventWidget::writeParameters);
	connect(ui->comboEventVar->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, &CallEventWidget::writeParameters);
	connect(ui->comboPageVar->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, &CallEventWidget::writeParameters);
	connect(ui->comboCommonEventVar->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, &CallEventWidget::writeParameters);
}

void CallEventWidget::writeParameters() {
	int mode = ui->groupMode->checkedId();
	auto& params = m_cmd->parameters;

	switch (mode) {
		case CommonEvent:
			params[1] = ui->comboCommonEvent->currentIndex() + 1;
			params[2] = 0;
			break;
		case MapEvent:
			params[1] = ui->comboEvent->currentData().toInt();
			params[2] = ui->spinPage->value();
			break;
		case MapEventVariables:
			params[1] = ui->comboEventVar->currentIndex() + 1;
			params[2] = ui->comboPageVar->currentIndex() + 1;
			break;
		case CommonEventVariable:
		case CommonEventVariableIndirect:
			params[1] = ui->comboCommonEventVar->currentIndex() + 1;
			params[2] = 0;
			break;
		default:
			// Unknown mode, none checked
			return;
	}
	params[0] = mode;
}

void CallEventWidget::updateEnabled() {
	int mode = ui->groupMode->checkedId();
	ui->comboCommonEvent->setEnabled(mode == CommonEvent);
	ui->comboEvent->setEnabled(mode == MapEvent);
	ui->labelPage->setEnabled(mode == MapEvent);
	ui->spinPage->setEnabled(mode == MapEvent);
	ui->groupVariables->setEnabled(mode == MapEventVariables);
	ui->comboCommonEventVar->setEnabled(mode == CommonEventVariable || mode == CommonEventVariableIndirect);
}
