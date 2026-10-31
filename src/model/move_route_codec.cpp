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

#include "move_route_codec.h"
#include <algorithm>

namespace {
using Code = lcf::rpg::MoveCommand::Code;

bool ReadInt(const lcf::DBArray<int32_t>& parameters, size_t& i, int32_t& out) {
	if (i >= parameters.size()) {
		return false;
	}
	out = parameters[i++];
	return true;
}

bool ReadString(const lcf::DBArray<int32_t>& parameters, size_t& i, lcf::DBString& out) {
	int32_t length;
	if (!ReadInt(parameters, i, length) || length < 0 || i + length > parameters.size()) {
		return false;
	}
	std::string str;
	for (int32_t j = 0; j < length; ++j) {
		str.push_back(static_cast<char>(parameters[i++]));
	}
	out = lcf::DBString(str);
	return true;
}

void WriteString(std::vector<int32_t>& out, const lcf::DBString& str) {
	out.push_back(static_cast<int32_t>(str.size()));
	for (char c : str) {
		out.push_back(static_cast<unsigned char>(c));
	}
}
}

bool MoveRouteCodec::Decode(const lcf::DBArray<int32_t>& parameters, std::vector<lcf::rpg::MoveCommand>& commands) {
	commands.clear();

	for (size_t i = first_command; i < parameters.size();) {
		lcf::rpg::MoveCommand cmd;
		cmd.command_id = parameters[i++];

		bool ok = true;
		switch (static_cast<Code>(cmd.command_id)) {
			case Code::switch_on:
			case Code::switch_off:
				ok = ReadInt(parameters, i, cmd.parameter_a);
				break;
			case Code::change_graphic:
				ok = ReadString(parameters, i, cmd.parameter_string)
					&& ReadInt(parameters, i, cmd.parameter_a);
				break;
			case Code::play_sound_effect:
				ok = ReadString(parameters, i, cmd.parameter_string)
					&& ReadInt(parameters, i, cmd.parameter_a)
					&& ReadInt(parameters, i, cmd.parameter_b)
					&& ReadInt(parameters, i, cmd.parameter_c);
				break;
			default:
				break;
		}

		if (!ok) {
			commands.clear();
			return false;
		}
		commands.push_back(cmd);
	}

	return true;
}

void MoveRouteCodec::Encode(lcf::DBArray<int32_t>& parameters, const std::vector<lcf::rpg::MoveCommand>& commands) {
	std::vector<int32_t> out(parameters.begin(), parameters.begin() + std::min<size_t>(parameters.size(), first_command));
	out.resize(first_command, 0);

	for (const auto& cmd : commands) {
		out.push_back(cmd.command_id);

		switch (static_cast<Code>(cmd.command_id)) {
			case Code::switch_on:
			case Code::switch_off:
				out.push_back(cmd.parameter_a);
				break;
			case Code::change_graphic:
				WriteString(out, cmd.parameter_string);
				out.push_back(cmd.parameter_a);
				break;
			case Code::play_sound_effect:
				WriteString(out, cmd.parameter_string);
				out.push_back(cmd.parameter_a);
				out.push_back(cmd.parameter_b);
				out.push_back(cmd.parameter_c);
				break;
			default:
				break;
		}
	}

	parameters = lcf::DBArray<int32_t>(out.begin(), out.end());
}
