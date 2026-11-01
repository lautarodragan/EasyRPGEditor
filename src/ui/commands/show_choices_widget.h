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
#include <array>

class QButtonGroup;
class QLineEdit;

namespace Ui {
class ShowChoicesWidget;
}

/**
 * Editor of a Show Choice block:
 *
 * ShowChoice        string: "choice 1/choice 2/...", parameters: [0] cancel type
 * ShowChoiceOption  string: choice text, parameters: [0] choice index (4: cancel)
 *   ...commands of the choice (indent + 1)
 *   END (indent + 1)
 * ... one ShowChoiceOption per choice and one for the cancel branch
 * ShowChoiceEnd
 *
 * Cancel type: 0 disallowed, 1-4 behaves like choice 1-4, 5 separate branch
 */
class ShowChoicesWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	explicit ShowChoicesWidget(ProjectData& project, QWidget* parent);
	~ShowChoicesWidget() override;

	void setData(EventCommandList* commands) override;

	void apply();

private:
	struct Segment {
		// Index of the ShowChoiceOption and index past its last command
		int begin = -1;
		int end = -1;

		bool valid() const { return begin >= 0; }
	};

	static constexpr int cancel_disallow = 0;
	static constexpr int cancel_branch = 5;

	void parseBlock();
	void updateState();
	bool hasContent(const Segment& segment) const;
	std::vector<int> usedChoices() const;

	Ui::ShowChoicesWidget *ui;

	std::array<QLineEdit*, 4> m_edits;
	QButtonGroup* m_cancelGroup = nullptr;

	// Options of the existing block
	std::array<Segment, 4> m_choices;
	Segment m_cancel;
	int m_end = -1;
	int m_extraOptions = 0;
};
