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

class QComboBox;

namespace EventTargetComboBox {
	/** IDs that RPG Maker reserves for the hero, vehicles and the calling event */
	constexpr int hero = 10001;
	constexpr int boat = 10002;
	constexpr int ship = 10003;
	constexpr int airship = 10004;
	constexpr int this_event = 10005;

	/**
	 * Fills a combo box with the characters an event command can target.
	 * The item data is the character ID.
	 *
	 * @param combo Combo box to fill
	 * @param current Character to select. Added when it is not on the current map.
	 * @param with_hero_and_vehicles Whether to list the hero and the vehicles
	 */
	void fill(QComboBox* combo, int current, bool with_hero_and_vehicles);
}
