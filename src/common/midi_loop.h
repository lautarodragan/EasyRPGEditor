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

#include <cstddef>

/**
 * Finds the loop point of a Standard MIDI File, as RPG Maker 2000/2003 uses it:
 * a Control Change 111 on any channel, with any value. Like EasyRPG Player,
 * the last one in the song counts when there are several.
 *
 * @param data Contents of the file
 * @param size Size of the file in bytes
 * @return Tick of the loop point, or -1 when there is none or the file is invalid
 */
int midiLoopTick(const unsigned char* data, std::size_t size);
