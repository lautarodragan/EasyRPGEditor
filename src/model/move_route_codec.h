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
#include <lcf/rpg/movecommand.h>
#include <string>
#include <vector>

/**
 * Converts between the parameters of a Move Event command and a list of
 * move commands.
 *
 * The parameters are: target, frequency, repeat, skip if blocked, followed by
 * the encoded route. Each move command is its code, followed by extra data for
 * switches (switch ID), graphics (name, index) and sounds (name, volume, tempo,
 * balance). Names are encoded as their length followed by one int per byte,
 * in the encoding of the game (liblcf does not convert them).
 */
namespace MoveRouteCodec {
	/** Index of the first move command in the parameters */
	constexpr size_t first_command = 4;

	/**
	 * Decodes the move commands of a Move Event command.
	 *
	 * @param parameters Parameters of the Move Event command
	 * @param commands Receives the decoded commands
	 * @param encoding Encoding of the names, they are converted to UTF-8. Empty to keep them as they are
	 * @return false when the data is truncated, commands is left empty then
	 */
	bool Decode(const lcf::DBArray<int32_t>& parameters, std::vector<lcf::rpg::MoveCommand>& commands, const std::string& encoding = {});

	/**
	 * Replaces the move commands of a Move Event command.
	 *
	 * @param parameters Parameters of the Move Event command, the first four are kept
	 * @param commands Commands to encode
	 * @param encoding Encoding of the names, they are converted from UTF-8. Empty to keep them as they are
	 */
	void Encode(lcf::DBArray<int32_t>& parameters, const std::vector<lcf::rpg::MoveCommand>& commands, const std::string& encoding = {});
}
