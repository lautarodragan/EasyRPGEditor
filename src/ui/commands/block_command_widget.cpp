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

#include "block_command_widget.h"
#include "model/event_command_list.h"
#include "model/event_command_templates.h"
#include "stringizer.h"
#include <QLabel>

namespace T = EventCommandTemplates;

BlockCommandWidget::BlockCommandWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent) {
	m_warning = new QLabel(this);
	m_warning->setWordWrap(true);
	m_warning->setStyleSheet(QStringLiteral("color: #c0392b;"));
	m_warning->hide();
}

void BlockCommandWidget::setData(EventCommandList* commands) {
	m_commands = commands;
	// Edit a copy: the command list is shared with the event, Cancel must not change it
	m_edit = commands->command();
	m_cmd = &m_edit;
	load();
	updateWarning();
}

bool BlockCommandWidget::hasBlock() const {
	return T::parseBlock(m_commands->commands(), m_commands->index()).end_line != -1;
}

void BlockCommandWidget::branchesChanged() {
	m_branches_changed = true;
	updateWarning();
}

void BlockCommandWidget::updateWarning() {
	QStringList lines;
	if (m_branches_changed) {
		const auto& commands = m_commands->commands();
		for (auto code : T::branchesLosingContent(commands, m_commands->index(), T::requiredBranches(m_edit))) {
			lcf::rpg::EventCommand branch;
			branch.code = static_cast<int32_t>(code);
			lines << tr("The commands of the branch \"%1\" will be deleted.").arg(Stringizer::stringize(branch));
		}
	}
	m_warning->setText(lines.join('\n'));
	m_warning->setVisible(!lines.isEmpty());
}

void BlockCommandWidget::apply() {
	auto& commands = m_commands->commands();
	const int index = m_commands->index();
	m_edit.indent = commands[index].indent;
	commands[index] = m_edit;

	if (m_branches_changed) {
		T::setBranches(commands, index, T::requiredBranches(m_edit));
	}
	updateWarning();
}
