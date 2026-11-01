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

#include "defines.h"

#ifdef Q_OS_WIN
#define PLAYER "Player.exe"
#else
#define PLAYER "easyrpg-player"
#endif

#include <QPixmap>
#include <QPainter>
#include <QListWidget>
#include <QQmlApplicationEngine>
#include <lcf/rpg/map.h>
#include <lcf/rpg/chipset.h>
#include "ui/other/run_game_dialog.h"
#include "model/project.h"

class Core : public QObject
{
	Q_OBJECT

public:

	enum Layer
	{
		LOWER,
		UPPER,
		EVENT
	};

	enum Tool
	{
		ZOOM,
		PENCIL,
		RECTANGLE,
		CIRCLE,
		FILL
	};

	Core();

	static Core* getCore();

	void LoadChipset(int n_chipsetid);
	void LoadBackground(QString name);
	/** ID of the loaded chipset, 0 when none */
	int chipsetId() const { return m_chipset.ID; }
	/** Name of the loaded panorama, empty when none */
	const QString& backgroundName() const { return m_backgroundName; }

	int tileSize();
	void setTileSize(int tileSize);

	QString rtpPath(const QString &folder, const QString &filename = QString()) const;

	Layer layer();
	void setLayer(const Layer &layer);

	Tool tool();
	void setTool(const Tool &tool);

	QString gameTitle();
	void setGameTitle(const QString &gameTitle);

	void beginPainting(QPixmap &dest);
	void renderTile(const short &tile_id, const QRect &dest_rect);
	void renderEvent(const lcf::rpg::Event& event, const QRect &dest_rect);
	/** Returns the 24x32 CharSet frame of an event page, or a null pixmap for tile graphics */
	QPixmap eventGraphic(const lcf::rpg::EventPage& page);
	void endPainting();

	QColor keycolor();

	short translate(int terrain_id, int _code = 0, int _scode = 0);
	int translate(short tile_id);

	inline bool chipsetIsNull() {return m_tileCache[0].isNull();}
	QPixmap createDummyPixmap(int width, int height);

	bool isWater(int terrain_id);
	bool isABWater(int terrain_id);
	bool isDWater(int terrain_id);
	bool isAnimation(int terrain_id);
	bool isDblock(int terrain_id);
	bool isEblock(int terrain_id);
	bool isFblock(int terrain_id);

	QString defDir() const;
	void setDefDir(const QString &defDir);

	void setRtpDir(const QString &n_path);

	short selection(int off_x, int off_y);
	int selWidth();
	int selHeight();
	void setSelection(std::vector<short> n_sel, int n_w, int n_h);

	/** ID of the map shown in the map editor, 0 when none */
	int currentMapId() const { return m_currentMapId; }
	void setCurrentMapId(int id) { m_currentMapId = id; }

	lcf::rpg::Event *currentMapEvent(int eventID);
	const QMap<int, lcf::rpg::Event *> *currentMapEvents() const { return m_currentMapEvents; }
	void setCurrentMapEvents(QMap<int, lcf::rpg::Event *> *events);

	void runGame();
	void runGameHere(int map_id, int x, int y);
	void runBattleTest(int troop_id);

	std::shared_ptr<Project>& project();
	const std::shared_ptr<Project>& project() const;

	QQmlApplicationEngine* qmlEngine();

signals:
	void toolChanged();

	void layerChanged();

	void chipsetChanged();

private:
	lcf::rpg::Map *m_map;
	lcf::rpg::Chipset m_chipset;
	int m_tileSize;
	QPainter m_painter;
	QString m_defDir;
	QString m_rtpDir;
	QColor m_keycolor;
	Layer m_layer;
	Tool m_tool;
	QPixmap m_background;
	QString m_backgroundName;
	int m_currentMapId = 0;
	QMap<int, QPixmap> m_tileCache;
	QMap<int, QPixmap> m_eventCache;
	QMap<int, short> m_dictionary;
	QMap<int, lcf::rpg::Map> m_maps;
	QMap<int, QWidget*> m_mapTabs;
	std::vector<short> m_lowerSel;
	std::vector<short> m_upperSel;
	short m_eventSel;
	int m_lowerSelW;
	int m_lowerSelH;
	int m_upperSelW;
	int m_upperSelH;
	static Core *core_instance;
	QMap<int, lcf::rpg::Event*> *m_currentMapEvents;
	RunGameDialog *m_runGameDialog;
	std::shared_ptr<Project> m_project;
	QQmlApplicationEngine* m_qml_engine = nullptr;
};

Core& core();
