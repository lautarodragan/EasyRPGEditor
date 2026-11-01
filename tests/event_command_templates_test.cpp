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

#include "model/event_command_templates.h"
#include <iostream>
#include <set>
#include <string>
#include <utility>

namespace {
int failures = 0;

void check(bool cond, const std::string& what) {
	if (!cond) {
		std::cerr << "FAIL: " << what << "\n";
		++failures;
	}
}

using Cmd = lcf::rpg::EventCommand::Code;
using Lines = std::vector<lcf::rpg::EventCommand>;
using Shape = std::vector<std::pair<Cmd, int>>;

namespace T = EventCommandTemplates;

const T::Template& find(Cmd code) {
	for (const auto& tmpl : T::all()) {
		if (tmpl.code == code) {
			return tmpl;
		}
	}
	std::cerr << "no template for " << code << "\n";
	std::abort();
}

// The head and its companions
Lines block(lcf::rpg::EventCommand head) {
	Lines lines = {head};
	auto companions = T::makeCompanions(head);
	lines.insert(lines.end(), companions.begin(), companions.end());
	return lines;
}

bool hasShape(const Lines& lines, const Shape& shape) {
	if (lines.size() != shape.size()) {
		return false;
	}
	for (size_t i = 0; i < lines.size(); ++i) {
		if (lines[i].code != static_cast<int32_t>(shape[i].first) || lines[i].indent != shape[i].second) {
			return false;
		}
	}
	return true;
}
}

int main() {
	// Loop at indent 1
	auto loop = block(T::makeCommand(find(Cmd::Loop), true, 1));
	check(hasShape(loop, {{Cmd::Loop, 1}, {Cmd::END, 2}, {Cmd::EndLoop, 1}}), "loop block");

	// Conditional Branch, the default has an else branch
	auto branch_head = T::makeCommand(find(Cmd::ConditionalBranch), true, 0);
	check(hasShape(block(branch_head), {{Cmd::ConditionalBranch, 0}, {Cmd::END, 1},
			{Cmd::ElseBranch, 0}, {Cmd::END, 1}, {Cmd::EndBranch, 0}}), "branch with else");
	branch_head.parameters[5] = 0;
	check(hasShape(block(branch_head), {{Cmd::ConditionalBranch, 0}, {Cmd::END, 1}, {Cmd::EndBranch, 0}}), "branch without else");

	auto battle_branch = block(T::makeCommand(find(Cmd::ConditionalBranch_B), true, 0));
	check(hasShape(battle_branch, {{Cmd::ConditionalBranch_B, 0}, {Cmd::END, 1},
			{Cmd::ElseBranch_B, 0}, {Cmd::END, 1}, {Cmd::EndBranch_B, 0}}), "battle branch");

	// Show Choice: one option per choice, cancel case 5 has its own branch
	auto choice_head = T::makeCommand(find(Cmd::ShowChoice), true, 0);
	auto choice = block(choice_head);
	check(hasShape(choice, {{Cmd::ShowChoice, 0}, {Cmd::ShowChoiceOption, 0}, {Cmd::END, 1},
			{Cmd::ShowChoiceOption, 0}, {Cmd::END, 1}, {Cmd::ShowChoiceEnd, 0}}), "choice block");
	check(choice[1].string == "Yes" && choice[1].parameters.size() == 1 && choice[1].parameters[0] == 0, "first option");
	check(choice[3].string == "No" && choice[3].parameters[0] == 1, "second option");

	choice_head.string = lcf::DBString("A/B/C");
	choice_head.parameters[0] = 5;
	choice = block(choice_head);
	check(choice.size() == 10 && choice[7].code == static_cast<int32_t>(Cmd::ShowChoiceOption)
		&& choice[7].string.empty() && choice[7].parameters[0] == 4, "cancel option");

	// Encounter: no handlers by default, handlers for custom escape and defeat
	auto encounter_head = T::makeCommand(find(Cmd::EnemyEncounter), true, 0);
	check(T::makeCompanions(encounter_head).empty(), "encounter without handlers");
	check(T::makeCommand(find(Cmd::EnemyEncounter), false, 0).parameters.size() == 6, "encounter parameters 2k");
	check(encounter_head.parameters.size() == 9, "encounter parameters 2k3");
	encounter_head.parameters[3] = 2;
	encounter_head.parameters[4] = 1;
	check(hasShape(block(encounter_head), {{Cmd::EnemyEncounter, 0}, {Cmd::VictoryHandler, 0}, {Cmd::END, 1},
			{Cmd::EscapeHandler, 0}, {Cmd::END, 1}, {Cmd::DefeatHandler, 0}, {Cmd::END, 1}, {Cmd::EndBattle, 0}}), "encounter handlers");

	// Shop and inn
	auto shop_head = T::makeCommand(find(Cmd::OpenShop), true, 0);
	check(T::makeCompanions(shop_head).empty(), "shop without handlers");
	shop_head.parameters[2] = 1;
	check(hasShape(block(shop_head), {{Cmd::OpenShop, 0}, {Cmd::Transaction, 0}, {Cmd::END, 1},
			{Cmd::NoTransaction, 0}, {Cmd::END, 1}, {Cmd::EndShop, 0}}), "shop handlers");

	// Block ends: a loop inside a branch, followed by an encounter without handlers
	Lines list = block(T::makeCommand(find(Cmd::ConditionalBranch), true, 0));
	auto inner = block(T::makeCommand(find(Cmd::Loop), true, 1));
	list.insert(list.begin() + 1, inner.begin(), inner.end());
	list.push_back(T::makeCommand(find(Cmd::EnemyEncounter), true, 0));
	list.push_back(T::makeCommand(find(Cmd::Wait), true, 0));
	// 0 branch, 1 loop, 2 empty, 3 end loop, 4 empty, 5 else, 6 empty, 7 end, 8 encounter, 9 wait
	check(T::blockEnd(list, 0) == 8, "branch end");
	check(T::blockEnd(list, 1) == 4, "loop end");
	check(T::blockEnd(list, 8) == 9, "encounter end");
	check(T::blockEnd(list, 9) == 10, "wait end");

	// Message lines
	Lines message = {T::makeCommand(find(Cmd::ShowMessage), true, 0)};
	lcf::rpg::EventCommand message_line;
	message_line.code = static_cast<int32_t>(Cmd::ShowMessage_2);
	message.push_back(message_line);
	message.push_back(message_line);
	message.push_back(T::makeCommand(find(Cmd::Wait), true, 0));
	check(T::blockEnd(message, 0) == 3, "message end");

	// Parameter counts for RPG Maker 2000
	check(T::makeCommand(find(Cmd::Teleport), false, 0).parameters.size() == 3, "teleport 2k");
	check(T::makeCommand(find(Cmd::Teleport), true, 0).parameters.size() == 4, "teleport 2k3");

	// Each command is listed once per context
	for (bool rpg2k3 : {false, true}) {
		for (bool battle : {false, true}) {
			std::set<Cmd> codes;
			for (const auto& tmpl : T::all()) {
				if (T::isAvailable(tmpl, rpg2k3, battle)) {
					check(codes.insert(tmpl.code).second, "duplicate template " + std::to_string(static_cast<int>(tmpl.code)));
				}
			}
		}
	}
	check(!T::isAvailable(find(Cmd::ChangeClass), false, false), "Change Class is 2k3 only");
	check(!T::isAvailable(find(Cmd::Teleport), true, true), "no Teleport in battle");
	check(T::isAvailable(find(Cmd::ConditionalBranch_B), false, true), "battle branch in 2k battles");

	if (failures) {
		std::cerr << failures << " failures\n";
		return 1;
	}
	std::cout << "OK\n";
	return 0;
}
