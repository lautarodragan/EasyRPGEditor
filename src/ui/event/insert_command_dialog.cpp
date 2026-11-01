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

#include "insert_command_dialog.h"
#include "stringizer.h"
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <map>

using Category = EventCommandTemplates::Category;
using Template = EventCommandTemplates::Template;

namespace {
QString categoryName(Category category) {
	switch (category) {
		case Category::Messages: return InsertCommandDialog::tr("Messages");
		case Category::GameProgression: return InsertCommandDialog::tr("Game Progression");
		case Category::FlowControl: return InsertCommandDialog::tr("Flow Control");
		case Category::Party: return InsertCommandDialog::tr("Party");
		case Category::Actors: return InsertCommandDialog::tr("Actors");
		case Category::System: return InsertCommandDialog::tr("System");
		case Category::MapMovement: return InsertCommandDialog::tr("Map/Movement");
		case Category::Screen: return InsertCommandDialog::tr("Screen");
		case Category::Pictures: return InsertCommandDialog::tr("Pictures");
		case Category::Audio: return InsertCommandDialog::tr("Audio");
		case Category::Battle: return InsertCommandDialog::tr("Battle");
		case Category::Advanced: return InsertCommandDialog::tr("Advanced");
	}
	return {};
}
}

InsertCommandDialog::InsertCommandDialog(bool rpg2k3, bool battle, QWidget* parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Insert Command"));
	resize(380, 560);

	m_filter = new QLineEdit(this);
	m_filter->setPlaceholderText(tr("Search..."));
	m_filter->setClearButtonEnabled(true);

	m_tree = new QTreeWidget(this);
	m_tree->setHeaderHidden(true);

	m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(m_filter);
	layout->addWidget(m_tree);
	layout->addWidget(m_buttons);

	// The table is ordered by category
	std::map<Category, QTreeWidgetItem*> categories;
	for (const auto& tmpl : EventCommandTemplates::all()) {
		if (!EventCommandTemplates::isAvailable(tmpl, rpg2k3, battle)) {
			continue;
		}
		auto*& category = categories[tmpl.category];
		if (!category) {
			category = new QTreeWidgetItem(m_tree, {categoryName(tmpl.category)});
			category->setFlags(Qt::ItemIsEnabled);
		}
		auto* item = new QTreeWidgetItem(category, {Stringizer::tr(tmpl.name)});
		item->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(const_cast<Template*>(&tmpl))));
	}
	m_tree->expandAll();

	connect(m_filter, &QLineEdit::textChanged, this, &InsertCommandDialog::filter);
	connect(m_tree, &QTreeWidget::currentItemChanged, this, &InsertCommandDialog::updateButtons);
	connect(m_tree, &QTreeWidget::itemDoubleClicked, this, [this] {
		if (selected()) {
			accept();
		}
	});
	connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	updateButtons();
	m_filter->setFocus();
}

const Template* InsertCommandDialog::selected() const {
	auto* item = m_tree->currentItem();
	if (!item || item->isHidden()) {
		return nullptr;
	}
	return static_cast<const Template*>(item->data(0, Qt::UserRole).value<void*>());
}

void InsertCommandDialog::filter(const QString& text) {
	// The first command starting with the text, otherwise the first containing it
	QTreeWidgetItem* best = nullptr;
	bool best_starts = false;
	for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
		auto* category = m_tree->topLevelItem(i);
		bool category_matches = category->text(0).contains(text, Qt::CaseInsensitive);
		int visible = 0;
		for (int j = 0; j < category->childCount(); ++j) {
			auto* item = category->child(j);
			bool show = category_matches || item->text(0).contains(text, Qt::CaseInsensitive);
			item->setHidden(!show);
			if (show) {
				++visible;
				bool starts = item->text(0).startsWith(text, Qt::CaseInsensitive);
				if (!best || (starts && !best_starts)) {
					best = item;
					best_starts = starts;
				}
			}
		}
		category->setHidden(visible == 0);
	}

	// Select the best match, so Enter inserts it
	if (!text.isEmpty() && best) {
		m_tree->setCurrentItem(best);
	}
	updateButtons();
}

void InsertCommandDialog::updateButtons() {
	m_buttons->button(QDialogButtonBox::Ok)->setEnabled(selected() != nullptr);
}
