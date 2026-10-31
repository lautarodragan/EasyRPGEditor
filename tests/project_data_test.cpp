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

#include "model/project_data.h"
#include <iostream>

namespace {
int failures = 0;

void check(bool cond, const char* what) {
	if (!cond) {
		std::cerr << "FAIL: " << what << "\n";
		++failures;
	}
}

lcf::rpg::MapInfo makeInfo(int id, int type) {
	lcf::rpg::MapInfo info;
	info.ID = id;
	info.type = type;
	return info;
}
}

int main() {
	// Areas have an ID in the map tree but no MapXXXX.lmu file.
	// Opening one in the map editor dereferenced a null map and crashed.
	ProjectData data;
	auto& maps = data.treeMap().maps;
	maps.push_back(makeInfo(0, lcf::rpg::TreeMap::MapType_root));
	maps.push_back(makeInfo(1, lcf::rpg::TreeMap::MapType_map));
	maps.push_back(makeInfo(42, lcf::rpg::TreeMap::MapType_area));

	check(!data.isEditableMap(0), "root node is not an editable map");
	check(data.isEditableMap(1), "map is an editable map");
	check(!data.isEditableMap(42), "area is not an editable map");
	check(!data.isEditableMap(7), "unknown ID is not an editable map");

	if (failures == 0) {
		std::cout << "All tests passed\n";
	}
	return failures == 0 ? 0 : 1;
}
