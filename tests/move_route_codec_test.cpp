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

#include "model/move_route_codec.h"
#include <iostream>

namespace {
int failures = 0;

void check(bool cond, const char* what) {
	if (!cond) {
		std::cerr << "FAIL: " << what << "\n";
		++failures;
	}
}

using Code = lcf::rpg::MoveCommand::Code;
}

int main() {
	// Target 10001, frequency 3, repeat, skip, then: Move Up, Switch ON 5,
	// Change Graphic "ab" index 2, Play SE "c" 80 100 50, Turn Left
	lcf::DBArray<int32_t> params = {
		10001, 3, 1, 1,
		0,
		32, 5,
		34, 2, 'a', 'b', 2,
		35, 1, 'c', 80, 100, 50,
		15
	};

	std::vector<lcf::rpg::MoveCommand> route;
	check(MoveRouteCodec::Decode(params, route), "decodes a valid route");
	check(route.size() == 5, "decodes five commands");
	if (route.size() == 5) {
		check(route[0].command_id == static_cast<int>(Code::move_up), "first is Move Up");
		check(route[1].command_id == static_cast<int>(Code::switch_on) && route[1].parameter_a == 5, "switch ID is read");
		check(route[2].parameter_string == "ab" && route[2].parameter_a == 2, "graphic name and index are read");
		check(route[3].parameter_string == "c" && route[3].parameter_a == 80
			&& route[3].parameter_b == 100 && route[3].parameter_c == 50, "sound name, volume, tempo and balance are read");
		check(route[4].command_id == static_cast<int>(Code::face_left), "last is Turn Left");
	}

	lcf::DBArray<int32_t> encoded = params;
	MoveRouteCodec::Encode(encoded, route);
	check(encoded == params, "encoding the decoded route gives the original parameters");

	route.erase(route.begin() + 1, route.end());
	MoveRouteCodec::Encode(encoded, route);
	check(encoded == lcf::DBArray<int32_t>({10001, 3, 1, 1, 0}), "encoding keeps target, frequency, repeat and skip");

	// Commands built from scratch, as the editor does when adding them
	std::vector<lcf::rpg::MoveCommand> built(4);
	built[0].command_id = static_cast<int>(Code::switch_off);
	built[0].parameter_a = 12;
	built[1].command_id = static_cast<int>(Code::change_graphic);
	built[1].parameter_string = lcf::DBString("Hero");
	built[1].parameter_a = 7;
	built[2].command_id = static_cast<int>(Code::play_sound_effect);
	built[2].parameter_string = lcf::DBString("Door");
	built[2].parameter_a = 90;
	built[2].parameter_b = 150;
	built[2].parameter_c = 0;
	built[3].command_id = static_cast<int>(Code::wait);
	MoveRouteCodec::Encode(encoded, built);
	check(encoded == lcf::DBArray<int32_t>({10001, 3, 1, 1,
		33, 12,
		34, 4, 'H', 'e', 'r', 'o', 7,
		35, 4, 'D', 'o', 'o', 'r', 90, 150, 0,
		23}), "switch, graphic and sound are encoded");
	std::vector<lcf::rpg::MoveCommand> decoded;
	check(MoveRouteCodec::Decode(encoded, decoded) && decoded.size() == 4
		&& decoded[0].command_id == built[0].command_id && decoded[0].parameter_a == 12
		&& decoded[1].parameter_string == "Hero" && decoded[1].parameter_a == 7
		&& decoded[2].parameter_string == "Door" && decoded[2].parameter_a == 90
		&& decoded[2].parameter_b == 150 && decoded[2].parameter_c == 0
		&& decoded[3].command_id == built[3].command_id, "built commands round-trip");

	// Names are stored in the game encoding, the route holds UTF-8
	built.resize(1);
	built[0].command_id = static_cast<int>(Code::change_graphic);
	built[0].parameter_string = lcf::DBString("Caf\xc3\xa9");
	built[0].parameter_a = 1;
	MoveRouteCodec::Encode(encoded, built, "1252");
	check(encoded == lcf::DBArray<int32_t>({10001, 3, 1, 1, 34, 4, 'C', 'a', 'f', 0xe9, 1}), "names are converted to the game encoding");
	check(MoveRouteCodec::Decode(encoded, decoded, "1252") && decoded.size() == 1
		&& decoded[0].parameter_string == "Caf\xc3\xa9", "names are converted to UTF-8");

	lcf::DBArray<int32_t> truncated = {10001, 3, 0, 0, 34, 5, 'a'};
	check(!MoveRouteCodec::Decode(truncated, route) && route.empty(), "truncated data is rejected");

	if (failures == 0) {
		std::cout << "All tests passed\n";
	}
	return failures == 0 ? 0 : 1;
}
