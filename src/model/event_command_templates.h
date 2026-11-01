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
#include <cstddef>
#include <vector>

/**
 * The commands that can be inserted into an event command list, with their
 * default parameters, and the structure of the command blocks
 * (Conditional Branch ... End, Loop ... End Loop, Show Choice ... End, ...).
 */
namespace EventCommandTemplates {
	enum class Category {
		Messages,
		GameProgression,
		FlowControl,
		Party,
		Actors,
		System,
		MapMovement,
		Screen,
		Pictures,
		Audio,
		Battle,
		Advanced
	};

	/** Where a command can be used: map events and common events, or troop pages */
	enum Context {
		Map2k = 1,
		Map2k3 = 2,
		Battle2k = 4,
		Battle2k3 = 8,
		Map = Map2k | Map2k3,
		Battle = Battle2k | Battle2k3,
		Rpg2k3 = Map2k3 | Battle2k3,
		Everywhere = Map | Battle
	};

	struct Template {
		lcf::rpg::EventCommand::Code code;
		Category category;
		int contexts;
		/** Untranslated name, translated in the "Stringizer" context */
		const char* name;
		/** Default parameters, as written by RPG Maker 2003 */
		std::vector<int32_t> parameters;
		const char* string = "";
		/** Number of parameters written by RPG Maker 2000, -1 when it's the same */
		int rpg2k_parameters = -1;
	};

	/** All insertable commands, ordered by category */
	const std::vector<Template>& all();

	/** Whether the template can be inserted into an event of this kind */
	bool isAvailable(const Template& tmpl, bool rpg2k3, bool battle);

	/** The command of the template with its default parameters for the engine */
	lcf::rpg::EventCommand makeCommand(const Template& tmpl, bool rpg2k3, int indent);

	/**
	 * The lines that must follow a new head command to form a valid block
	 * (branches with their empty line and the closing line), at the indent of
	 * the head. Empty for a command that isn't a block, or one that has no
	 * branches due to its parameters (an encounter without handlers, ...).
	 */
	std::vector<lcf::rpg::EventCommand> makeCompanions(const lcf::rpg::EventCommand& head);

	/**
	 * Index past the last line of the command at index: a message or comment
	 * with its additional lines, or a block up to its closing line.
	 * Requires correct indents.
	 */
	size_t blockEnd(const std::vector<lcf::rpg::EventCommand>& commands, size_t index);

	/**
	 * Whether the command is part of the block of another command
	 * (Else, End, a choice option, a battle handler, ...) and can't be
	 * deleted or have commands inserted before it on its own.
	 */
	bool isCompanion(int code);
}
