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

class QLabel;

/**
 * Base of the editors of commands with branches (battle, shop and inn
 * handlers, else branch).
 * The editor works on a copy of the command, apply() writes it back and
 * gives the block the branches its parameters need, keeping the commands
 * of the remaining branches. The block is only changed after
 * branchesChanged(), parameters that aren't edited keep their values.
 */
class BlockCommandWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	BlockCommandWidget(ProjectData& project, QWidget* parent);

	using EventCommandBaseWidget::setData;
	void setData(EventCommandList* commands) override;

	void apply();

protected:
	/** Shows the parameters of m_edit, called by setData */
	virtual void load() = 0;

	/** Call after the user changed a parameter that decides the branches */
	void branchesChanged();

	/** The label showing which branches lose their commands, to be placed by the subclass */
	QLabel* warningLabel() const { return m_warning; }

	/** Whether the command has its block (head, branches and closing line) */
	bool hasBlock() const;

	lcf::rpg::EventCommand m_edit;

private:
	void updateWarning();

	QLabel* m_warning;
	bool m_branches_changed = false;
};
