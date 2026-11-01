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

#include "show_choices_widget.h"
#include "ui_show_choices_widget.h"
#include "common/dbstring.h"
#include "model/event_command_list.h"
#include <QButtonGroup>
#include <lcf/rpg/eventcommand.h>

using Cmd = lcf::rpg::EventCommand::Code;

ShowChoicesWidget::ShowChoicesWidget(ProjectData& project, QWidget* parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::ShowChoicesWidget) {

	ui->setupUi(this);

	m_edits = { ui->editChoice1, ui->editChoice2, ui->editChoice3, ui->editChoice4 };

	m_cancelGroup = new QButtonGroup(this);
	int i = 0;
	for (auto* button : { ui->buttonCancelDisallow, ui->buttonCancelChoice1, ui->buttonCancelChoice2,
			ui->buttonCancelChoice3, ui->buttonCancelChoice4, ui->buttonCancelBranch }) {
		m_cancelGroup->addButton(button, i++);
	}

	for (auto* edit : m_edits) {
		connect(edit, &QLineEdit::textChanged, this, &ShowChoicesWidget::updateState);
	}
	connect(m_cancelGroup, &QButtonGroup::idToggled, this, &ShowChoicesWidget::updateState);
}

ShowChoicesWidget::~ShowChoicesWidget() {
	delete ui;
}

void ShowChoicesWidget::setData(EventCommandList* commands) {
	EventCommandBaseWidget::setData(commands);

	parseBlock();

	const auto& cmds = m_commands->commands();
	if (m_end >= 0) {
		for (size_t i = 0; i < m_choices.size(); ++i) {
			if (m_choices[i].valid()) {
				m_edits[i]->setText(ToQString(cmds[m_choices[i].begin].string));
			}
		}
	} else {
		// No valid block follows: Take the choices from the string
		QStringList choices = ToQString(m_cmd->string).split('/');
		for (int i = 0; i < choices.size() && i < static_cast<int>(m_edits.size()); ++i) {
			m_edits[i]->setText(choices[i]);
		}
	}

	int cancel = m_cmd->parameters.empty() ? cancel_disallow : m_cmd->parameters[0];
	if (cancel < cancel_disallow || cancel > cancel_branch) {
		cancel = cancel_disallow;
	}
	m_cancelGroup->button(cancel)->setChecked(true);

	updateState();
}

void ShowChoicesWidget::parseBlock() {
	const auto& cmds = m_commands->commands();
	int index = m_commands->index();
	int indent = cmds[index].indent;

	m_choices = {};
	m_cancel = {};
	m_end = -1;
	m_extraOptions = 0;

	size_t next_choice = 0;
	Segment* current = nullptr;
	Segment ignored;

	for (int i = index + 1; i < static_cast<int>(cmds.size()); ++i) {
		const auto& cmd = cmds[i];

		if (cmd.indent > indent && current) {
			continue;
		}

		if (current) {
			current->end = i;
			current = nullptr;
		}

		if (cmd.indent != indent) {
			break;
		}

		auto code = static_cast<Cmd>(cmd.code);
		if (code == Cmd::ShowChoiceEnd) {
			m_end = i;
			break;
		} else if (code != Cmd::ShowChoiceOption) {
			break;
		}

		// Like the Player, the position of an option gives its choice,
		// the cancel branch has the index 4
		if (!cmd.parameters.empty() && cmd.parameters[0] >= 4 && !m_cancel.valid()) {
			current = &m_cancel;
		} else if (next_choice < m_choices.size() && (cmd.parameters.empty() || cmd.parameters[0] < 4)) {
			current = &m_choices[next_choice++];
		} else {
			++m_extraOptions;
			current = &ignored;
		}
		current->begin = i;
	}

	if (m_end < 0) {
		// The block is broken, leave the following commands alone
		m_choices = {};
		m_cancel = {};
		m_extraOptions = 0;
	}
}

bool ShowChoicesWidget::hasContent(const Segment& segment) const {
	// An option without commands contains only the terminating empty line
	return segment.valid() && segment.end - segment.begin > 2;
}

std::vector<int> ShowChoicesWidget::usedChoices() const {
	std::vector<int> used;
	for (size_t i = 0; i < m_edits.size(); ++i) {
		if (!m_edits[i]->text().isEmpty()) {
			used.push_back(static_cast<int>(i));
		}
	}
	if (used.empty()) {
		// A Show Choice needs at least one choice
		used.push_back(0);
	}
	return used;
}

void ShowChoicesWidget::updateState() {
	for (size_t i = 0; i < m_edits.size(); ++i) {
		m_cancelGroup->button(static_cast<int>(i) + 1)->setEnabled(!m_edits[i]->text().isEmpty());
	}

	QStringList warnings;
	auto used = usedChoices();
	for (size_t i = 0; i < m_choices.size(); ++i) {
		if (hasContent(m_choices[i]) && std::find(used.begin(), used.end(), static_cast<int>(i)) == used.end()) {
			warnings << tr("Choice %1 is empty: Its commands will be deleted.").arg(i + 1);
		}
	}
	if (hasContent(m_cancel) && m_cancelGroup->checkedId() != cancel_branch) {
		warnings << tr("The commands of the cancel branch will be deleted.");
	}
	int cancel = m_cancelGroup->checkedId();
	if (cancel >= 1 && cancel <= 4 && m_edits[cancel - 1]->text().isEmpty()) {
		warnings << tr("Choice %1 is empty: Cancelling will be disallowed.").arg(cancel);
	}
	if (m_extraOptions > 0) {
		warnings << tr("Options after the fourth choice will be deleted.");
	}
	if (m_end < 0) {
		warnings << tr("The options of this Show Choice are missing and will be added.");
	}

	ui->labelWarning->setText(warnings.join('\n'));
	ui->labelWarning->setVisible(!warnings.isEmpty());
}

void ShowChoicesWidget::apply() {
	auto& cmds = m_commands->commands();
	int index = m_commands->index();
	int indent = cmds[index].indent;

	auto make_cmd = [](Cmd code, int indent, const QString& text, std::vector<int32_t> params) {
		lcf::rpg::EventCommand cmd;
		cmd.code = static_cast<int32_t>(code);
		cmd.indent = indent;
		cmd.string = ToDBString(text);
		cmd.parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
		return cmd;
	};

	auto add_option = [&](std::vector<lcf::rpg::EventCommand>& block, const Segment& segment, const QString& text, int option) {
		if (segment.valid()) {
			// Keep the commands of the option
			size_t first = block.size();
			block.insert(block.end(), cmds.begin() + segment.begin, cmds.begin() + segment.end);
			auto& opt = block[first];
			opt.string = ToDBString(text);
			if (opt.parameters.empty()) {
				opt.parameters = lcf::DBArray<int32_t>(1);
			}
			opt.parameters[0] = option;
		} else {
			block.push_back(make_cmd(Cmd::ShowChoiceOption, indent, text, {option}));
			block.push_back(make_cmd(Cmd::END, indent + 1, QString(), {}));
		}
	};

	auto used = usedChoices();
	int cancel = m_cancelGroup->checkedId();
	if (cancel >= 1 && cancel <= 4) {
		// Cancelling selects the choice at the same position after removing the empty ones
		auto it = std::find(used.begin(), used.end(), cancel - 1);
		if (it == used.end() || m_edits[cancel - 1]->text().isEmpty()) {
			cancel = cancel_disallow;
		} else {
			cancel = static_cast<int>(it - used.begin()) + 1;
		}
	}

	std::vector<lcf::rpg::EventCommand> block;
	QStringList texts;

	block.push_back(cmds[index]);
	for (size_t i = 0; i < used.size(); ++i) {
		QString text = m_edits[used[i]]->text();
		texts << text;
		add_option(block, m_choices[used[i]], text, static_cast<int>(i));
	}
	if (cancel == cancel_branch) {
		add_option(block, m_cancel, QString(), 4);
	}
	if (m_end >= 0) {
		block.push_back(cmds[m_end]);
	} else {
		block.push_back(make_cmd(Cmd::ShowChoiceEnd, indent, QString(), {}));
	}

	auto& choice = block[0];
	choice.string = ToDBString(texts.join('/'));
	if (choice.parameters.empty()) {
		choice.parameters = lcf::DBArray<int32_t>(1);
	}
	choice.parameters[0] = cancel;

	// Replace the old block
	int end = m_end >= 0 ? m_end + 1 : index + 1;
	cmds.erase(cmds.begin() + index, cmds.begin() + end);
	cmds.insert(cmds.begin() + index, block.begin(), block.end());

	// The block changed: Parse it again and show the choices
	// without the empty ones for further applies
	m_cmd = &m_commands->command();
	parseBlock();
	for (size_t i = 0; i < m_edits.size(); ++i) {
		m_edits[i]->setText(static_cast<qsizetype>(i) < texts.size() ? texts[i] : QString());
	}
	m_cancelGroup->button(cancel)->setChecked(true);
	updateState();
}
