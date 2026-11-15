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

#include "move_route_editor.h"
#include "stringizer.h"
#include "common/dbstring.h"
#include "model/project.h"
#include "model/project_data.h"
#include "ui/common/rpg_combobox.h"
#include "ui/picker/picker_audio_widget.h"
#include "ui/picker/picker_charset_widget.h"
#include "ui/picker/picker_dialog.h"
#include "defines.h"
#include <lcf/rpg/sound.h>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>

namespace {
using Code = lcf::rpg::MoveCommand::Code;

// Layout of the command buttons, one list per column, as in RPG Maker
const std::vector<std::vector<Code>> button_columns = {
	{
		Code::move_up, Code::move_right, Code::move_down, Code::move_left,
		Code::move_upright, Code::move_downright, Code::move_downleft, Code::move_upleft,
		Code::move_random, Code::move_towards_hero, Code::move_away_from_hero, Code::move_forward,
		Code::increase_movement_speed, Code::decrease_movement_speed
	},
	{
		Code::face_up, Code::face_right, Code::face_down, Code::face_left,
		Code::turn_90_degree_right, Code::turn_90_degree_left, Code::turn_180_degree, Code::turn_90_degree_random,
		Code::face_random_direction, Code::face_hero, Code::face_away_from_hero, Code::wait,
		Code::increase_movement_frequence, Code::decrease_movement_frequence
	},
	{
		Code::begin_jump, Code::end_jump, Code::lock_facing, Code::unlock_facing,
		Code::walk_everywhere_on, Code::walk_everywhere_off, Code::stop_animation, Code::start_animation,
		Code::increase_transp, Code::decrease_transp,
		Code::switch_on, Code::switch_off, Code::change_graphic, Code::play_sound_effect
	}
};

// Commands with options, edited in a dialog
bool needsDialog(Code code) {
	return code == Code::switch_on || code == Code::switch_off
		|| code == Code::change_graphic || code == Code::play_sound_effect;
}
}

MoveRouteEditor::MoveRouteEditor(QWidget* parent) : QWidget(parent) {
	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_left = new QVBoxLayout();
	m_left->addWidget(new QLabel(tr("Movement Commands"), this));
	m_list = new QListWidget(this);
	m_left->addWidget(m_list);
	auto* buttons = new QHBoxLayout();
	m_delete = new QPushButton(tr("Delete"), this);
	m_deleteAll = new QPushButton(tr("Delete All"), this);
	buttons->addWidget(m_delete);
	buttons->addWidget(m_deleteAll);
	m_left->addLayout(buttons);
	layout->addLayout(m_left);

	m_commands = new QGroupBox(tr("Add Command"), this);
	m_commandsLayout = new QGridLayout(m_commands);
	layout->addWidget(m_commands);

	addCommandButtons();

	m_list->setToolTip(tr("Double-click a command with options to edit it"));
	connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
		editCommand(m_list->row(item));
	});
	connect(m_delete, &QPushButton::clicked, this, &MoveRouteEditor::deleteCommand);
	connect(m_deleteAll, &QPushButton::clicked, this, &MoveRouteEditor::deleteAllCommands);
}

void MoveRouteEditor::setProject(ProjectData& project) {
	m_project = &project;
}

void MoveRouteEditor::setRoute(const std::vector<lcf::rpg::MoveCommand>& route, bool editable) {
	m_route = route;
	m_editable = editable;
	// Without editing, the data stays untouched instead of being overwritten with a partial route
	m_commands->setEnabled(editable);
	m_list->setEnabled(editable);
	m_delete->setEnabled(editable);
	m_deleteAll->setEnabled(editable);
	refreshRoute(static_cast<int>(m_route.size()));
}

const std::vector<lcf::rpg::MoveCommand>& MoveRouteEditor::route() const {
	return m_route;
}

void MoveRouteEditor::addBelowList(QWidget* widget) {
	m_left->addWidget(widget);
}

void MoveRouteEditor::addCommandButtons() {
	for (size_t col = 0; col < button_columns.size(); ++col) {
		for (size_t row = 0; row < button_columns[col].size(); ++row) {
			Code code = button_columns[col][row];
			auto* button = new QPushButton(Stringizer::moveCommand(static_cast<int>(code)) + (needsDialog(code) ? "..." : ""), m_commands);
			connect(button, &QPushButton::clicked, this, [this, code] { insertCommand(static_cast<int>(code)); });
			m_commandsLayout->addWidget(button, static_cast<int>(row), static_cast<int>(col));
		}
	}
}

void MoveRouteEditor::refreshRoute(int select_row) {
	m_list->clear();
	for (const auto& cmd : m_route) {
		m_list->addItem(Stringizer::moveCommand(cmd));
	}
	// Marks the end of the route, commands are inserted before the selection
	m_list->addItem("");
	m_list->setCurrentRow(std::clamp(select_row, 0, static_cast<int>(m_route.size())));
}

void MoveRouteEditor::insertCommand(int code) {
	if (!m_editable) {
		return;
	}
	int row = std::clamp(m_list->currentRow(), 0, static_cast<int>(m_route.size()));
	lcf::rpg::MoveCommand cmd;
	cmd.command_id = code;
	if (static_cast<Code>(code) == Code::play_sound_effect) {
		lcf::rpg::Sound sound;
		cmd.parameter_a = sound.volume;
		cmd.parameter_b = sound.tempo;
		cmd.parameter_c = sound.balance;
	}
	if (!editOptions(cmd)) {
		return;
	}
	m_route.insert(m_route.begin() + row, cmd);
	emit routeChanged();
	// Keep the selection on the same item, so the next command goes after this one
	refreshRoute(row + 1);
}

void MoveRouteEditor::editCommand(int row) {
	if (!m_editable || row < 0 || row >= static_cast<int>(m_route.size())
			|| !needsDialog(static_cast<Code>(m_route[row].command_id))) {
		return;
	}
	lcf::rpg::MoveCommand cmd = m_route[row];
	if (!editOptions(cmd)) {
		return;
	}
	m_route[row] = cmd;
	emit routeChanged();
	refreshRoute(row);
}

bool MoveRouteEditor::editOptions(lcf::rpg::MoveCommand& cmd) {
	switch (static_cast<Code>(cmd.command_id)) {
		case Code::switch_on:
		case Code::switch_off:
			return editSwitch(cmd);
		case Code::change_graphic:
			return editGraphic(cmd);
		case Code::play_sound_effect:
			return editSound(cmd);
		default:
			return true;
	}
}

bool MoveRouteEditor::editSwitch(lcf::rpg::MoveCommand& cmd) {
	QDialog dialog(this);
	dialog.setWindowTitle(Stringizer::moveCommand(cmd.command_id));

	auto* combo = new SwitchRpgComboBox(&dialog);
	combo->makeModel(*m_project);
	combo->setCurrentIndex(std::max(combo->comboBox()->findData(cmd.parameter_a), 0));

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	auto* layout = new QVBoxLayout(&dialog);
	layout->addWidget(combo);
	layout->addWidget(buttons);
	dialog.resize(360, dialog.sizeHint().height());

	int id = 0;
	if (dialog.exec() != QDialog::Accepted || (id = combo->comboBox()->currentData().toInt()) <= 0) {
		return false;
	}
	cmd.parameter_a = id;
	return true;
}

bool MoveRouteEditor::editGraphic(lcf::rpg::MoveCommand& cmd) {
	auto* widget = new PickerCharsetWidget(std::clamp(cmd.parameter_a, 0, 7), this);
	PickerDialog dialog(*m_project, FileFinder::FileType::Image, widget, this);
	QString name;
	connect(&dialog, &PickerDialog::fileSelected, this, [&](const QString& baseName) {
		name = baseName;
	});
	dialog.setDirectoryAndFile(CHARSET, ToQString(cmd.parameter_string));
	if (dialog.exec() != QDialog::Accepted || name.isEmpty()) {
		return false;
	}
	cmd.parameter_string = ToDBString(name);
	cmd.parameter_a = widget->index();
	return true;
}

bool MoveRouteEditor::editSound(lcf::rpg::MoveCommand& cmd) {
	lcf::rpg::Sound sound;
	sound.volume = cmd.parameter_a;
	sound.tempo = cmd.parameter_b;
	sound.balance = cmd.parameter_c;

	auto* widget = new PickerAudioWidget(sound, this);
	PickerDialog dialog(*m_project, FileFinder::FileType::Sound, widget, this);
	QString name;
	connect(&dialog, &PickerDialog::fileSelected, this, [&](const QString& baseName) {
		name = baseName;
	});
	dialog.setDirectoryAndFile(SOUND, ToQString(cmd.parameter_string));
	if (dialog.exec() != QDialog::Accepted || name.isEmpty()) {
		return false;
	}
	cmd.parameter_string = ToDBString(name);
	cmd.parameter_a = widget->volume();
	cmd.parameter_b = widget->tempo();
	cmd.parameter_c = widget->balance();
	return true;
}

void MoveRouteEditor::deleteCommand() {
	int row = m_list->currentRow();
	if (!m_editable || row < 0 || row >= static_cast<int>(m_route.size())) {
		return;
	}
	m_route.erase(m_route.begin() + row);
	emit routeChanged();
	refreshRoute(row);
}

void MoveRouteEditor::deleteAllCommands() {
	if (!m_editable) {
		return;
	}
	m_route.clear();
	emit routeChanged();
	refreshRoute(0);
}
