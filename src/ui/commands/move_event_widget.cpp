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

#include "move_event_widget.h"
#include "ui_move_event_widget.h"
#include "ui/common/event_target_combobox.h"
#include "stringizer.h"
#include "common/dbstring.h"
#include "model/move_route_codec.h"
#include "model/project.h"
#include "model/project_data.h"
#include "ui/common/rpg_combobox.h"
#include "ui/picker/picker_audio_widget.h"
#include "ui/picker/picker_charset_widget.h"
#include "ui/picker/picker_dialog.h"
#include "defines.h"
#include <lcf/rpg/sound.h>
#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
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

MoveEventWidget::MoveEventWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::MoveEventWidget) {

	ui->setupUi(this);

	for (int i = 1; i <= 8; ++i) {
		auto* radio = findChild<QAbstractButton*>(QString("radioFrequency%1").arg(i));
		ui->groupFrequency_arg1->setId(radio, i);
	}

	m_encoding = m_project.project().encoding().toStdString();

	addCommandButtons();

	ui->listRoute->setToolTip(tr("Double-click a command with options to edit it"));
	connect(ui->listRoute, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
		editCommand(ui->listRoute->row(item));
	});
	connect(ui->pushDelete, &QPushButton::clicked, this, &MoveEventWidget::deleteCommand);
	connect(ui->pushDeleteAll, &QPushButton::clicked, this, &MoveEventWidget::deleteAllCommands);
}

MoveEventWidget::~MoveEventWidget() {
	delete ui;
}

void MoveEventWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Ensure frequency, repeat and skip exist before the base class binds them,
	// and that the frequency has a radio button (1 to 8)
	std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
	if (params.size() < MoveRouteCodec::first_command) {
		params.resize(MoveRouteCodec::first_command, 0);
	}
	params[1] = std::clamp(params[1], 1, 8);
	cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());

	EventCommandBaseWidget::setData(cmd);

	m_route_valid = MoveRouteCodec::Decode(cmd->parameters, m_route, m_encoding);
	if (!m_route_valid) {
		// Keep the data untouched instead of overwriting it with a partial route
		ui->groupCommands->setEnabled(false);
		ui->listRoute->setEnabled(false);
		ui->pushDelete->setEnabled(false);
		ui->pushDeleteAll->setEnabled(false);
	}

	populateTargets();
	refreshRoute(static_cast<int>(m_route.size()));
}

void MoveEventWidget::addCommandButtons() {
	for (size_t col = 0; col < button_columns.size(); ++col) {
		for (size_t row = 0; row < button_columns[col].size(); ++row) {
			Code code = button_columns[col][row];
			auto* button = new QPushButton(Stringizer::moveCommand(static_cast<int>(code)) + (needsDialog(code) ? "..." : ""), ui->groupCommands);
			connect(button, &QPushButton::clicked, this, [this, code] { insertCommand(static_cast<int>(code)); });
			ui->layoutCommands->addWidget(button, static_cast<int>(row), static_cast<int>(col));
		}
	}
}

void MoveEventWidget::populateTargets() {
	EventTargetComboBox::fill(ui->comboTarget, m_cmd->parameters[0], true);

	connect(ui->comboTarget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		m_cmd->parameters[0] = ui->comboTarget->itemData(i).toInt();
	});
}

void MoveEventWidget::refreshRoute(int select_row) {
	ui->listRoute->clear();
	for (const auto& cmd : m_route) {
		ui->listRoute->addItem(Stringizer::moveCommand(cmd));
	}
	// Marks the end of the route, commands are inserted before the selection
	ui->listRoute->addItem("");
	ui->listRoute->setCurrentRow(std::clamp(select_row, 0, static_cast<int>(m_route.size())));
}

void MoveEventWidget::insertCommand(int code) {
	if (!m_route_valid) {
		return;
	}
	int row = std::clamp(ui->listRoute->currentRow(), 0, static_cast<int>(m_route.size()));
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
	writeRoute();
	// Keep the selection on the same item, so the next command goes after this one
	refreshRoute(row + 1);
}

void MoveEventWidget::editCommand(int row) {
	if (!m_route_valid || row < 0 || row >= static_cast<int>(m_route.size())
			|| !needsDialog(static_cast<Code>(m_route[row].command_id))) {
		return;
	}
	lcf::rpg::MoveCommand cmd = m_route[row];
	if (!editOptions(cmd)) {
		return;
	}
	m_route[row] = cmd;
	writeRoute();
	refreshRoute(row);
}

bool MoveEventWidget::editOptions(lcf::rpg::MoveCommand& cmd) {
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

bool MoveEventWidget::editSwitch(lcf::rpg::MoveCommand& cmd) {
	QDialog dialog(this);
	dialog.setWindowTitle(Stringizer::moveCommand(cmd.command_id));

	auto* combo = new SwitchRpgComboBox(&dialog);
	combo->makeModel(m_project);
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

bool MoveEventWidget::editGraphic(lcf::rpg::MoveCommand& cmd) {
	auto* widget = new PickerCharsetWidget(std::clamp(cmd.parameter_a, 0, 7), this);
	PickerDialog dialog(m_project, FileFinder::FileType::Image, widget, this);
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

bool MoveEventWidget::editSound(lcf::rpg::MoveCommand& cmd) {
	lcf::rpg::Sound sound;
	sound.volume = cmd.parameter_a;
	sound.tempo = cmd.parameter_b;
	sound.balance = cmd.parameter_c;

	auto* widget = new PickerAudioWidget(sound, this);
	PickerDialog dialog(m_project, FileFinder::FileType::Sound, widget, this);
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

void MoveEventWidget::deleteCommand() {
	int row = ui->listRoute->currentRow();
	if (!m_route_valid || row < 0 || row >= static_cast<int>(m_route.size())) {
		return;
	}
	m_route.erase(m_route.begin() + row);
	writeRoute();
	refreshRoute(row);
}

void MoveEventWidget::deleteAllCommands() {
	if (!m_route_valid) {
		return;
	}
	m_route.clear();
	writeRoute();
	refreshRoute(0);
}

void MoveEventWidget::writeRoute() {
	MoveRouteCodec::Encode(m_cmd->parameters, m_route, m_encoding);
}
