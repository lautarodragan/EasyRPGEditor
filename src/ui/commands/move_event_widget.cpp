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
#include "core.h"
#include "stringizer.h"
#include "common/dbstring.h"
#include "model/move_route_codec.h"
#include <QAbstractButton>
#include <QPushButton>
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

// Commands with options that need their own dialog
bool needsDialog(Code code) {
	return code == Code::switch_on || code == Code::switch_off
		|| code == Code::change_graphic || code == Code::play_sound_effect;
}

// Vehicles and the event itself use reserved IDs
constexpr int target_hero = 10001;
constexpr int target_boat = 10002;
constexpr int target_ship = 10003;
constexpr int target_airship = 10004;
constexpr int target_this_event = 10005;
}

MoveEventWidget::MoveEventWidget(ProjectData& project, QWidget *parent) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::MoveEventWidget) {

	ui->setupUi(this);

	for (int i = 1; i <= 8; ++i) {
		auto* radio = findChild<QAbstractButton*>(QString("radioFrequency%1").arg(i));
		ui->groupFrequency_arg1->setId(radio, i);
	}

	addCommandButtons();

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

	m_route_valid = MoveRouteCodec::Decode(cmd->parameters, m_route);
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
			bool dialog = needsDialog(code);
			auto* button = new QPushButton(Stringizer::moveCommand(static_cast<int>(code)) + (dialog ? "..." : ""), ui->groupCommands);
			if (dialog) {
				button->setEnabled(false);
				button->setToolTip(tr("Not supported yet"));
			}
			connect(button, &QPushButton::clicked, this, [this, code] { insertCommand(static_cast<int>(code)); });
			ui->layoutCommands->addWidget(button, static_cast<int>(row), static_cast<int>(col));
		}
	}
}

void MoveEventWidget::populateTargets() {
	auto* combo = ui->comboTarget;
	combo->clear();
	combo->addItem(Stringizer::characterName(target_hero), target_hero);
	combo->addItem(Stringizer::characterName(target_boat), target_boat);
	combo->addItem(Stringizer::characterName(target_ship), target_ship);
	combo->addItem(Stringizer::characterName(target_airship), target_airship);
	combo->addItem(Stringizer::characterName(target_this_event), target_this_event);

	if (const auto* events = core().currentMapEvents()) {
		for (auto it = events->begin(); it != events->end(); ++it) {
			combo->addItem(QString("%1: %2").arg(it.key(), 4, 10, QChar('0')).arg(ToQString(it.value()->name)), it.key());
		}
	}

	int target = m_cmd->parameters[0];
	int index = combo->findData(target);
	if (index == -1) {
		// Event not on the current map, keep it selectable
		combo->addItem(Stringizer::characterName(target), target);
		index = combo->count() - 1;
	}
	combo->setCurrentIndex(index);

	connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
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
	m_route.insert(m_route.begin() + row, cmd);
	writeRoute();
	// Keep the selection on the same item, so the next command goes after this one
	refreshRoute(row + 1);
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
	MoveRouteCodec::Encode(m_cmd->parameters, m_route);
}
