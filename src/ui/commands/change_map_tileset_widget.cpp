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

#include "change_map_tileset_widget.h"
#include "ui_change_map_tileset_widget.h"
#include "param_bindings.h"
#include "common/dbstring.h"
#include "model/project_data.h"

using namespace ParamBindings;

ChangeMapTilesetWidget::ChangeMapTilesetWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ChangeMapTilesetWidget) {

	ui->setupUi(this);

	// ChipsetRpgComboBox does not build yet (RpgReflect names a ChipsetWidget, the class is
	// ChipSetWidget), so the chipsets are listed by hand
	const auto& chipsets = project.database().chipsets;
	for (size_t i = 0; i < chipsets.size(); ++i) {
		ui->comboChipset->addItem(QString("%1: %2").arg(i + 1, 4, 10, QChar('0')).arg(ToQString(chipsets[i].name)), static_cast<int>(i + 1));
	}
}

ChangeMapTilesetWidget::~ChangeMapTilesetWidget() {
	delete ui;
}

void ChangeMapTilesetWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Parameters: chipset ID (a second one in Maniac Patch is kept)
	ensureParameters(cmd, 1);
	EventCommandBaseWidget::setData(cmd);

	bindComboData(ui->comboChipset, m_cmd, 0);
}
