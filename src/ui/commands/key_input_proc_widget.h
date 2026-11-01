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
#include <vector>

class QCheckBox;
class RpgComboBoxBase;

/**
 * Edits a Key Input Processing.
 * The parameters of the keys depend on the engine and on the number of
 * parameters (RPG Maker 2000 before and after v1.50, RPG Maker 2003 before
 * and after v1.05, RPG Maker 2000 commands in RPG Maker 2003), like the
 * Player reads them.
 */
class KeyInputProcWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	KeyInputProcWidget(ProjectData& project, QWidget* parent);

	using EventCommandBaseWidget::setData;
	void setData(lcf::rpg::EventCommand* cmd) override;

private:
	void updateEnabled();

	bool m_2k3;
	RpgComboBoxBase* m_variable;
	QCheckBox* m_wait;
	QCheckBox* m_decision;
	QCheckBox* m_cancel;
	QCheckBox* m_shift;
	QCheckBox* m_directions;
	QCheckBox* m_down;
	QCheckBox* m_left;
	QCheckBox* m_right;
	QCheckBox* m_up;
	QCheckBox* m_numbers;
	QCheckBox* m_operators;
	QCheckBox* m_timed;
	RpgComboBoxBase* m_time_variable;
};
