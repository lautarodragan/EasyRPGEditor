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

#include "open_shop_widget.h"
#include "command_bindings.h"
#include "common/dbstring.h"
#include "model/project_data.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <algorithm>

using namespace CommandBindings;

namespace {
// Parameters as read by Game_Interpreter_Map::CommandOpenShop of the Player
enum Param {
	ParamType = 0, // 0 buy and sell, 1 buy only, 2 sell only
	ParamStyle = 1, // messages of shop A, B or C
	ParamHandlers = 2,
	// 3 is unused
	ParamGoods = 4, // the goods follow until the end
};
}

OpenShopWidget::OpenShopWidget(ProjectData& project, QWidget* parent) :
	BlockCommandWidget(project, parent) {

	auto* layout = new QHBoxLayout(this);
	auto* left = new QVBoxLayout;

	auto* type_box = new QGroupBox(tr("Shop type"));
	auto* type_layout = new QVBoxLayout(type_box);
	m_type_group = new QButtonGroup(this);
	int id = 0;
	for (const QString& text : { tr("Buy and sell"), tr("Buy only"), tr("Sell only") }) {
		auto* radio = new QRadioButton(text);
		m_type_group->addButton(radio, id++);
		type_layout->addWidget(radio);
	}
	left->addWidget(type_box);

	auto* style_box = new QGroupBox(tr("Messages"));
	auto* style_layout = new QVBoxLayout(style_box);
	m_style = new QComboBox;
	m_style->addItem(tr("Shop A"), 0);
	m_style->addItem(tr("Shop B"), 1);
	m_style->addItem(tr("Shop C"), 2);
	style_layout->addWidget(m_style);
	left->addWidget(style_box);

	m_handlers = new QCheckBox(tr("Handlers for transaction and no transaction"));
	left->addWidget(m_handlers);
	left->addWidget(warningLabel());
	left->addStretch(1);
	layout->addLayout(left);

	auto* goods_box = new QGroupBox(tr("Goods"));
	auto* goods_layout = new QVBoxLayout(goods_box);
	m_filter = new QLineEdit;
	m_filter->setPlaceholderText(tr("Filter"));
	m_filter->setClearButtonEnabled(true);
	goods_layout->addWidget(m_filter);
	m_items = new QListWidget;
	m_items->setMinimumSize(260, 300);
	goods_layout->addWidget(m_items);
	layout->addWidget(goods_box, 1);

	connect(m_filter, &QLineEdit::textChanged, this, &OpenShopWidget::filterItems);
}

void OpenShopWidget::load() {
	bind(m_type_group, &m_edit, ParamType);
	bind(m_style, &m_edit, ParamStyle);
	bind(m_handlers, &m_edit, ParamHandlers, 0, [this] { branchesChanged(); });

	m_goods.clear();
	for (size_t i = ParamGoods; i < m_edit.parameters.size(); ++i) {
		m_goods.push_back(m_edit.parameters[i]);
	}

	{
		QSignalBlocker blocker(m_items);
		m_items->clear();
		auto add = [this](int32_t id, const QString& name) {
			auto* item = new QListWidgetItem(QStringLiteral("%1: %2").arg(id, 4, 10, QChar('0')).arg(name), m_items);
			item->setData(Qt::UserRole, id);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			bool sold = std::find(m_goods.begin(), m_goods.end(), id) != m_goods.end();
			item->setCheckState(sold ? Qt::Checked : Qt::Unchecked);
		};
		const auto& items = m_project.database().items;
		for (const auto& item : items) {
			add(item.ID, ToQString(item.name));
		}
		// Goods that aren't in the database
		for (int32_t id : m_goods) {
			if ((id < 1 || id > static_cast<int>(items.size())) && m_items->findItems(unknownText(id), Qt::MatchEndsWith).isEmpty()) {
				add(id, unknownText(id));
			}
		}
	}
	connect(m_items, &QListWidget::itemChanged, this, &OpenShopWidget::itemChanged);
}

void OpenShopWidget::itemChanged(QListWidgetItem* item) {
	const int32_t id = item->data(Qt::UserRole).toInt();
	if (item->checkState() == Qt::Checked) {
		if (std::find(m_goods.begin(), m_goods.end(), id) == m_goods.end()) {
			m_goods.push_back(id);
		}
	} else {
		m_goods.erase(std::remove(m_goods.begin(), m_goods.end(), id), m_goods.end());
	}

	// The goods follow the first four parameters
	std::vector<int32_t> params(m_edit.parameters.begin(), m_edit.parameters.end());
	params.resize(ParamGoods, 0);
	params.insert(params.end(), m_goods.begin(), m_goods.end());
	m_edit.parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
}

void OpenShopWidget::filterItems(const QString& text) {
	for (int i = 0; i < m_items->count(); ++i) {
		auto* item = m_items->item(i);
		item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
	}
}
