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

#include "event_target_combobox.h"
#include "core.h"
#include "stringizer.h"
#include "common/dbstring.h"
#include <QComboBox>

void EventTargetComboBox::fill(QComboBox* combo, int current, bool with_hero_and_vehicles) {
	combo->clear();

	if (with_hero_and_vehicles) {
		for (int id : { hero, boat, ship, airship }) {
			combo->addItem(Stringizer::characterName(id), id);
		}
	}
	combo->addItem(Stringizer::characterName(this_event), this_event);

	if (const auto* events = core().currentMapEvents()) {
		for (auto it = events->begin(); it != events->end(); ++it) {
			combo->addItem(QString("%1: %2").arg(it.key(), 4, 10, QChar('0')).arg(ToQString(it.value()->name)), it.key());
		}
	}

	int index = combo->findData(current);
	if (index == -1) {
		// Event not on the current map, keep it selectable
		combo->addItem(Stringizer::characterName(current), current);
		index = combo->count() - 1;
	}
	combo->setCurrentIndex(index);
}
