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
#include <type_traits>
#include <lcf/rpg/eventcommand.h>
#include "model/event_command_list.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QAbstractItemModel>
#include <lcf/rpg/fwd.h>
#include "model/project.h"
#include "ui/database/actor_widget.h"
#include "ui/database/attribute_widget.h"
#include "ui/database/battle_animation2_widget.h"
#include "ui/database/battle_animation_widget.h"
#include "ui/database/chipset_widget.h"
#include "ui/database/class_widget.h"
#include "ui/database/common_event_widget.h"
#include "ui/database/enemy_group_widget.h"
#include "ui/database/enemy_widget.h"
#include "ui/database/item_widget.h"
#include "ui/database/skill_widget.h"
#include "ui/database/state_widget.h"
#include "ui/database/terrain_widget.h"

class WidgetAsDialogWrapperBase : public QDialog {
	Q_OBJECT
public:
	explicit WidgetAsDialogWrapperBase(QWidget* parent = nullptr) : QDialog(parent) {
		buttonBox = new QDialogButtonBox(this);
		buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok|QDialogButtonBox::Help);

		QObject::connect(buttonBox, &QDialogButtonBox::clicked, this, &WidgetAsDialogWrapperBase::clicked);
	}

	void clicked(QAbstractButton* button) {
		switch (buttonBox->standardButton(button)) {
			case QDialogButtonBox::Ok:
				accept();
				break;
			case QDialogButtonBox::Cancel:
			case QDialogButtonBox::Close:
				reject();
				break;
			case QDialogButtonBox::Apply:
				apply();
				break;
			case QDialogButtonBox::Help:
				help();
				break;
			default:
				break;
		}
	}

	void accept() override {
		apply();
		QDialog::accept();
	};

	virtual void apply() {}

	void reject() override {
		QDialog::reject();
	}

	void help() {
		// FIXME: not implemented
	}

signals:
	void valueSelected(int newValue);

protected:
	QDialogButtonBox* buttonBox = nullptr;
};

template <typename T>
class HasApply {
private:
	typedef char YesType[1];
	typedef char NoType[2];
	template <typename C> static YesType& test( decltype(&C::apply) ) ;
	template <typename C> static NoType& test(...);
public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

/**
 * Wraps a Widget inside a Dialog and applies the forwarded data when OK or Apply
 * are pressed
 * @tparam WIDGET Type of the managed widget
 * @tparam DATA Data forwarded to the widget
 */
template<typename WIDGET, typename DATA>
class WidgetAsDialogWrapper : public WidgetAsDialogWrapperBase {
public:
	explicit WidgetAsDialogWrapper(ProjectData& project, DATA& data, QWidget* parent = nullptr) :
	WidgetAsDialogWrapperBase(parent), dataOriginal(data) {
		dataCopy = data;
		if constexpr (std::is_same_v<DATA, lcf::rpg::EventCommand>) {
			originalCode = data.code;
			originalParameterCount = data.parameters.size();
		} else if constexpr (std::is_same_v<DATA, EventCommandList>) {
			originalCode = data.command().code;
			originalParameterCount = data.command().parameters.size();
		}

		wrappedWidget = new WIDGET(project, this);
		wrappedWidget->setData(&dataCopy);

		auto* verticalLayout = new QVBoxLayout(this);
		verticalLayout->addWidget(wrappedWidget);
		verticalLayout->addWidget(buttonBox);
		setLayout(verticalLayout);
	}

	WIDGET* widget() const {
		return wrappedWidget;
	}

private:
	void apply() override {
		if constexpr (HasApply<WIDGET>::value) {
			wrappedWidget->apply();
		}

		if constexpr (std::is_same_v<DATA, lcf::rpg::EventCommand>) {
			trimPadding(dataCopy);
		} else if constexpr (std::is_same_v<DATA, EventCommandList>) {
			if (dataCopy.index() < static_cast<int>(dataCopy.size())) {
				trimPadding(dataCopy.command());
			}
		}

		dataOriginal = dataCopy;
	}

	/**
	 * Editors add zero parameters up to the ones they show. Drop them again
	 * when they are still zero: the Player reads missing parameters as 0, and
	 * commands keep the size they were saved with (e.g. by RPG Maker 2000).
	 */
	void trimPadding(lcf::rpg::EventCommand& cmd) const {
		if (cmd.code != originalCode || cmd.parameters.size() <= originalParameterCount) {
			return;
		}
		for (size_t i = originalParameterCount; i < cmd.parameters.size(); ++i) {
			if (cmd.parameters[i] != 0) {
				return;
			}
		}
		cmd.parameters = lcf::DBArray<int32_t>(cmd.parameters.begin(), cmd.parameters.begin() + originalParameterCount);
	}

	int originalCode = 0;
	size_t originalParameterCount = 0;

	DATA& dataOriginal;
	DATA dataCopy;
	WIDGET* wrappedWidget = nullptr;
};
