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

#include "command_bindings.h"
#include "common/dbstring.h"
#include "defines.h"
#include "ui/common/rpg_combobox.h"
#include "ui/picker/picker_backdrop_widget.h"
#include "ui/picker/picker_dialog.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QSpinBox>
#include <vector>

namespace CommandBindings {

int32_t param(const lcf::rpg::EventCommand& cmd, size_t index, int32_t fallback) {
	return index < cmd.parameters.size() ? cmd.parameters[index] : fallback;
}

void setParam(lcf::rpg::EventCommand& cmd, size_t index, int32_t value) {
	if (index >= cmd.parameters.size()) {
		std::vector<int32_t> params(cmd.parameters.begin(), cmd.parameters.end());
		params.resize(index + 1, 0);
		cmd.parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	}
	cmd.parameters[index] = value;
}

namespace {
void notify(const Changed& changed) {
	if (changed) {
		changed();
	}
}
}

void bind(QSpinBox* spin, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback, Changed changed) {
	{
		QSignalBlocker blocker(spin);
		spin->setValue(param(*cmd, index, fallback));
	}
	QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged), spin, [=](int value) {
		setParam(*cmd, index, value);
		notify(changed);
	});
}

void bind(QAbstractButton* check, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback, Changed changed) {
	{
		QSignalBlocker blocker(check);
		check->setChecked(param(*cmd, index, fallback) != 0);
	}
	QObject::connect(check, &QAbstractButton::toggled, check, [=](bool checked) {
		setParam(*cmd, index, checked ? 1 : 0);
		notify(changed);
	});
}

void bind(QButtonGroup* group, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback, Changed changed) {
	if (auto* button = group->button(param(*cmd, index, fallback))) {
		QSignalBlocker blocker(group);
		button->setChecked(true);
	}
	QObject::connect(group, &QButtonGroup::idToggled, group, [=](int id, bool checked) {
		if (checked) {
			setParam(*cmd, index, id);
			notify(changed);
		}
	});
}

void bind(QComboBox* combo, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback, Changed changed) {
	{
		QSignalBlocker blocker(combo);
		selectData(combo, param(*cmd, index, fallback));
	}
	QObject::connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), combo, [=](int row) {
		if (row >= 0) {
			setParam(*cmd, index, combo->itemData(row).toInt());
			notify(changed);
		}
	});
}

void bind(RpgComboBoxBase* combo, lcf::rpg::EventCommand* cmd, size_t index, int32_t fallback, Changed changed) {
	auto* box = combo->comboBox();
	{
		QSignalBlocker blocker(box);
		int id = param(*cmd, index, fallback);
		// IDs outside of the database show an empty selection
		box->setCurrentIndex(id >= 1 && id <= box->count() ? id - 1 : -1);
	}
	QObject::connect(box, qOverload<int>(&QComboBox::currentIndexChanged), combo, [=](int row) {
		if (row >= 0) {
			setParam(*cmd, index, row + 1);
			notify(changed);
		}
	});
}

void bindString(QLineEdit* edit, lcf::rpg::EventCommand* cmd, Changed changed) {
	{
		QSignalBlocker blocker(edit);
		edit->setText(ToQString(cmd->string));
	}
	QObject::connect(edit, &QLineEdit::textChanged, edit, [=](const QString& text) {
		cmd->string = ToDBString(text);
		notify(changed);
	});
}

void addNumbers(QComboBox* combo, int first, int count, int offset, const QString& format) {
	for (int value = first; value < first + count; ++value) {
		combo->addItem(format.arg(value + offset), value);
	}
}

void selectData(QComboBox* combo, int32_t value) {
	int row = combo->findData(value);
	if (row == -1) {
		combo->addItem(unknownText(value), value);
		row = combo->count() - 1;
	}
	combo->setCurrentIndex(row);
}

QString unknownText(int32_t value) {
	return QStringLiteral("<%1?>").arg(value);
}

void pickBackdrop(ProjectData& project, QLineEdit* edit, QWidget* parent) {
	auto* widget = new PickerBackdropWidget(parent);
	PickerDialog dialog(project, FileFinder::FileType::Image, widget, parent);
	QObject::connect(&dialog, &PickerDialog::fileSelected, edit, [edit](const QString& baseName) {
		edit->setText(baseName);
	});
	dialog.setDirectoryAndFile(BACKDROP, edit->text());
	dialog.exec();
}

void addMonsters(QComboBox* combo) {
	// The commands count the members of the troop from 0
	addNumbers(combo, 0, 8, 1, QObject::tr("Monster %1"));
}

}
