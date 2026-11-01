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

#include "comment_widget.h"
#include "ui_comment_widget.h"
#include "common/dbstring.h"
#include "model/event_command_list.h"
#include <lcf/rpg/eventcommand.h>

using Cmd = lcf::rpg::EventCommand::Code;

CommentWidget::CommentWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::CommentWidget) {

	ui->setupUi(this);
}

CommentWidget::~CommentWidget() {
	delete ui;
}

void CommentWidget::setData(EventCommandList* commands) {
	EventCommandBaseWidget::setData(commands);

	QStringList text = { ToQString(m_cmd->string) };

	for (size_t i = commands->index() + 1; i < commands->size(); ++i) {
		auto& cur_cmd = commands->commands()[i];

		if (static_cast<Cmd>(cur_cmd.code) != Cmd::Comment_2) {
			break;
		}

		text.append(ToQString(cur_cmd.string));
	}

	ui->comment->setPlainText(text.join('\n'));
}

void CommentWidget::apply() {
	auto& commands = m_commands->commands();
	size_t index = m_commands->index();

	// Remove the old additional lines
	size_t end = index + 1;
	while (end < commands.size() && static_cast<Cmd>(commands[end].code) == Cmd::Comment_2) {
		++end;
	}
	commands.erase(commands.begin() + index + 1, commands.begin() + end);

	// The first line belongs to the Comment, each other line is a Comment_2
	QStringList text = ui->comment->toPlainText().split('\n');
	commands[index].string = ToDBString(text[0]);

	std::vector<lcf::rpg::EventCommand> additional;
	for (qsizetype i = 1; i < text.size(); ++i) {
		lcf::rpg::EventCommand cmd;
		cmd.code = static_cast<int32_t>(Cmd::Comment_2);
		cmd.indent = commands[index].indent;
		cmd.string = ToDBString(text[i]);
		additional.push_back(cmd);
	}
	commands.insert(commands.begin() + index + 1, additional.begin(), additional.end());

	// The insertion may have moved the commands
	m_cmd = &m_commands->command();
}
