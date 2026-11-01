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

#include "show_inn_widget.h"
#include "command_bindings.h"
#include "ui/common/rpg_spinbox.h"
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>

using namespace CommandBindings;

namespace {
// Parameters as read by Game_Interpreter_Map::CommandShowInn of the Player
enum Param {
	ParamStyle = 0, // messages of inn A or B
	ParamPrice = 1, // 0 skips the question
	ParamHandlers = 2,
};
}

ShowInnWidget::ShowInnWidget(ProjectData& project, QWidget* parent) :
	BlockCommandWidget(project, parent) {

	auto* layout = new QVBoxLayout(this);
	auto* form = new QFormLayout;
	m_style = new QComboBox;
	m_style->addItem(tr("Inn A"), 0);
	m_style->addItem(tr("Inn B"), 1);
	form->addRow(tr("Messages"), m_style);
	m_price = new RpgSpinBox;
	m_price->setRange(0, 999999);
	m_price->setToolTip(tr("An inn for free doesn't ask whether to stay"));
	form->addRow(tr("Price"), m_price);
	layout->addLayout(form);
	m_handlers = new QCheckBox(tr("Handlers for stay and no stay"));
	layout->addWidget(m_handlers);
	layout->addWidget(warningLabel());
	layout->addStretch(1);
}

void ShowInnWidget::load() {
	bind(m_style, &m_edit, ParamStyle);
	bind(m_price, &m_edit, ParamPrice);
	bind(m_handlers, &m_edit, ParamHandlers, 0, [this] { branchesChanged(); });
}
