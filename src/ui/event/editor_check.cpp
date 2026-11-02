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
#include "event_dialog.h"
#include "core.h"
#include "model/project.h"
#include "model/project_data.h"
#include "ui/database/database_dialog.h"
#include "ui/maptree/map_properties_dialog.h"
#include <lcf/ldb/reader.h>
#include <lcf/lmt/reader.h>
#include <lcf/lmu/reader.h>
#include <lcf/rpg/map.h>
#include <lcf/saveopt.h>
#include <QPushButton>
#include <QApplication>
#include <QDialogButtonBox>
#include <QListView>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <sstream>

namespace {
/**
 * Runs a modal dialog and presses its OK button, as a user accepting it
 * without changes. Message boxes are answered with No/Cancel and reported.
 */
void execAndAccept(QDialog& dialog, QStringList& notes, const QString& where) {
	QTimer timer;
	timer.setInterval(0);
	QObject::connect(&timer, &QTimer::timeout, [&] {
		auto* active = qobject_cast<QDialog*>(QApplication::activeModalWidget());
		if (!active) {
			return;
		}
		if (auto* box = qobject_cast<QMessageBox*>(active)) {
			notes << QString("%1: message box \"%2\"").arg(where, box->text());
			box->reject();
			return;
		}
		auto* buttons = active->findChild<QDialogButtonBox*>();
		if (auto* ok = buttons ? buttons->button(QDialogButtonBox::Ok) : nullptr) {
			ok->click();
		} else {
			active->accept();
		}
	});
	timer.start();
	dialog.exec();
}

/** First lines that differ between two texts, for a readable report */
QString firstDifferences(const std::string& before, const std::string& after) {
	const QStringList a = QString::fromStdString(before).split('\n');
	const QStringList b = QString::fromStdString(after).split('\n');
	QStringList out;
	for (int i = 0; i < std::max(a.size(), b.size()) && out.size() < 6; ++i) {
		const QString la = i < a.size() ? a[i].trimmed() : QString("(none)");
		const QString lb = i < b.size() ? b[i].trimmed() : QString("(none)");
		if (la != lb) {
			out << QString("    line %1: %2  ->  %3").arg(i + 1).arg(la.left(160), lb.left(160));
		}
	}
	return out.join('\n');
}

std::string mapXml(const lcf::rpg::Map& map, lcf::EngineVersion engine) {
	std::ostringstream xml;
	lcf::LMU_Reader::SaveXml(xml, map, engine);
	return xml.str();
}
}

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

	out << "Checked " << opened << " editors in " << lists << " command lists: "
		<< changed << " changed" << Qt::endl;

	const auto engine = lcf::GetEngineVersion(database);
	int dialogs_changed = 0;

	// Event dialog of every event: its pages, conditions, graphic, movement...
	int events = 0;
	int events_changed = 0;
	for (const auto& info : data.treeMap().maps) {
		if (info.ID == 0 || info.type != lcf::rpg::TreeMap::MapType_map) {
			continue;
		}
		auto map = project->loadMap(info.ID);
		for (auto& event : map->events) {
			const QString where = QString("Map%1 event %2").arg(info.ID, 4, 10, QChar('0')).arg(event.ID);
			EventDialog dialog(data);
			dialog.setEvent(event);
			execAndAccept(dialog, notes, where);
			++events;
			const auto edited = dialog.getEvent();
			if (!(edited == event)) {
				++events_changed;
				lcf::rpg::Map a, b;
				a.events = { event };
				b.events = { edited };
				out << "CHANGED " << where << " (event dialog):\n" << firstDifferences(mapXml(a, engine), mapXml(b, engine)) << Qt::endl;
			}
		}
	}
	out << "Checked " << events << " event dialogs: " << events_changed << " changed" << Qt::endl;
	dialogs_changed += events_changed;

	// Map properties of every map
	int maps = 0;
	int maps_changed = 0;
	for (const auto& info : data.treeMap().maps) {
		if (info.ID == 0 || info.type != lcf::rpg::TreeMap::MapType_map) {
			continue;
		}
		const QString where = QString("Map%1").arg(info.ID, 4, 10, QChar('0'));
		auto original = project->loadMap(info.ID);
		auto map = *original;
		auto edited_info = info;
		MapPropertiesDialog dialog(data, edited_info, map);
		execAndAccept(dialog, notes, where);
		++maps;
		if (!(map == *original)) {
			++maps_changed;
			out << "CHANGED " << where << " (map properties, map):\n"
				<< firstDifferences(mapXml(*original, engine), mapXml(map, engine)) << Qt::endl;
		}
		if (!(edited_info == info)) {
			++maps_changed;
			lcf::rpg::TreeMap a, b;
			a.maps = { info };
			b.maps = { edited_info };
			std::ostringstream xa, xb;
			lcf::LMT_Reader::SaveXml(xa, a, engine);
			lcf::LMT_Reader::SaveXml(xb, b, engine);
			out << "CHANGED " << where << " (map properties, map tree):\n" << firstDifferences(xa.str(), xb.str()) << Qt::endl;
		}
	}
	out << "Checked " << maps << " map properties dialogs: " << maps_changed << " changed" << Qt::endl;
	dialogs_changed += maps_changed;

	// Database: selecting an entry in a list loads it into its editor
	{
		DatabaseDialog dialog(data);
		int entries = 0;
		for (auto* list : dialog.findChildren<QListView*>()) {
			auto* model = list->model();
			if (!model) {
				continue;
			}
			for (int row = 0; row < model->rowCount(); ++row) {
				list->setCurrentIndex(model->index(row, 0));
				++entries;
			}
		}
		const bool db_changed = !(dialog.editedData().database() == database);
		out << "Checked the database dialog (" << entries << " entries selected): "
			<< (db_changed ? 1 : 0) << " changed" << Qt::endl;
		if (db_changed) {
			std::ostringstream xa, xb;
			lcf::LDB_Reader::SaveXml(xa, database);
			lcf::LDB_Reader::SaveXml(xb, dialog.editedData().database());
			out << "CHANGED database:\n" << firstDifferences(xa.str(), xb.str()) << Qt::endl;
			++dialogs_changed;
		}
	}

	for (const auto& note : notes) {
		out << "NOTE " << note << Qt::endl;
	}

	return changed == 0 && dialogs_changed == 0 ? 0 : 1;
}
