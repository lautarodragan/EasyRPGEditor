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

#include "common/midi_loop.h"
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <vector>

namespace {
int failures = 0;

void check(int got, int expected, const char* what) {
	if (got != expected) {
		std::cerr << "FAIL: " << what << ": got " << got << ", expected " << expected << "\n";
		++failures;
	}
}

using Bytes = std::vector<unsigned char>;

void putBig(Bytes& out, uint32_t value, int bytes) {
	for (int i = bytes - 1; i >= 0; --i) {
		out.push_back(static_cast<unsigned char>(value >> (i * 8)));
	}
}

Bytes varLen(uint32_t value) {
	Bytes out = { static_cast<unsigned char>(value & 0x7F) };
	while (value >>= 7) {
		out.insert(out.begin(), static_cast<unsigned char>(0x80 | (value & 0x7F)));
	}
	return out;
}

// A track event: delta time and the raw bytes after it
struct Event {
	uint32_t delta;
	Bytes bytes;
};

Bytes smf(std::initializer_list<std::vector<Event>> tracks) {
	Bytes out = { 'M', 'T', 'h', 'd' };
	putBig(out, 6, 4);
	putBig(out, 1, 2);
	putBig(out, static_cast<uint32_t>(tracks.size()), 2);
	putBig(out, 96, 2);
	for (const auto& track : tracks) {
		Bytes data;
		for (const auto& event : track) {
			Bytes delta = varLen(event.delta);
			data.insert(data.end(), delta.begin(), delta.end());
			data.insert(data.end(), event.bytes.begin(), event.bytes.end());
		}
		out.insert(out.end(), { 'M', 'T', 'r', 'k' });
		putBig(out, static_cast<uint32_t>(data.size()), 4);
		out.insert(out.end(), data.begin(), data.end());
	}
	return out;
}

int loopTick(const Bytes& data) {
	return midiLoopTick(data.data(), data.size());
}

const Bytes end_of_track = { 0xFF, 0x2F, 0x00 };
const Bytes tempo = { 0xFF, 0x51, 0x03, 0x07, 0xA1, 0x20 };
const Bytes sysex = { 0xF0, 0x05, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
}

int main() {
	// Notes, program change and other controllers, but no loop point
	Bytes plain = smf({ {
		{ 0, tempo },
		{ 0, { 0xC0, 0x05 } },
		{ 0, { 0xB0, 0x07, 0x64 } },
		{ 0, { 0x90, 0x3C, 0x64 } },
		{ 96, { 0x80, 0x3C, 0x00 } },
		{ 0, end_of_track },
	} });
	check(loopTick(plain), -1, "no CC111");

	// CC111 on channel 4 with value 0 in the second track, after a SysEx,
	// a long delta time and running status
	Bytes second_track = smf({
		{ { 0, tempo }, { 2000, end_of_track } },
		{
			{ 0, sysex },
			{ 0, { 0x93, 0x40, 0x64 } },
			{ 200, { 0x40, 0x00 } },
			{ 0, { 0xB3, 0x0A, 0x40 } },
			{ 300, { 0x6F, 0x00 } },
			{ 0, { 0x90, 0x3C, 0x64 } },
			{ 100, end_of_track },
		},
	});
	check(loopTick(second_track), 500, "CC111 with running status in the second track");

	// Running status kept after a meta event, like FluidSynth does
	Bytes after_meta = smf({ {
		{ 0, { 0xB0, 0x07, 0x64 } },
		{ 10, { 0xFF, 0x01, 0x01, 'x' } },
		{ 20, { 0x6F, 0x7F } },
		{ 0, end_of_track },
	} });
	check(loopTick(after_meta), 30, "CC111 with running status after a meta event");

	// Several: the last one counts, as in EasyRPG Player
	Bytes several = smf({
		{ { 300, { 0xB0, 0x6F, 0x00 } }, { 0, end_of_track } },
		{ { 100, { 0xB1, 0x6F, 0x00 } }, { 400, { 0xB1, 0x6F, 0x00 } }, { 0, end_of_track } },
	});
	check(loopTick(several), 500, "several CC111");

	// Value 111 of another controller, and CC111 after the end of the track
	Bytes not_loop = smf({ {
		{ 50, { 0xB0, 0x07, 0x6F } },
		{ 10, end_of_track },
		{ 10, { 0xB0, 0x6F, 0x00 } },
	} });
	check(loopTick(not_loop), -1, "no CC111 before the end of the track");

	// Not a MIDI file, or a broken one
	check(loopTick({ 'R', 'I', 'F', 'F', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }), -1, "not a MIDI file");
	check(midiLoopTick(nullptr, 0), -1, "empty");
	for (std::size_t size = 0; size < second_track.size(); ++size) {
		int tick = midiLoopTick(second_track.data(), size);
		if (tick != -1 && tick != 500) {
			check(tick, 500, "truncated file");
		}
	}
	Bytes huge_chunk = second_track;
	huge_chunk[18] = 0xFF; // Length of the first track
	check(loopTick(huge_chunk), -1, "track longer than the file");

	if (failures == 0) {
		std::cout << "All tests passed\n";
	}
	return failures == 0 ? 0 : 1;
}
