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

#include "midi_loop.h"
#include <algorithm>
#include <cstdint>

namespace {
uint32_t readBig(const unsigned char* p, int bytes) {
	uint32_t value = 0;
	for (int i = 0; i < bytes; ++i) {
		value = (value << 8) | p[i];
	}
	return value;
}

// Reads a variable length quantity, false when it runs past the end
bool readVarLen(const unsigned char*& p, const unsigned char* end, uint32_t& value) {
	value = 0;
	for (int i = 0; i < 4; ++i) {
		if (p >= end) {
			return false;
		}
		unsigned char byte = *p++;
		value = (value << 7) | (byte & 0x7F);
		if (!(byte & 0x80)) {
			return true;
		}
	}
	return false;
}
}

int midiLoopTick(const unsigned char* data, std::size_t size) {
	if (size < 14 || readBig(data, 4) != 0x4D546864) { // "MThd"
		return -1;
	}

	int loop_tick = -1;
	std::size_t chunk = 8 + std::size_t(readBig(data + 4, 4));
	while (chunk < size && size - chunk >= 8) {
		bool is_track = readBig(data + chunk, 4) == 0x4D54726B; // "MTrk"
		const unsigned char* p = data + chunk + 8;
		std::size_t length = std::min<std::size_t>(readBig(data + chunk + 4, 4), size - chunk - 8);
		const unsigned char* track_end = p + length;
		chunk += 8 + length;
		if (!is_track) {
			continue;
		}

		uint32_t tick = 0;
		unsigned char status = 0;
		while (p < track_end) {
			uint32_t delta;
			if (!readVarLen(p, track_end, delta) || p >= track_end) {
				break;
			}
			tick += delta;

			unsigned char byte = *p;
			if (byte == 0xFF || byte == 0xF0 || byte == 0xF7) {
				// Meta event or SysEx: skip its data
				int meta_type = -1;
				if (byte == 0xFF) {
					if (track_end - p < 2) {
						break;
					}
					meta_type = p[1];
				}
				p += byte == 0xFF ? 2 : 1;
				uint32_t skip;
				if (!readVarLen(p, track_end, skip) || skip > std::size_t(track_end - p) || meta_type == 0x2F) {
					break;
				}
				// Running status stays, as FluidSynth keeps it
				p += skip;
				continue;
			}
			if (byte & 0x80) {
				status = byte;
				++p;
			}
			if (status < 0x80 || status >= 0xF0) {
				break; // Data without a status, or not a channel message
			}

			int data_bytes = (status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0 ? 1 : 2;
			if (track_end - p < data_bytes) {
				break;
			}
			if ((status & 0xF0) == 0xB0 && p[0] == 111) {
				loop_tick = std::max(loop_tick, static_cast<int>(std::min<uint32_t>(tick, INT32_MAX)));
			}
			p += data_bytes;
		}
	}
	return loop_tick;
}
