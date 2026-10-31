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

#include "show_message_widget.h"
#include "ui_show_message_widget.h"
#include "common/dbstring.h"
#include "model/event_command_list.h"
#include "lcf/rpg/eventcommand.h"

ShowMessageWidget::ShowMessageWidget(ProjectData &project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ShowMessageWidget) {

	ui->setupUi(this);
}

ShowMessageWidget::~ShowMessageWidget() {
	delete ui;
}

void ShowMessageWidget::setData(EventCommandList* commands) {
	EventCommandBaseWidget::setData(commands);

	using Cmd = lcf::rpg::EventCommand::Code;

	ui->message->append(ToQString(m_cmd->string));

	for (size_t i = commands->index() + 1; i < commands->size(); ++i) {
		auto& cur_cmd = commands->commands()[i];

		if (static_cast<Cmd>(cur_cmd.code) != Cmd::ShowMessage_2) {
			break;
		}

		ui->message->append(ToQString(cur_cmd.string));
	}
}

void ShowMessageWidget::apply() {
	using Cmd = lcf::rpg::EventCommand::Code;

	auto& commands = m_commands->commands();
	size_t index = m_commands->index();

	// Remove the old additional lines
	size_t end = index + 1;
	while (end < commands.size() && static_cast<Cmd>(commands[end].code) == Cmd::ShowMessage_2) {
		++end;
	}
	commands.erase(commands.begin() + index + 1, commands.begin() + end);

	// The first line belongs to the ShowMessage, each other line is a ShowMessage_2
	QStringList text = lines();
	commands[index].string = ToDBString(text[0]);

	std::vector<lcf::rpg::EventCommand> additional;
	for (qsizetype i = 1; i < text.size(); ++i) {
		lcf::rpg::EventCommand cmd;
		cmd.code = static_cast<int32_t>(Cmd::ShowMessage_2);
		cmd.indent = commands[index].indent;
		cmd.string = ToDBString(text[i]);
		additional.push_back(cmd);
	}
	commands.insert(commands.begin() + index + 1, additional.begin(), additional.end());

	// The insertion may have moved the commands
	m_cmd = &m_commands->command();
}

void ShowMessageWidget::textChanged() {
	// The message window of RPG Maker shows four lines
	if (lines().size() > 4) {
		ui->labelWarning->setText(tr("RPG Maker shows up to 4 lines per message."));
	} else {
		ui->labelWarning->clear();
	}
}

QStringList ShowMessageWidget::lines() const {
	return ui->message->toPlainText().split('\n');
}
