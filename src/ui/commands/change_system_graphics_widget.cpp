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

#include "change_system_graphics_widget.h"
#include "ui_change_system_graphics_widget.h"
#include "param_bindings.h"
#include "defines.h"
#include "common/dbstring.h"
#include "ui/picker/picker_dialog.h"
#include "ui/picker/picker_backdrop_widget.h"

using namespace ParamBindings;

ChangeSystemGraphicsWidget::ChangeSystemGraphicsWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ChangeSystemGraphicsWidget) {

	ui->setupUi(this);

	ui->groupWallpaperButtons->setId(ui->radioStretch, 0);
	ui->groupWallpaperButtons->setId(ui->radioTile, 1);
	ui->groupFontButtons->setId(ui->radioFont1, 0);
	ui->groupFontButtons->setId(ui->radioFont2, 1);

	connect(ui->pushPick, &QPushButton::clicked, this, &ChangeSystemGraphicsWidget::pickFile);
}

ChangeSystemGraphicsWidget::~ChangeSystemGraphicsWidget() {
	delete ui;
}

void ChangeSystemGraphicsWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: window background (0: stretch, 1: tile), font
	// (a third one of Maniac Patch is kept)
	ensureParameters(cmd, 2);
	EventCommandBaseWidget::setData(cmd);

	ui->lineFile->setText(ToQString(m_cmd->string));
	bindRadios(ui->groupWallpaperButtons, m_cmd, 0);
	bindRadios(ui->groupFontButtons, m_cmd, 1);
}

void ChangeSystemGraphicsWidget::pickFile() {
	auto* widget = new PickerBackdropWidget(this);
	PickerDialog dialog(m_project, FileFinder::FileType::Image, widget, this);
	connect(&dialog, &PickerDialog::fileSelected, this, [this](const QString& baseName) {
		m_cmd->string = ToDBString(baseName);
		ui->lineFile->setText(baseName);
	});
	dialog.setDirectoryAndFile(SYSTEM, ToQString(m_cmd->string));
	dialog.exec();
}
