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

#pragma once

#include "ui/event/event_command_base_widget.h"

class FaceSetGraphicsItem;

namespace Ui {
class FaceGraphicsWidget;
}

/**
 * Editor of Change Face Graphic:
 * string: FaceSet file (empty erases the face)
 * parameters: [0] face index, [1] position (0 left, 1 right), [2] flipped
 */
class FaceGraphicsWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	explicit FaceGraphicsWidget(ProjectData& project, QWidget* parent);
	~FaceGraphicsWidget() override;

	void setData(lcf::rpg::EventCommand* cmd) override;

	void onParameterChanged(int index, int new_value) override;

private:
	void faceSetClicked();
	void eraseClicked();
	void refreshFace();

	Ui::FaceGraphicsWidget *ui;

	FaceSetGraphicsItem* m_faceItem = nullptr;
};
