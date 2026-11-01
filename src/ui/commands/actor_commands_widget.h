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

#include "ui/event/event_command_base_widget.h"
#include <functional>
#include <memory>
#include <utility>
#include <vector>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QRadioButton;
class QSpinBox;
class QVBoxLayout;

/**
 * Base of the editors of the commands that change actors: Change HP, SP,
 * Parameters, Skills, Equipment, Condition, Class and Battle Commands.
 *
 * All of them start with the target (parameter 0: 0 entire party, 1 actor,
 * 2 actor ID from a variable; parameter 1: actor or variable ID).
 *
 * The controls show the parameters and write a parameter only when the user
 * changes its control: values the editor can't show are kept until edited.
 */
class ActorCommandWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	ActorCommandWidget(ProjectData& project, QWidget* parent);

	void setData(lcf::rpg::EventCommand* cmd) override;

protected:
	/** Constant value or variable, as a type and a value parameter */
	struct Operand {
		QRadioButton* radio_constant = nullptr;
		QRadioButton* radio_variable = nullptr;
		QComboBox* combo_variable = nullptr;
		QWidget* constant = nullptr;
		std::function<int()> constant_value;
		int idx_type = 0;
		int idx_value = 0;

		/** 0: constant, 1: variable, -1: another type (none selected) */
		int type() const;
		int value() const;
	};

	/** Adds the controls of the command, after the target */
	virtual void build() = 0;

	int param(int index) const;
	void setParam(int index, int value);

	QGroupBox* addGroup(const QString& title);

	/** Radio buttons in a row whose IDs are the values of parameter index */
	QButtonGroup* addChoice(const QString& title, int index, const QStringList& labels,
			std::function<void(int)> changed = {});

	/**
	 * Constant or variable. The constant is edited with constant, whose value
	 * is read with constant_value. Call writeOperand when the constant changes.
	 * Without load, the parameters hold something else and the controls show a constant.
	 */
	Operand& addOperand(QGroupBox* group, int idx_type, int idx_value, QWidget* constant,
			std::function<int()> constant_value, bool load = true);
	void writeOperand(const Operand& operand);
	void updateOperand(const Operand& operand, bool enabled = true);

	QSpinBox* makeSpin(int max, int value);
	/** Entries of a combo box: ID and name */
	using Entries = std::vector<std::pair<int, QString>>;

	/** Lists the entries as "0001: Name". Selects current, adding it when it is missing. */
	QComboBox* makeCombo(const Entries& entries, int current);
	Entries variables() const;

	/** Lists names whose values are their positions, adds current when it is out of range */
	QComboBox* makeValueCombo(const QStringList& names, int current);
	void bindCombo(QComboBox* combo, int index, std::function<void(int)> changed = {});
	void bindCheck(QCheckBox* check, int index);

	static int comboId(QComboBox* combo);
	static void selectId(QComboBox* combo, int id);

	QVBoxLayout* m_layout = nullptr;
	// Pointers stay valid: Operands are never removed
	std::vector<std::unique_ptr<Operand>> m_operands;

private:
	void addTarget();
};

/** Parameters: target (2), 0 increase / 1 decrease, operand (2), can kill */
class ChangeHPWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/** Parameters: target (2), 0 increase / 1 decrease, operand (2) */
class ChangeSPWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/** Parameters: target (2), 0 increase / 1 decrease, parameter, operand (2) */
class ChangeParametersWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/** Parameters: target (2), 0 learn / 1 forget, skill operand (2) */
class ChangeSkillsWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/**
 * Parameters: target (2), 0 equip / 1 unequip, then
 * equip: item operand (2), unequip: slot (0-4 weapon to accessory, 5 all)
 */
class ChangeEquipmentWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/** Parameters: target (2), 0 inflict / 1 remove, condition ID */
class ChangeConditionWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/**
 * RPG Maker 2003. Parameters: target (2), class ID (0: none), level 1,
 * skill mode, parameter mode, show a message
 */
class ChangeClassWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};

/**
 * RPG Maker 2003. Parameters: target (2), battle command ID (removing 0
 * removes all), 0 remove / 1 add
 */
class ChangeBattleCommandsWidget : public ActorCommandWidget
{
	Q_OBJECT
public:
	using ActorCommandWidget::ActorCommandWidget;
protected:
	void build() override;
};
