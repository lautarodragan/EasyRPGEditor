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

class QFormLayout;

/**
 * Base of the editors of commands with a few parameters, mostly commands of
 * troop pages. A subclass builds its controls into the form in its
 * constructor and sets m_bind, which connects them to the parameters.
 * The controls only write the parameters the user changes.
 */
class SimpleCommandWidget : public EventCommandBaseWidget
{
	Q_OBJECT

public:
	SimpleCommandWidget(ProjectData& project, QWidget* parent);

	using EventCommandBaseWidget::setData;
	void setData(lcf::rpg::EventCommand* cmd) override;

protected:
	QFormLayout* m_form;
	std::function<void(lcf::rpg::EventCommand*)> m_bind;
};

/** Enter Hero Name: actor, initial letters, show the current name */
class EnterHeroNameWidget : public SimpleCommandWidget {
public:
	EnterHeroNameWidget(ProjectData& project, QWidget* parent);
};

/** Change Monster HP: monster, increase or decrease by a constant, variable or percentage, can kill */
class ChangeMonsterHPWidget : public SimpleCommandWidget {
public:
	ChangeMonsterHPWidget(ProjectData& project, QWidget* parent);
};

/** Change Monster MP: monster, increase or decrease by a constant or variable */
class ChangeMonsterMPWidget : public SimpleCommandWidget {
public:
	ChangeMonsterMPWidget(ProjectData& project, QWidget* parent);
};

/** Change Monster Condition: monster, inflict or remove, condition */
class ChangeMonsterConditionWidget : public SimpleCommandWidget {
public:
	ChangeMonsterConditionWidget(ProjectData& project, QWidget* parent);
};

/** Show Hidden Monster: monster */
class ShowHiddenMonsterWidget : public SimpleCommandWidget {
public:
	ShowHiddenMonsterWidget(ProjectData& project, QWidget* parent);
};

/** Change Battle BG: background file */
class ChangeBattleBGWidget : public SimpleCommandWidget {
public:
	ChangeBattleBGWidget(ProjectData& project, QWidget* parent);
};

/** Show Battle Animation of troop pages: animation, monster or party member, wait */
class ShowBattleAnimationBattleWidget : public SimpleCommandWidget {
public:
	ShowBattleAnimationBattleWidget(ProjectData& project, QWidget* parent);
};

/** Enable Combo: actor, battle command, number of times */
class EnableComboWidget : public SimpleCommandWidget {
public:
	EnableComboWidget(ProjectData& project, QWidget* parent);
};

/** Force Flee: party, all monsters or one monster, ignoring the battle condition */
class ForceFleeWidget : public SimpleCommandWidget {
public:
	ForceFleeWidget(ProjectData& project, QWidget* parent);
};
