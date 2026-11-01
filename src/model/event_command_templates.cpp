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

#include "event_command_templates.h"
#include <algorithm>
#include <string_view>

// Marks the names for lupdate, they are translated where they are shown
#ifndef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(scope, x) x
#endif

namespace EventCommandTemplates {

using Cmd = lcf::rpg::EventCommand::Code;
using Cat = Category;

const std::vector<Template>& all() {
	// Parameters follow RPG Maker 2003 and the EasyRPG Player.
	// Event targets: 10001 is the hero, 10005 this event.
	// Actor targets: 0 is the entire party, 1 a fixed actor, 2 an actor from a variable.
	static const std::vector<Template> templates = {
		// Messages
		{ Cmd::ShowMessage, Cat::Messages, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Show Message"), {} },
		// transparent, position bottom, avoid covering the hero, block parallel events
		{ Cmd::MessageOptions, Cat::Messages, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Message Options"), {0, 2, 1, 0} },
		// no face (empty name), left, not flipped
		{ Cmd::ChangeFaceGraphic, Cat::Messages, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Face Graphic"), {0, 0, 0} },
		// cancel selects choice 2
		{ Cmd::ShowChoice, Cat::Messages, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Show Choice"), {2}, "Yes/No" },
		// 1 digit into variable 1
		{ Cmd::InputNumber, Cat::Messages, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Input Number"), {1, 1} },

		// Game progression
		// switch 1 ON
		{ Cmd::ControlSwitches, Cat::GameProgression, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Control Switches"), {0, 1, 1, 0} },
		// variable 1 = 0
		{ Cmd::ControlVars, Cat::GameProgression, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Control Variables"), {0, 1, 1, 0, 0, 0, 0} },
		// set timer 1 to 0 seconds, the last parameter (timer 2) is RPG Maker 2003 only
		{ Cmd::TimerOperation, Cat::GameProgression, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Timer Operation"), {0, 0, 0, 0, 0, 0}, "", 5 },

		// Flow control
		// switch 1 is ON, with else branch
		{ Cmd::ConditionalBranch, Cat::FlowControl, Map, QT_TRANSLATE_NOOP("Stringizer", "Conditional Branch"), {0, 1, 0, 0, 0, 1} },
		{ Cmd::ConditionalBranch_B, Cat::FlowControl, Battle, QT_TRANSLATE_NOOP("Stringizer", "Conditional Branch"), {0, 1, 0, 0, 0, 1} },
		{ Cmd::Loop, Cat::FlowControl, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Loop"), {} },
		{ Cmd::BreakLoop, Cat::FlowControl, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Break Loop"), {} },
		{ Cmd::Label, Cat::FlowControl, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Label"), {1} },
		{ Cmd::JumpToLabel, Cat::FlowControl, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Jump To Label"), {1} },
		// common event 1. Battle events of RPG Maker 2003 use Call Common Event instead.
		{ Cmd::CallEvent, Cat::FlowControl, Map | Battle2k, QT_TRANSLATE_NOOP("Stringizer", "Call Event"), {0, 1, 0} },
		{ Cmd::CallCommonEvent, Cat::FlowControl, Battle2k3, QT_TRANSLATE_NOOP("Stringizer", "Call Common Event"), {1} },
		{ Cmd::EndEventProcessing, Cat::FlowControl, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "End Event Processing"), {} },
		{ Cmd::EraseEvent, Cat::FlowControl, Map, QT_TRANSLATE_NOOP("Stringizer", "Erase Event"), {} },
		// 1 second, the last parameter (wait for key) is RPG Maker 2003 only
		{ Cmd::Wait, Cat::FlowControl, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Wait"), {10, 0}, "", 1 },

		// Party
		// add 0 gold
		{ Cmd::ChangeGold, Cat::Party, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Gold"), {0, 0, 0} },
		// add 1 of item 1
		{ Cmd::ChangeItems, Cat::Party, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Items"), {0, 0, 1, 0, 1} },
		// add actor 1
		{ Cmd::ChangePartyMembers, Cat::Party, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Party Members"), {0, 0, 1} },
		// buy and sell, no handlers, no goods
		{ Cmd::OpenShop, Cat::Party, Map, QT_TRANSLATE_NOOP("Stringizer", "Open Shop"), {0, 0, 0, 0} },
		// free, no handlers
		{ Cmd::ShowInn, Cat::Party, Map, QT_TRANSLATE_NOOP("Stringizer", "Show Inn"), {0, 0, 0} },

		// Actors
		{ Cmd::ChangeExp, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Exp"), {0, 0, 0, 0, 1, 0} },
		{ Cmd::ChangeLevel, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Level"), {0, 0, 0, 0, 1, 0} },
		{ Cmd::ChangeParameters, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Parameters"), {0, 0, 0, 0, 0, 1} },
		{ Cmd::ChangeSkills, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Skills"), {0, 0, 0, 0, 1} },
		// equip item 1
		{ Cmd::ChangeEquipment, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Equipment"), {0, 0, 0, 0, 1} },
		{ Cmd::ChangeHP, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change HP"), {0, 0, 0, 0, 1, 0} },
		{ Cmd::ChangeSP, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change SP"), {0, 0, 0, 0, 1} },
		// inflict condition 1
		{ Cmd::ChangeCondition, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Condition"), {0, 0, 0, 1} },
		{ Cmd::FullHeal, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Full Heal"), {0, 0} },
		{ Cmd::SimulatedAttack, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Simulated Attack"), {0, 0, 0, 0, 0, 0, 0, 0} },
		{ Cmd::ChangeHeroName, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Hero Name"), {1} },
		{ Cmd::ChangeHeroTitle, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Hero Title"), {1} },
		{ Cmd::ChangeSpriteAssociation, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Sprite Association"), {1, 0, 0} },
		{ Cmd::ChangeActorFace, Cat::Actors, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change Actor Face"), {1, 0} },
		// actor 1 to class 1
		{ Cmd::ChangeClass, Cat::Actors, Rpg2k3, QT_TRANSLATE_NOOP("Stringizer", "Change Class"), {1, 1, 1, 0, 0, 0, 0} },
		// add battle command 1 to actor 1
		{ Cmd::ChangeBattleCommands, Cat::Actors, Rpg2k3, QT_TRANSLATE_NOOP("Stringizer", "Change Battle Commands"), {1, 1, 1, 1, 0} },

		// System
		{ Cmd::ChangeSystemBGM, Cat::System, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change System BGM"), {0, 0, 100, 100, 50} },
		{ Cmd::ChangeSystemSFX, Cat::System, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change System SFX"), {0, 100, 100, 50} },
		{ Cmd::ChangeSystemGraphics, Cat::System, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Change System Graphics"), {0, 0} },
		{ Cmd::ChangeScreenTransitions, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Screen Transitions"), {0, 0} },
		{ Cmd::ChangeTeleportAccess, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Teleport Access"), {0} },
		{ Cmd::ChangeEscapeAccess, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Escape Access"), {0} },
		{ Cmd::ChangeSaveAccess, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Save Access"), {0} },
		{ Cmd::ChangeMainMenuAccess, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Main Menu Access"), {0} },
		{ Cmd::OpenSaveMenu, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Open Save Menu"), {} },
		{ Cmd::OpenMainMenu, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Open Main Menu"), {} },
		// actor 1, letters, show the current name
		{ Cmd::EnterHeroName, Cat::System, Map, QT_TRANSLATE_NOOP("Stringizer", "Enter Hero Name"), {1, 0, 1} },
		{ Cmd::GameOver, Cat::System, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Game Over"), {} },
		{ Cmd::ReturntoTitleScreen, Cat::System, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Return to Title Screen"), {} },
		// RPG Maker 2003 1.10 and newer
		{ Cmd::OpenLoadMenu, Cat::System, Map2k3, QT_TRANSLATE_NOOP("Stringizer", "Open Load Menu"), {} },
		{ Cmd::ExitGame, Cat::System, Map2k3, QT_TRANSLATE_NOOP("Stringizer", "Exit Game"), {} },
		{ Cmd::ToggleAtbMode, Cat::System, Map2k3, QT_TRANSLATE_NOOP("Stringizer", "Toggle ATB Mode"), {} },
		{ Cmd::ToggleFullscreen, Cat::System, Map2k3, QT_TRANSLATE_NOOP("Stringizer", "Toggle Fullscreen"), {} },
		{ Cmd::OpenVideoOptions, Cat::System, Map2k3, QT_TRANSLATE_NOOP("Stringizer", "Open Video Options"), {} },

		// Map and movement
		// map 1 (0,0), the last parameter (direction) is RPG Maker 2003 only
		{ Cmd::Teleport, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Teleport"), {1, 0, 0, 0}, "", 3 },
		// into variables 1, 2 and 3
		{ Cmd::MemorizeLocation, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Memorize Location"), {1, 2, 3} },
		{ Cmd::RecallToLocation, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Recall To Location"), {1, 2, 3} },
		{ Cmd::EnterExitVehicle, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Enter/Exit Vehicle"), {} },
		// boat to map 1 (0,0)
		{ Cmd::SetVehicleLocation, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Set Vehicle Location"), {0, 0, 1, 0, 0} },
		{ Cmd::ChangeVehicleGraphic, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Vehicle Graphic"), {0, 0} },
		// this event to (0,0), the last parameter (direction) is RPG Maker 2003 only
		{ Cmd::ChangeEventLocation, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Event Location"), {10005, 0, 0, 0, 0}, "", 4 },
		{ Cmd::TradeEventLocations, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Trade Event Locations"), {10001, 10005} },
		// at (0,0) into variable 1
		{ Cmd::StoreTerrainID, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Store Terrain ID"), {0, 0, 0, 1} },
		{ Cmd::StoreEventID, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Store Event ID"), {0, 0, 0, 1} },
		// this event, frequency 8, no repeat, no skip, empty route
		{ Cmd::MoveEvent, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Move Event"), {10005, 8, 0, 0} },
		{ Cmd::ProceedWithMovement, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Proceed With Movement"), {} },
		{ Cmd::HaltAllMovement, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Halt All Movement"), {} },
		{ Cmd::PlayerVisibility, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Player Visibility"), {0} },
		{ Cmd::ChangeMapTileset, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Map Tileset"), {1} },
		{ Cmd::ChangePBG, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Change PBG"), {0, 0, 0, 0, 0, 0} },
		{ Cmd::ChangeEncounterSteps, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Change Encounter Steps"), {25} },
		{ Cmd::TileSubstitution, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Tile Substitution"), {0, 0, 0} },
		// add map 1 (0,0)
		{ Cmd::TeleportTargets, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Teleport Targets"), {0, 1, 0, 0, 0, 0} },
		// map 1 (0,0)
		{ Cmd::EscapeTarget, Cat::MapMovement, Map, QT_TRANSLATE_NOOP("Stringizer", "Escape Target"), {1, 0, 0, 0, 0} },

		// Screen
		// -1 uses the default transition
		{ Cmd::EraseScreen, Cat::Screen, Map, QT_TRANSLATE_NOOP("Stringizer", "Erase Screen"), {-1} },
		{ Cmd::ShowScreen, Cat::Screen, Map, QT_TRANSLATE_NOOP("Stringizer", "Show Screen"), {-1} },
		// normal colors in 1 second, wait
		{ Cmd::TintScreen, Cat::Screen, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Tint Screen"), {100, 100, 100, 100, 10, 1} },
		// white in 0.5 seconds, wait, the last parameter (start/stop) is RPG Maker 2003 only
		{ Cmd::FlashScreen, Cat::Screen, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Flash Screen"), {31, 31, 31, 31, 5, 1, 0}, "", 6 },
		// strength 3, speed 5 for 1 second, wait, the last parameter (start/stop) is RPG Maker 2003 only
		{ Cmd::ShakeScreen, Cat::Screen, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Shake Screen"), {3, 5, 10, 1, 0}, "", 4 },
		// lock
		{ Cmd::PanScreen, Cat::Screen, Map, QT_TRANSLATE_NOOP("Stringizer", "Pan Screen"), {0, 0, 0, 0, 0} },
		{ Cmd::WeatherEffects, Cat::Screen, Map, QT_TRANSLATE_NOOP("Stringizer", "Weather Effects"), {0, 0} },
		// animation 1 on this event
		{ Cmd::ShowBattleAnimation, Cat::Screen, Map, QT_TRANSLATE_NOOP("Stringizer", "Show Battle Animation"), {1, 10005, 0, 0} },
		// white for 0.5 seconds on this event
		{ Cmd::FlashSprite, Cat::Screen, Map, QT_TRANSLATE_NOOP("Stringizer", "Flash Sprite"), {10005, 31, 31, 31, 31, 5, 0} },

		// Pictures
		// picture 1 at the center of the screen, the last two parameters are RPG Maker 2003 only
		{ Cmd::ShowPicture, Cat::Pictures, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Show Picture"), {1, 0, 160, 120, 0, 100, 0, 1, 100, 100, 100, 100, 0, 0, 0, 0}, "", 14 },
		// to the center of the screen in 1 second, wait, the last parameter is RPG Maker 2003 only
		{ Cmd::MovePicture, Cat::Pictures, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Move Picture"), {1, 0, 160, 120, 0, 100, 0, 0, 100, 100, 100, 100, 0, 0, 10, 1, 0}, "", 16 },
		{ Cmd::ErasePicture, Cat::Pictures, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Erase Picture"), {1} },

		// Audio, an empty name is (OFF)
		{ Cmd::PlayBGM, Cat::Audio, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Play BGM"), {0, 100, 100, 50} },
		// in 1 second
		{ Cmd::FadeOutBGM, Cat::Audio, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Fade Out BGM"), {1000} },
		{ Cmd::MemorizeBGM, Cat::Audio, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Memorize BGM"), {} },
		{ Cmd::PlayMemorizedBGM, Cat::Audio, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Play Memorized BGM"), {} },
		{ Cmd::PlaySound, Cat::Audio, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Play Sound"), {100, 100, 50} },
		// at (0,0) with size 320x240
		{ Cmd::PlayMovie, Cat::Audio, Map, QT_TRANSLATE_NOOP("Stringizer", "Play Movie"), {0, 0, 0, 320, 240} },

		// Battle
		// troop 1, map background, escape ends the event, defeat is game over, no handlers.
		// The last three parameters (condition, formation, terrain) are RPG Maker 2003 only.
		{ Cmd::EnemyEncounter, Cat::Battle, Map, QT_TRANSLATE_NOOP("Stringizer", "Enemy Encounter"), {0, 1, 0, 1, 0, 0, 0, 1, 1}, "", 6 },
		// monster 1 loses 1 HP, can't die
		{ Cmd::ChangeMonsterHP, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Change Monster HP"), {0, 1, 0, 1, 0} },
		{ Cmd::ChangeMonsterMP, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Change Monster MP"), {0, 1, 0, 1} },
		{ Cmd::ChangeMonsterCondition, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Change Monster Condition"), {0, 0, 1} },
		{ Cmd::ShowHiddenMonster, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Show Hidden Monster"), {0} },
		{ Cmd::ChangeBattleBG, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Change Battle BG"), {} },
		// animation 1 on all monsters, the last parameter (target allies) is RPG Maker 2003 only
		{ Cmd::ShowBattleAnimation_B, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Show Battle Animation"), {1, -1, 0, 0}, "", 3 },
		// the party flees
		{ Cmd::ForceFlee, Cat::Battle, Battle2k3, QT_TRANSLATE_NOOP("Stringizer", "Force Flee"), {0, 0, 0} },
		// actor 1 repeats battle command 1 two times
		{ Cmd::EnableCombo, Cat::Battle, Battle2k3, QT_TRANSLATE_NOOP("Stringizer", "Enable Combo"), {1, 1, 2} },
		{ Cmd::TerminateBattle, Cat::Battle, Battle, QT_TRANSLATE_NOOP("Stringizer", "Terminate Battle"), {} },

		// Advanced
		// into variable 1, wait, all keys, the parameters after the fifth are RPG Maker 2003 only
		{ Cmd::KeyInputProc, Cat::Advanced, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Key Input Proc"), {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1}, "", 5 },
		{ Cmd::Comment, Cat::Advanced, Everywhere, QT_TRANSLATE_NOOP("Stringizer", "Comment"), {} },
	};
	return templates;
}

bool isAvailable(const Template& tmpl, bool rpg2k3, bool battle) {
	int context = battle ? (rpg2k3 ? Battle2k3 : Battle2k) : (rpg2k3 ? Map2k3 : Map2k);
	return (tmpl.contexts & context) != 0;
}

lcf::rpg::EventCommand makeCommand(const Template& tmpl, bool rpg2k3, int indent) {
	lcf::rpg::EventCommand cmd;
	cmd.code = static_cast<int32_t>(tmpl.code);
	cmd.indent = indent;
	cmd.string = lcf::DBString(tmpl.string);

	size_t count = tmpl.parameters.size();
	if (!rpg2k3 && tmpl.rpg2k_parameters >= 0) {
		count = std::min(count, static_cast<size_t>(tmpl.rpg2k_parameters));
	}
	cmd.parameters = lcf::DBArray<int32_t>(tmpl.parameters.begin(), tmpl.parameters.begin() + count);
	return cmd;
}

namespace {
lcf::rpg::EventCommand line(Cmd code, int indent, std::string_view string = {}, std::vector<int32_t> parameters = {}) {
	lcf::rpg::EventCommand cmd;
	cmd.code = static_cast<int32_t>(code);
	cmd.indent = indent;
	cmd.string = lcf::DBString(string);
	cmd.parameters = lcf::DBArray<int32_t>(parameters.begin(), parameters.end());
	return cmd;
}

int32_t parameter(const lcf::rpg::EventCommand& cmd, size_t index) {
	return index < cmd.parameters.size() ? cmd.parameters[index] : 0;
}

// The branches of a block and its closing line, at the indent of the head
struct BlockCodes {
	std::vector<Cmd> branches;
	Cmd end;
};

bool blockCodes(int code, BlockCodes& block) {
	switch (static_cast<Cmd>(code)) {
		case Cmd::ConditionalBranch: block = {{Cmd::ElseBranch}, Cmd::EndBranch}; return true;
		case Cmd::ConditionalBranch_B: block = {{Cmd::ElseBranch_B}, Cmd::EndBranch_B}; return true;
		case Cmd::Loop: block = {{}, Cmd::EndLoop}; return true;
		case Cmd::ShowChoice: block = {{Cmd::ShowChoiceOption}, Cmd::ShowChoiceEnd}; return true;
		case Cmd::EnemyEncounter: block = {{Cmd::VictoryHandler, Cmd::EscapeHandler, Cmd::DefeatHandler}, Cmd::EndBattle}; return true;
		case Cmd::OpenShop: block = {{Cmd::Transaction, Cmd::NoTransaction}, Cmd::EndShop}; return true;
		case Cmd::ShowInn: block = {{Cmd::Stay, Cmd::NoStay}, Cmd::EndInn}; return true;
		default: return false;
	}
}
}

std::vector<lcf::rpg::EventCommand> makeCompanions(const lcf::rpg::EventCommand& head) {
	std::vector<lcf::rpg::EventCommand> lines;
	const int indent = head.indent;

	// A branch: its line and the empty line ending its content
	auto branch = [&](Cmd code, std::string_view string = {}, std::vector<int32_t> parameters = {}) {
		lines.push_back(line(code, indent, string, std::move(parameters)));
		lines.push_back(line(Cmd::END, indent + 1));
	};

	switch (static_cast<Cmd>(head.code)) {
		case Cmd::ConditionalBranch:
		case Cmd::ConditionalBranch_B: {
			const bool battle = static_cast<Cmd>(head.code) == Cmd::ConditionalBranch_B;
			lines.push_back(line(Cmd::END, indent + 1));
			if (parameter(head, 5)) {
				branch(battle ? Cmd::ElseBranch_B : Cmd::ElseBranch);
			}
			lines.push_back(line(battle ? Cmd::EndBranch_B : Cmd::EndBranch, indent));
			break;
		}
		case Cmd::Loop:
			lines.push_back(line(Cmd::END, indent + 1));
			lines.push_back(line(Cmd::EndLoop, indent));
			break;
		case Cmd::ShowChoice: {
			// The choices are separated by "/", at most 4
			std::string_view choices = head.string;
			for (int32_t i = 0; i < 4; ++i) {
				size_t slash = choices.find('/');
				branch(Cmd::ShowChoiceOption, choices.substr(0, slash), {i});
				if (slash == std::string_view::npos) {
					break;
				}
				choices.remove_prefix(slash + 1);
			}
			// Cancel case: 0 disallowed, 1-4 a choice, 5 its own branch
			if (parameter(head, 0) == 5) {
				branch(Cmd::ShowChoiceOption, {}, {4});
			}
			lines.push_back(line(Cmd::ShowChoiceEnd, indent));
			break;
		}
		case Cmd::EnemyEncounter: {
			// Escape: 0 disallowed, 1 ends the event, 2 handler. Defeat: 0 game over, 1 handler.
			const bool escape = parameter(head, 3) == 2;
			const bool defeat = parameter(head, 4) == 1;
			if (escape || defeat) {
				branch(Cmd::VictoryHandler);
				if (escape) {
					branch(Cmd::EscapeHandler);
				}
				if (defeat) {
					branch(Cmd::DefeatHandler);
				}
				lines.push_back(line(Cmd::EndBattle, indent));
			}
			break;
		}
		case Cmd::OpenShop:
			if (parameter(head, 2)) {
				branch(Cmd::Transaction);
				branch(Cmd::NoTransaction);
				lines.push_back(line(Cmd::EndShop, indent));
			}
			break;
		case Cmd::ShowInn:
			if (parameter(head, 2)) {
				branch(Cmd::Stay);
				branch(Cmd::NoStay);
				lines.push_back(line(Cmd::EndInn, indent));
			}
			break;
		default:
			break;
	}
	return lines;
}

size_t blockEnd(const std::vector<lcf::rpg::EventCommand>& commands, size_t index) {
	const auto& head = commands[index];
	size_t end = index + 1;

	// Additional lines of messages and comments
	Cmd continuation = Cmd::END;
	switch (static_cast<Cmd>(head.code)) {
		case Cmd::ShowMessage: continuation = Cmd::ShowMessage_2; break;
		case Cmd::Comment: continuation = Cmd::Comment_2; break;
		default: break;
	}
	if (continuation != Cmd::END) {
		while (end < commands.size() && static_cast<Cmd>(commands[end].code) == continuation) {
			++end;
		}
		return end;
	}

	BlockCodes block;
	if (!blockCodes(head.code, block)) {
		return end;
	}

	// The content of the branches is indented deeper, the branches and the
	// closing line are at the indent of the head
	for (size_t i = index + 1; i < commands.size(); ++i) {
		const auto& cmd = commands[i];
		if (cmd.indent > head.indent) {
			continue;
		}
		if (cmd.indent == head.indent) {
			if (static_cast<Cmd>(cmd.code) == block.end) {
				return i + 1;
			}
			if (std::find(block.branches.begin(), block.branches.end(), static_cast<Cmd>(cmd.code)) != block.branches.end()) {
				continue;
			}
		}
		// A block without branches (encounter without handlers, ...) or a broken one
		break;
	}
	return end;
}

bool isCompanion(int code) {
	switch (static_cast<Cmd>(code)) {
		case Cmd::ElseBranch:
		case Cmd::EndBranch:
		case Cmd::ElseBranch_B:
		case Cmd::EndBranch_B:
		case Cmd::EndLoop:
		case Cmd::ShowChoiceOption:
		case Cmd::ShowChoiceEnd:
		case Cmd::VictoryHandler:
		case Cmd::EscapeHandler:
		case Cmd::DefeatHandler:
		case Cmd::EndBattle:
		case Cmd::Transaction:
		case Cmd::NoTransaction:
		case Cmd::EndShop:
		case Cmd::Stay:
		case Cmd::NoStay:
		case Cmd::EndInn:
			return true;
		default:
			return false;
	}
}

}
