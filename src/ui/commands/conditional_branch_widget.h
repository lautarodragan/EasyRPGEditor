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
#include <lcf/rpg/eventcommand.h>

class QButtonGroup;

namespace Ui {
class ConditionalBranchWidget;
}

/**
 * Edits a Conditional Branch.
 * The widget works on a copy of the command and writes it back in apply(),
 * where it also adds or removes the Else Branch of the command.
 */
class ConditionalBranchWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	explicit ConditionalBranchWidget(ProjectData& project, QWidget* parent);
	~ConditionalBranchWidget() override;

	using EventCommandBaseWidget::setData;
	void setData(EventCommandList* commands) override;

	void apply();

private:
	void loadParameters();
	void connectWidgets();
	void writeParameters();
	void updateEnabled();
	void updateElseWarning();

	Ui::ConditionalBranchWidget* ui;
	QButtonGroup* m_type_group;
	QButtonGroup* m_actor_group;
	lcf::rpg::EventCommand m_edit;
};
