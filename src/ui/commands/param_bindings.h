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

#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QCoreApplication>
#include <QSpinBox>
#include <algorithm>
#include <vector>
#include <lcf/rpg/eventcommand.h>
#include "ui/common/rpg_combobox.h"

/**
 * Helpers to show the parameters of an event command in controls.
 * A parameter is only written when the user changes its control, so values
 * the controls can't show (out of range, unknown or from patches) are kept.
 */
namespace ParamBindings {
	/** Adds zeros up to count parameters, longer lists are kept as they are */
	inline void ensureParameters(lcf::rpg::EventCommand* cmd, size_t count) {
		if (cmd->parameters.size() < count) {
			std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
			params.resize(count, 0);
			cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
		}
	}

	inline void bindSpin(QSpinBox* spin, lcf::rpg::EventCommand* cmd, int index) {
		spin->setValue(cmd->parameters[index]);
		QObject::connect(spin, qOverload<int>(&QSpinBox::valueChanged), spin, [cmd, index](int value) {
			cmd->parameters[index] = value;
		});
	}

	/** Checked when the parameter is not 0, writes 1 or 0 */
	inline void bindCheck(QAbstractButton* check, lcf::rpg::EventCommand* cmd, int index) {
		check->setChecked(cmd->parameters[index] != 0);
		QObject::connect(check, &QAbstractButton::toggled, check, [cmd, index](bool checked) {
			cmd->parameters[index] = checked ? 1 : 0;
		});
	}

	/** The button IDs of the group are the parameter values. No button is checked for other values. */
	inline void bindRadios(QButtonGroup* group, lcf::rpg::EventCommand* cmd, int index) {
		if (auto* button = group->button(cmd->parameters[index])) {
			button->setChecked(true);
		}
		QObject::connect(group, &QButtonGroup::idToggled, group, [cmd, index](int id, bool checked) {
			if (checked) {
				cmd->parameters[index] = id;
			}
		});
	}

	/** Selects the item whose data is value, adding an "Unknown" item when there is none */
	inline void selectData(QComboBox* combo, int value) {
		int i = combo->findData(value);
		if (i == -1) {
			combo->addItem(QCoreApplication::translate("CommandBindings", "Unknown (%1)").arg(value), value);
			i = combo->count() - 1;
		}
		combo->setCurrentIndex(i);
	}

	/** The item data of the combo box are the parameter values */
	inline void bindComboData(QComboBox* combo, lcf::rpg::EventCommand* cmd, int index) {
		selectData(combo, cmd->parameters[index]);
		QObject::connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), combo, [combo, cmd, index](int i) {
			if (i >= 0) {
				cmd->parameters[index] = combo->itemData(i).toInt();
			}
		});
	}

	/** For database combo boxes (variables, chipsets...): IDs start at 1, the rows at 0 */
	inline void bindDatabaseCombo(RpgComboBoxBase* combo, lcf::rpg::EventCommand* cmd, int index) {
		auto* box = combo->comboBox();
		int row = cmd->parameters[index] - 1;
		box->setCurrentIndex(row >= 0 && row < box->count() ? row : -1);
		QObject::connect(box, qOverload<int>(&QComboBox::currentIndexChanged), combo, [cmd, index](int i) {
			if (i >= 0) {
				cmd->parameters[index] = i + 1;
			}
		});
	}
}
