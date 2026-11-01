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

#include "erase_picture_widget.h"
#include "ui_erase_picture_widget.h"
#include <QSignalBlocker>

ErasePictureWidget::ErasePictureWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ErasePictureWidget) {

	ui->setupUi(this);
}

ErasePictureWidget::~ErasePictureWidget() {
	delete ui;
}

void ErasePictureWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: picture ID, then RPG Maker 2003 v1.12 and the Maniac Patch add
	// a mode (0: picture, 1: variable, 2: range, 3: variable range, 4: indirect, 5: all)
	// and the end of the range. Only the plain picture ID is edited here.
	if (cmd->parameters.empty()) {
		cmd->parameters = lcf::DBArray<int32_t>(1, 1);
	}

	EventCommandBaseWidget::setData(cmd);

	int mode = cmd->parameters.size() > 1 ? cmd->parameters[1] : 0;
	int end = cmd->parameters.size() > 2 ? cmd->parameters[2] : 0;
	{
		QSignalBlocker blocker(ui->spinNumber);
		ui->spinNumber->setValue(cmd->parameters[0]);
	}
	connect(ui->spinNumber, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
		m_cmd->parameters[0] = value;
	});

	QString note;
	switch (mode) {
		case 0:
			break;
		case 1:
			note = tr("Picture in variable %1").arg(cmd->parameters[0]);
			break;
		case 2:
			note = tr("Pictures %1 to %2").arg(cmd->parameters[0]).arg(end);
			break;
		case 3:
			note = tr("Pictures in variable %1 to variable %2").arg(cmd->parameters[0]).arg(end);
			break;
		case 4:
			note = tr("Picture in the variable named by variable %1").arg(cmd->parameters[0]);
			break;
		case 5:
			note = tr("All pictures");
			break;
		default:
			note = tr("Unknown mode %1").arg(mode);
			break;
	}
	ui->labelNote->setVisible(!note.isEmpty());
	ui->spinNumber->setEnabled(note.isEmpty());
	if (!note.isEmpty()) {
		ui->labelNote->setText(note + "\n" + tr("(RPG Maker 2003 v1.12 or Maniac Patch, kept as is)"));
	}
}
