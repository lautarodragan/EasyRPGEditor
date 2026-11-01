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
#include <QStringList>

namespace Ui {
class CommentWidget;
}

/**
 * Editor of a Comment: the first line is the Comment,
 * each further line is a Comment_2 that follows it.
 */
class CommentWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	explicit CommentWidget(ProjectData& project, QWidget* parent);
	~CommentWidget() override;

	void setData(EventCommandList* commands) override;

	void apply();

private:
	Ui::CommentWidget *ui;
};
