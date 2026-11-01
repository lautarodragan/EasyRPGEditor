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

#include "change_pbg_widget.h"
#include "ui_change_pbg_widget.h"
#include "param_bindings.h"
#include "defines.h"
#include "common/dbstring.h"
#include "ui/picker/picker_dialog.h"
#include "ui/picker/picker_panorama_widget.h"

using namespace ParamBindings;

ChangePBGWidget::ChangePBGWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ChangePBGWidget) {

	ui->setupUi(this);

	connect(ui->pushPick, &QPushButton::clicked, this, &ChangePBGWidget::pickFile);
	for (auto* check : { ui->checkHorizontal, ui->checkHorizontalAuto, ui->checkVertical, ui->checkVerticalAuto }) {
		connect(check, &QCheckBox::toggled, this, &ChangePBGWidget::updateEnabled);
	}
}

ChangePBGWidget::~ChangePBGWidget() {
	delete ui;
}

void ChangePBGWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: horizontal scroll, vertical scroll, horizontal autoscroll, its speed,
	// vertical autoscroll, its speed (a seventh one of Maniac Patch is kept)
	ensureParameters(cmd, 6);
	EventCommandBaseWidget::setData(cmd);

	ui->lineFile->setText(ToQString(m_cmd->string));
	bindCheck(ui->checkHorizontal, m_cmd, 0);
	bindCheck(ui->checkVertical, m_cmd, 1);
	bindCheck(ui->checkHorizontalAuto, m_cmd, 2);
	bindSpin(ui->spinHorizontalSpeed, m_cmd, 3);
	bindCheck(ui->checkVerticalAuto, m_cmd, 4);
	bindSpin(ui->spinVerticalSpeed, m_cmd, 5);

	updateEnabled();
}

void ChangePBGWidget::updateEnabled() {
	// Autoscroll needs scrolling, the speed needs autoscroll
	ui->checkHorizontalAuto->setEnabled(ui->checkHorizontal->isChecked());
	ui->spinHorizontalSpeed->setEnabled(ui->checkHorizontal->isChecked() && ui->checkHorizontalAuto->isChecked());
	ui->checkVerticalAuto->setEnabled(ui->checkVertical->isChecked());
	ui->spinVerticalSpeed->setEnabled(ui->checkVertical->isChecked() && ui->checkVerticalAuto->isChecked());
}

void ChangePBGWidget::pickFile() {
	auto* widget = new PickerPanoramaWidget(this);
	PickerDialog dialog(m_project, FileFinder::FileType::Image, widget, this);
	connect(&dialog, &PickerDialog::fileSelected, this, [this](const QString& baseName) {
		m_cmd->string = ToDBString(baseName);
		ui->lineFile->setText(baseName);
	});
	dialog.setDirectoryAndFile(PANORAMA, ToQString(m_cmd->string));
	dialog.exec();
}
