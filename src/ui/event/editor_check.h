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

#include <QString>

/**
 * Opens the editors of a project and accepts them without changes, which
 * must leave the project as it was: the editor of every event command (maps,
 * common events and troop pages), the event dialog of every event, the map
 * properties of every map and the database dialog with every entry selected.
 * Prints what changed. Nothing is saved.
 *
 * @param project_dir Folder of the project
 * @return 0 when nothing changed, 1 when something did, 2 when the project can't be loaded
 */
int checkEditors(const QString& project_dir);
