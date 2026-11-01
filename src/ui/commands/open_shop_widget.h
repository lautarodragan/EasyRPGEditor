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

#include "block_command_widget.h"
#include <vector>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

/**
 * Edits an Open Shop: buy and sell, message style, the goods and the
 * transaction handlers (with their branches).
 */
class OpenShopWidget : public BlockCommandWidget
{
	Q_OBJECT

public:
	OpenShopWidget(ProjectData& project, QWidget* parent);

protected:
	void load() override;

private:
	void itemChanged(QListWidgetItem* item);
	void filterItems(const QString& text);

	QButtonGroup* m_type_group;
	QComboBox* m_style;
	QCheckBox* m_handlers;
	QLineEdit* m_filter;
	QListWidget* m_items;
	/** Item IDs in the order of the parameters */
	std::vector<int32_t> m_goods;
};
