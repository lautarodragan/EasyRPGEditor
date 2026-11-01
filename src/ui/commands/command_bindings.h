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

#include <lcf/rpg/eventcommand.h>
#include <QString>
#include <functional>

class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QLineEdit;
class QSpinBox;
class ProjectData;
class QWidget;
class RpgComboBoxBase;

/**
 * Connects the controls of a command editor to the parameters of the command.
 *
 * A control shows the parameter, which is only written when the user changes
 * the control: parameters that aren't edited stay as they are, also values
 * the control can't show. Writing a parameter past the end of the list
 * extends it with zeros.
 * The callback runs after the parameter was written.
 */
namespace CommandBindings {
	using Changed = std::function<void()>;

	/** The parameter, or the fallback when the command has fewer parameters */
	int32_t param(const lcf::rpg::EventCommand& cmd, size_t index, int32_t fallback = 0);

	/** Writes the parameter, extending the list with zeros when needed */
	void setParam(lcf::rpg::EventCommand& cmd, size_t index, int32_t value);

	void bind(QSpinBox* spin, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback = 0, Changed changed = {});

	/** Checked when the parameter isn't 0, writes 1 or 0 */
	void bind(QAbstractButton* check, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback = 0, Changed changed = {});

	/** The buttons have the parameter values as IDs, no button is checked for other values */
	void bind(QButtonGroup* group, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback = 0, Changed changed = {});

	/**
	 * The item data are the parameter values.
	 * Another value is added as an item with the text from unknownText.
	 */
	void bind(QComboBox* combo, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback = 0, Changed changed = {});

	/** A combo box of database entries, the first row has ID 1 */
	void bind(RpgComboBoxBase* combo, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback = 1, Changed changed = {});

	/** The string of the command */
	void bindString(QLineEdit* edit, lcf::rpg::EventCommand* cmd, Changed changed = {});

	/** Adds the items with the values first to first + count - 1 as data, shown as value + offset */
	void addNumbers(QComboBox* combo, int first, int count, int offset = 0, const QString& format = QStringLiteral("%1"));

	/** Selects the item with the value as data, adding it when it is missing */
	void selectData(QComboBox* combo, int32_t value);

	QString unknownText(int32_t value);

	/** Lets the user choose a battle background (Backdrop folder) for the line edit */
	void pickBackdrop(ProjectData& project, QLineEdit* edit, QWidget* parent);

	/** Adds the members of a troop (0 to 7) as Monster 1 to Monster 8 */
	void addMonsters(QComboBox* combo);
}
