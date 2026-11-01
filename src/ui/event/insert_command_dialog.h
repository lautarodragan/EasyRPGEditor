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

#include <QDialog>
#include "model/event_command_templates.h"

class QDialogButtonBox;
class QLineEdit;
class QTreeWidget;

/**
 * Lets the user choose the command to insert into an event, grouped by category.
 */
class InsertCommandDialog : public QDialog
{
	Q_OBJECT

public:
	InsertCommandDialog(bool rpg2k3, bool battle, QWidget* parent = nullptr);

	/** The chosen command, nullptr when none is selected */
	const EventCommandTemplates::Template* selected() const;

private:
	void filter(const QString& text);
	void updateButtons();

	QLineEdit* m_filter = nullptr;
	QTreeWidget* m_tree = nullptr;
	QDialogButtonBox* m_buttons = nullptr;
};
