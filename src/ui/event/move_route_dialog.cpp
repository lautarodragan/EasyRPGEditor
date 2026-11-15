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

#include "move_route_dialog.h"
#include "ui/common/move_route_editor.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QVBoxLayout>

MoveRouteDialog::MoveRouteDialog(ProjectData& project, lcf::rpg::MoveRoute& route, QWidget* parent) :
	QDialog(parent), m_route(route) {

	setWindowTitle(tr("Movement Pattern"));

	m_editor = new MoveRouteEditor(this);
	m_editor->setProject(project);
	m_editor->setRoute(route.move_commands);

	auto* options = new QGroupBox(tr("Options"), this);
	auto* options_layout = new QVBoxLayout(options);
	m_repeat = new QCheckBox(tr("Repeat Action"), options);
	m_repeat->setChecked(route.repeat);
	m_skip = new QCheckBox(tr("Skip if Cannot Move"), options);
	m_skip->setChecked(route.skippable);
	options_layout->addWidget(m_repeat);
	options_layout->addWidget(m_skip);
	m_editor->addBelowList(options);

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(m_editor);
	layout->addWidget(buttons);
}

void MoveRouteDialog::accept() {
	m_route.move_commands = m_editor->route();
	m_route.repeat = m_repeat->isChecked();
	m_route.skippable = m_skip->isChecked();
	QDialog::accept();
}
