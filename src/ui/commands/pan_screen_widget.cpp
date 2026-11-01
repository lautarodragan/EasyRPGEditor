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

#include "pan_screen_widget.h"
#include "ui_pan_screen_widget.h"
#include "param_bindings.h"
#include "stringizer.h"

using namespace ParamBindings;

PanScreenWidget::PanScreenWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::PanScreenWidget) {

	ui->setupUi(this);

	int id = 0;
	for (auto* button : { ui->radioLock, ui->radioUnlock, ui->radioPan, ui->radioReset }) {
		ui->groupMode->setId(button, id++);
	}

	for (int dir = 0; dir < 4; ++dir) {
		ui->comboDirection->addItem(Stringizer::direction(dir), dir);
	}

	// Same speeds as the movement of events
	const QStringList speeds = { tr("1: 1/8x Slower"), tr("2: 1/4x Slower"), tr("3: 1/2x Slower"),
		tr("4: Normal"), tr("5: 2x Faster"), tr("6: 4x Faster") };
	for (int i = 0; i < speeds.size(); ++i) {
		ui->comboSpeed->addItem(speeds[i], i + 1);
	}

	connect(ui->groupMode, &QButtonGroup::idToggled, this, [this] { updateEnabled(); });
}

PanScreenWidget::~PanScreenWidget() {
	delete ui;
}

void PanScreenWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: operation (0: lock, 1: unlock, 2: pan, 3: reset), direction, distance, speed, wait.
	// Maniac Patch adds pixel pans (operations 4 to 7) and more parameters, they are kept.
	ensureParameters(cmd, 5);
	EventCommandBaseWidget::setData(cmd);

	bindRadios(ui->groupMode, m_cmd, 0);
	bindComboData(ui->comboDirection, m_cmd, 1);
	bindSpin(ui->spinDistance, m_cmd, 2);
	bindComboData(ui->comboSpeed, m_cmd, 3);
	bindCheck(ui->checkWait, m_cmd, 4);

	updateEnabled();
}

void PanScreenWidget::updateEnabled() {
	int mode = ui->groupMode->checkedId();
	bool pan = mode == 2;
	bool moves = mode == 2 || mode == 3;
	ui->comboDirection->setEnabled(pan);
	ui->spinDistance->setEnabled(pan);
	ui->comboSpeed->setEnabled(moves);
	ui->checkWait->setEnabled(moves);
}
