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

#include "map_location_dialog.h"
#include "core.h"
#include "common/dbstring.h"
#include "model/project.h"
#include "model/project_data.h"
#include <lcf/rpg/map.h>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <map>

namespace {
// Draws the map with the tile renderer of Core. Core renders one chipset at a
// time, so the chipset and panorama of the map editor are restored afterwards.
QPixmap renderMap(const lcf::rpg::Map& map) {
	const int tile = core().tileSize();

	const int old_chipset = core().chipsetId();
	const QString old_background = core().backgroundName();
	core().LoadChipset(map.chipset_id);
	core().LoadBackground(map.parallax_flag ? ToQString(map.parallax_name) : QString());

	QPixmap pixmap(map.width * tile, map.height * tile);
	pixmap.fill(Qt::black);

	core().beginPainting(pixmap);
	for (int y = 0; y < map.height; ++y) {
		for (int x = 0; x < map.width; ++x) {
			size_t index = static_cast<size_t>(y * map.width + x);
			QRect rect(x * tile, y * tile, tile, tile);
			if (index < map.lower_layer.size()) {
				core().renderTile(map.lower_layer[index], rect);
			}
			if (index < map.upper_layer.size()) {
				core().renderTile(map.upper_layer[index], rect);
			}
		}
	}

	// Events with a tile graphic need the chipset of the map
	for (const auto& event : map.events) {
		if (!event.pages.empty() && event.pages[0].character_name.empty() && event.pages[0].character_index != 0) {
			QRect rect(event.x * tile, event.y * tile, tile, tile);
			core().renderTile(static_cast<short>(event.pages[0].character_index + 10000), rect);
		}
	}
	core().endPainting();

	if (old_chipset != 0) {
		core().LoadChipset(old_chipset);
	}
	core().LoadBackground(old_background);

	// Draw the CharSet frame of the first page standing on the tile, like the
	// game does. Lower events are drawn last, so they overlap the ones behind.
	std::vector<const lcf::rpg::Event*> events;
	for (const auto& event : map.events) {
		events.push_back(&event);
	}
	std::stable_sort(events.begin(), events.end(), [](const auto* a, const auto* b) { return a->y < b->y; });

	QPainter painter(&pixmap);
	painter.setPen(Qt::white);
	painter.setBrush(QColor(0, 0, 0, 96));
	for (const auto* event : events) {
		QRect rect(event->x * tile, event->y * tile, tile, tile);
		QPixmap graphic = event->pages.empty() ? QPixmap() : core().eventGraphic(event->pages[0]);
		if (!graphic.isNull()) {
			// The frame is 24x32 game pixels, where a tile is 16 pixels
			const int w = 24 * tile / 16;
			const int h = 32 * tile / 16;
			painter.drawPixmap(QRect(rect.x() + (tile - w) / 2, rect.bottom() + 1 - h, w, h), graphic);
		} else if (event->pages.empty() || event->pages[0].character_index == 0) {
			// No graphic: mark the event, like RPG Maker does
			painter.drawRect(rect.adjusted(2, 2, -3, -3));
		}
	}

	return pixmap;
}
}

MapLocationDialog::MapLocationDialog(ProjectData& project, int map_id, int x, int y, bool allow_map_change, QWidget* parent) :
	QDialog(parent), m_project(project), m_x(x), m_y(y) {

	setWindowTitle(tr("Location"));
	resize(allow_map_change ? 900 : 640, 560);

	m_tree = new QTreeWidget(this);
	m_tree->setHeaderHidden(true);
	m_tree->setVisible(allow_map_change);

	m_scene = new QGraphicsScene(this);
	m_map = m_scene->addPixmap(QPixmap());
	m_cursor = m_scene->addRect(QRectF(), QPen(Qt::yellow, 2));
	m_cursor->setZValue(1);

	m_view = new QGraphicsView(m_scene, this);
	m_view->setBackgroundBrush(palette().dark());
	m_view->viewport()->installEventFilter(this);

	auto* splitter = new QSplitter(this);
	splitter->addWidget(m_tree);
	splitter->addWidget(m_view);
	splitter->setStretchFactor(1, 1);
	splitter->setSizes({ 220, 680 });

	m_label = new QLabel(this);

	auto* zoom_group = new QButtonGroup(this);
	auto* bottom = new QHBoxLayout();
	bottom->addWidget(m_label, 1);
	for (auto [text, zoom] : { std::pair{ "2/1", 2.0 }, { "1/1", 1.0 }, { "1/2", 0.5 }, { "1/4", 0.25 } }) {
		auto* button = new QToolButton(this);
		button->setText(text);
		button->setCheckable(true);
		button->setChecked(zoom == 1.0);
		zoom_group->addButton(button);
		connect(button, &QToolButton::clicked, this, [this, zoom] { setZoom(zoom); });
		bottom->addWidget(button);
	}

	m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	bottom->addWidget(m_buttons);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(splitter, 1);
	layout->addLayout(bottom);

	buildTree();
	showMap(map_id);
}

void MapLocationDialog::buildTree() {
	const auto& tree_map = m_project.treeMap();

	std::map<int, QTreeWidgetItem*> items;
	for (const auto& info : tree_map.maps) {
		auto* item = new QTreeWidgetItem({ ToQString(info.name) });
		item->setData(0, Qt::UserRole, info.ID);
		if (!m_project.isEditableMap(info.ID)) {
			// Root and areas have no map to pick a location on
			item->setForeground(0, palette().brush(QPalette::Disabled, QPalette::Text));
		}
		items[info.ID] = item;
	}

	// tree_order lists the maps in display order, parents before children
	for (int id : tree_map.tree_order) {
		auto it = items.find(id);
		if (it == items.end()) {
			continue;
		}
		auto info = std::find_if(tree_map.maps.begin(), tree_map.maps.end(), [id](const auto& m) { return m.ID == id; });
		auto parent = items.find(info->parent_map);
		if (id == 0 || parent == items.end() || parent->first == id) {
			m_tree->addTopLevelItem(it->second);
		} else {
			parent->second->addChild(it->second);
		}
	}
	m_tree->expandToDepth(1);

	connect(m_tree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* current) {
		if (!current) {
			return;
		}
		int id = current->data(0, Qt::UserRole).toInt();
		if (id != m_map_id && m_project.isEditableMap(id)) {
			showMap(id);
		}
	});
}

void MapLocationDialog::showMap(int map_id) {
	auto map = m_project.project().loadMap(map_id);
	if (!map) {
		return;
	}

	m_map_id = map_id;
	m_width = map->width;
	m_height = map->height;
	m_map->setPixmap(renderMap(*map));
	m_scene->setSceneRect(m_map->boundingRect());

	// Show the map in the tree without picking it again
	QSignalBlocker blocker(m_tree);
	auto found = m_tree->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive);
	for (auto* item : found) {
		if (item->data(0, Qt::UserRole).toInt() == map_id) {
			m_tree->setCurrentItem(item);
			m_tree->scrollToItem(item);
			break;
		}
	}

	selectTile(m_x, m_y);
	m_view->centerOn(m_cursor);
}

void MapLocationDialog::selectTile(int x, int y) {
	m_x = std::clamp(x, 0, std::max(m_width - 1, 0));
	m_y = std::clamp(y, 0, std::max(m_height - 1, 0));
	const int tile = core().tileSize();
	m_cursor->setRect(m_x * tile, m_y * tile, tile, tile);
	updateLabel();
}

void MapLocationDialog::setZoom(double zoom) {
	m_view->setTransform(QTransform::fromScale(zoom, zoom));
	// Smooth when zoomed out, sharp pixels when zoomed in
	m_view->setRenderHint(QPainter::SmoothPixmapTransform, zoom < 1.0);
	m_view->centerOn(m_cursor);
}

void MapLocationDialog::updateLabel() {
	QString name;
	for (const auto& info : m_project.treeMap().maps) {
		if (info.ID == m_map_id) {
			name = ToQString(info.name);
			break;
		}
	}
	m_label->setText(QString("%1:%2 (%3,%4)")
		.arg(m_map_id, 4, 10, QChar('0'))
		.arg(name)
		.arg(m_x, 3, 10, QChar('0'))
		.arg(m_y, 3, 10, QChar('0')));
	m_buttons->button(QDialogButtonBox::Ok)->setEnabled(m_map_id != 0);
}

void MapLocationDialog::showEvent(QShowEvent* event) {
	QDialog::showEvent(event);
	// The view has its size now
	m_view->centerOn(m_cursor);
}

bool MapLocationDialog::eventFilter(QObject* watched, QEvent* event) {
	if (watched == m_view->viewport()
		&& (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)) {
		auto* mouse = static_cast<QMouseEvent*>(event);
		if (mouse->button() == Qt::LeftButton && m_width > 0) {
			QPointF pos = m_view->mapToScene(mouse->position().toPoint());
			if (!m_scene->sceneRect().contains(pos)) {
				// Clicked beside a map smaller than the view
				return true;
			}
			const int tile = core().tileSize();
			selectTile(static_cast<int>(pos.x()) / tile, static_cast<int>(pos.y()) / tile);
			if (event->type() == QEvent::MouseButtonDblClick) {
				accept();
			}
			return true;
		}
	}
	return QDialog::eventFilter(watched, event);
}
