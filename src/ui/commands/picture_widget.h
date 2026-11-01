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
#include <functional>

class QCheckBox;
class QComboBox;
class QSpinBox;

namespace Ui {
class PictureWidget;
}

/**
 * Editor for Show Picture and Move Picture, which share most of their parameters.
 * Only parameters changed in the dialog are written: the others, also the ones of
 * later engine versions and patches, are kept as they are.
 */
class PictureWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	enum class Kind { Show, Move };

	PictureWidget(ProjectData& project, QWidget* parent, Kind kind);
	~PictureWidget() override;

	void setData(lcf::rpg::EventCommand* cmd) override;

private:
	int param(int index) const;
	void setParam(int index, int value);
	/** Writes value into the bits of mask, keeping the other bits */
	void setParamBits(int index, int mask, int value);

	void bindSpin(QSpinBox* spin, int index);
	void bindCheck(QCheckBox* check, int index);
	void bindFlag(QCheckBox* check, int index, int bit);
	void bindCombo(QComboBox* combo, int index, std::function<void(int)> changed = {});

	void writePosition();
	void updateEnabled();
	void pickFile();

	Ui::PictureWidget* ui;
	Kind m_kind;
	bool m_v112 = false;
};

class ShowPictureWidget : public PictureWidget
{
	Q_OBJECT
public:
	ShowPictureWidget(ProjectData& project, QWidget* parent) : PictureWidget(project, parent, Kind::Show) {}
};

class MovePictureWidget : public PictureWidget
{
	Q_OBJECT
public:
	MovePictureWidget(ProjectData& project, QWidget* parent) : PictureWidget(project, parent, Kind::Move) {}
};
