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

#include <QTreeWidget>
#include <lcf/rpg/commonevent.h>
#include <lcf/rpg/trooppage.h>
#include <lcf/rpg/eventpage.h>

class ProjectData;

class EventCommandsWidget : public QTreeWidget
{
	Q_OBJECT

public:
	explicit EventCommandsWidget(QWidget *parent = nullptr);

	void setData(ProjectData& project, lcf::rpg::CommonEvent* event);
	void setData(ProjectData& project, lcf::rpg::TroopPage* event);
	void setData(ProjectData& project, lcf::rpg::EventPage* event);

	/**
	 * Opens the editor of every command and accepts it without changes,
	 * which must leave the commands as they were (see --check-editors).
	 *
	 * @param opened Incremented for each editor opened
	 * @param notes Receives the message boxes the editors showed
	 * @return A description of each command that an editor changed
	 */
	QStringList checkEditorsRoundTrip(int& opened, QStringList& notes);

private:
	template<typename T>
	void setDataInternal(ProjectData& project, T* event);
	std::vector<lcf::rpg::EventCommand>	* m_commands = nullptr;

	void addRawEvent(QTreeWidgetItem* item);
	void editRawEvent(QTreeWidgetItem* item, int column, bool show_warning);
	void insertCommand(QTreeWidgetItem* item);
	void deleteEvent(QTreeWidgetItem* item);
	bool canInsertBefore(QTreeWidgetItem* item) const;
	bool canDelete(QTreeWidgetItem* item) const;
	/** The empty line at the end of the list, it's not a command */
	bool isEndLine(QTreeWidgetItem* item) const;
	void refreshList();
	void selectIndex(int index);

	/** Result of openEditor for commands without an editor */
	static constexpr int NoEditor = -1;
	/** Opens the editor of the command at index, returns the dialog result or NoEditor */
	int openEditor(int index);
	int openRawEditor(int index, bool show_warning);

	ProjectData* m_project = nullptr;
	/** Commands of a troop page, they have their own set of commands */
	bool m_battle = false;

protected:
	void keyPressEvent(QKeyEvent* event) override;

private slots:
	void editEvent(QTreeWidgetItem* item, int column);
	void showContextMenu(const QPoint& pos);
};
