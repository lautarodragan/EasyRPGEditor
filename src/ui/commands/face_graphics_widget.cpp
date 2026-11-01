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

#include "face_graphics_widget.h"
#include "ui_face_graphics_widget.h"

#include "common/dbstring.h"
#include "defines.h"
#include "ui/picker/picker_dialog.h"
#include "ui/picker/picker_faceset_widget.h"
#include "ui/viewer/faceset_graphics_item.h"

FaceGraphicsWidget::FaceGraphicsWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::FaceGraphicsWidget) {

	ui->setupUi(this);

	ui->groupPosition_arg1->setId(ui->buttonPositionLeft, 0);
	ui->groupPosition_arg1->setId(ui->buttonPositionRight, 1);

	m_faceItem = new FaceSetGraphicsItem(project);
	ui->viewFace->scale(2.0, 2.0);
	ui->viewFace->setItem(m_faceItem);
	QObject::connect(ui->viewFace, &RpgGraphicsViewBase::clicked, this, &FaceGraphicsWidget::faceSetClicked);
	QObject::connect(ui->buttonSet, &QPushButton::clicked, this, &FaceGraphicsWidget::faceSetClicked);
	QObject::connect(ui->buttonErase, &QPushButton::clicked, this, &FaceGraphicsWidget::eraseClicked);
}

FaceGraphicsWidget::~FaceGraphicsWidget() {
	delete ui;
}

void FaceGraphicsWidget::setData(lcf::rpg::EventCommand* cmd) {
	// The parameter handlers add missing parameters
	EventCommandBaseWidget::setData(cmd);

	refreshFace();
}

void FaceGraphicsWidget::onParameterChanged(int index, int) {
	if (index == 2) {
		refreshFace();
	}
}

void FaceGraphicsWidget::faceSetClicked() {
	auto* widget = new PickerFacesetWidget(m_cmd->parameters[0], this);
	PickerDialog dialog(m_project, FileFinder::FileType::Image, widget, this);
	QObject::connect(&dialog, &PickerDialog::fileSelected, this, [&](const QString& baseName) {
		m_cmd->string = ToDBString(baseName);
		m_cmd->parameters[0] = widget->index();
		refreshFace();
	});
	dialog.setDirectoryAndFile(FACESET, ToQString(m_cmd->string));
	dialog.exec();
}

void FaceGraphicsWidget::eraseClicked() {
	m_cmd->string = lcf::DBString();
	m_cmd->parameters[0] = 0;
	refreshFace();
}

void FaceGraphicsWidget::refreshFace() {
	QString name = ToQString(m_cmd->string);
	m_faceItem->refresh(name, m_cmd->parameters[0]);
	m_faceItem->setFlip(m_cmd->parameters[2] != 0);
	ui->buttonErase->setEnabled(!name.isEmpty());

	if (name.isEmpty()) {
		ui->labelFace->setText(tr("No face (erases the face graphic)"));
	} else {
		ui->labelFace->setText(tr("%1, index %2").arg(name).arg(m_cmd->parameters[0]));
	}
}
