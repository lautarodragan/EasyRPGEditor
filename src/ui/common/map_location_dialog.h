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

#include <QDialog>

class ProjectData;
class QDialogButtonBox;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;

/**
 * Picks a map and a tile on it, like the location dialog of RPG Maker:
 * a map tree, the map itself and the selected location.
 */
class MapLocationDialog : public QDialog {
	Q_OBJECT

public:
	/**
	 * @param project Project the maps belong to
	 * @param map_id Map shown first
	 * @param x X of the selected tile
	 * @param y Y of the selected tile
	 * @param allow_map_change Whether the map tree is shown to pick another map
	 * @param parent Parent widget
	 */
	MapLocationDialog(ProjectData& project, int map_id, int x, int y, bool allow_map_change, QWidget* parent = nullptr);

	int mapId() const { return m_map_id; }
	int x() const { return m_x; }
	int y() const { return m_y; }

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	void buildTree();
	void showMap(int map_id);
	void selectTile(int x, int y);
	void setZoom(double zoom);
	void updateLabel();

	ProjectData& m_project;
	int m_map_id = 0;
	int m_x = 0;
	int m_y = 0;
	int m_width = 0;
	int m_height = 0;

	QTreeWidget* m_tree = nullptr;
	QGraphicsView* m_view = nullptr;
	QGraphicsScene* m_scene = nullptr;
	QGraphicsPixmapItem* m_map = nullptr;
	QGraphicsRectItem* m_cursor = nullptr;
	QLabel* m_label = nullptr;
	QDialogButtonBox* m_buttons = nullptr;
};
