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

#include "editor_check.h"
#include "event_commands_widget.h"
#include "core.h"
#include "model/project.h"
#include "model/project_data.h"
#include <lcf/rpg/map.h>
#include <QTextStream>

int checkEditors(const QString& project_dir) {
	QTextStream out(stdout);

	auto project = Project::load(project_dir);
	if (!project || !project->loadDatabaseAndMapTree()) {
		out << "Cannot load the project " << project_dir << Qt::endl;
		return 2;
	}
	core().project() = project;
	auto& data = project->projectData();

	EventCommandsWidget widget;
	int lists = 0;
	int opened = 0;
	int changed = 0;
	QStringList notes;

	auto check = [&](auto* holder, const QString& where) {
		++lists;
		const auto first_note = notes.size();
		widget.setData(data, holder);
		for (const auto& change : widget.checkEditorsRoundTrip(opened, notes)) {
			++changed;
			out << "CHANGED " << where << ": " << change << Qt::endl;
		}
		for (auto i = first_note; i < notes.size(); ++i) {
			notes[i] = where + ": " + notes[i];
		}
	};

	for (const auto& info : data.treeMap().maps) {
		if (info.ID == 0 || info.type != lcf::rpg::TreeMap::MapType_map) {
			continue;
		}
		auto map = project->loadMap(info.ID);
		if (!map) {
			out << "Cannot load map " << info.ID << Qt::endl;
			return 2;
		}
		for (auto& event : map->events) {
			for (auto& page : event.pages) {
				check(&page, QString("Map%1 event %2 page %3").arg(info.ID, 4, 10, QChar('0')).arg(event.ID).arg(page.ID));
			}
		}
	}

	auto& database = data.database();
	for (auto& event : database.commonevents) {
		check(&event, QString("Common event %1").arg(event.ID));
	}
	for (auto& troop : database.troops) {
		for (auto& page : troop.pages) {
			check(&page, QString("Troop %1 page %2").arg(troop.ID).arg(page.ID));
		}
	}

	for (const auto& note : notes) {
		out << "NOTE " << note << Qt::endl;
	}
	out << "Checked " << opened << " editors in " << lists << " command lists: "
		<< changed << " changed" << Qt::endl;

	return changed == 0 ? 0 : 1;
}
