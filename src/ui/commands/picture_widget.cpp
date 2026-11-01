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

#include "picture_widget.h"
#include "ui_picture_widget.h"
#include "ui/picker/picker_dialog.h"
#include "ui/picker/picker_backdrop_widget.h"
#include "common/dbstring.h"
#include "model/project_data.h"
#include "defines.h"
#include <QCoreApplication>
#include <QSignalBlocker>
#include <algorithm>
#include <vector>

namespace {
/*
 * Parameters, as read by EasyRPG Player (Game_Interpreter):
 *
 * Both commands:
 *  0 picture ID
 *  1 position mode (0: constant, 1: variables), the Maniac Patch stores the origin above the low byte
 *  2 X, 3 Y
 *  5 magnification
 *  6 transparency (of the top half in RPG Maker 2003)
 *  8-11 red, green, blue, saturation
 *  12 effect (0: none, 1: rotation, 2: ripple, 3: fixed angle of the Maniac Patch)
 *  13 effect speed
 *
 * Show Picture:
 *  4 scroll with the map
 *  7 use the transparent color
 *  14 bottom transparency (RPG Maker 2003), Maniac Patch flags above the low byte
 *  15, 16 unused (Maniac Patch)
 *  RPG Maker 2003 v1.12 (30 parameters):
 *  17 picture ID mode, 18, 19 file name substitution, 20 magnification mode, 21 transparency mode,
 *  22 spritesheet columns (0: no spritesheet), 23 rows, 24 frame mode (0: frame, 1: frame from
 *  variable, 2: animate), 25 frame, variable or speed, 26 play once, 27 map layer, 28 battle layer, 29 flags
 *
 * Move Picture:
 *  4, 7 unused (Maniac Patch)
 *  14 duration in tenths of a second
 *  15 wait until done (Maniac Patch: more flags in the other bits)
 *  16 bottom transparency (RPG Maker 2003 before v1.12)
 *  RPG Maker 2003 v1.12 (22 parameters): 17 picture ID mode, 18, 19 unused, 20 magnification mode,
 *  21 transparency mode
 */
constexpr int show_v112_size = 30;
constexpr int move_v112_size = 22;

// Values of a new command, for the parameters that are missing
const std::vector<int32_t> show_defaults = { 1, 0, 160, 120, 0, 100, 0, 1, 100, 100, 100, 100, 0, 0, 0, 0 };
const std::vector<int32_t> move_defaults = { 1, 0, 160, 120, 0, 100, 0, 0, 100, 100, 100, 100, 0, 0, 10, 1, 0 };

// Flags of Show Picture in RPG Maker 2003 v1.12
constexpr int flag_erase_map = 1;
constexpr int flag_erase_battle = 2;
constexpr int flag_tint = 16;
constexpr int flag_flash = 32;
constexpr int flag_shake = 64;

void fillCombo(QComboBox* combo, std::initializer_list<std::pair<const char*, int>> items) {
	for (auto& [text, value] : items) {
		combo->addItem(QCoreApplication::translate("PictureWidget", text), value);
	}
}
}

PictureWidget::PictureWidget(ProjectData& project, QWidget* parent, Kind kind) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::PictureWidget),
	m_kind(kind) {

	ui->setupUi(this);

	ui->comboVarX->makeModel(project);
	ui->comboVarY->makeModel(project);

	fillCombo(ui->comboOrigin, {
		{ QT_TR_NOOP("Center"), 0 }, { QT_TR_NOOP("Top left"), 1 }, { QT_TR_NOOP("Bottom left"), 2 },
		{ QT_TR_NOOP("Top right"), 3 }, { QT_TR_NOOP("Bottom right"), 4 }, { QT_TR_NOOP("Top"), 5 },
		{ QT_TR_NOOP("Bottom"), 6 }, { QT_TR_NOOP("Left"), 7 }, { QT_TR_NOOP("Right"), 8 } });
	fillCombo(ui->comboEffect, {
		{ QT_TR_NOOP("None"), 0 }, { QT_TR_NOOP("Rotation"), 1 }, { QT_TR_NOOP("Ripple"), 2 } });
	fillCombo(ui->comboFrameMode, {
		{ QT_TR_NOOP("Frame"), 0 }, { QT_TR_NOOP("Frame from variable"), 1 }, { QT_TR_NOOP("Animate, speed"), 2 } });
	fillCombo(ui->comboMapLayer, {
		{ QT_TR_NOOP("None"), 0 }, { QT_TR_NOOP("Below the parallax"), 1 }, { QT_TR_NOOP("Below the lower tiles"), 2 },
		{ QT_TR_NOOP("Below events below the hero"), 3 }, { QT_TR_NOOP("Below events at the hero's level"), 4 },
		{ QT_TR_NOOP("Below the upper tiles"), 5 }, { QT_TR_NOOP("Below events above the hero"), 6 },
		{ QT_TR_NOOP("Below the weather"), 7 }, { QT_TR_NOOP("Below battle animations"), 8 },
		{ QT_TR_NOOP("Below windows"), 9 }, { QT_TR_NOOP("Below the timer"), 10 } });
	fillCombo(ui->comboBattleLayer, {
		{ QT_TR_NOOP("None"), 0 }, { QT_TR_NOOP("Below the background"), 1 },
		{ QT_TR_NOOP("Below battlers and animations"), 2 }, { QT_TR_NOOP("Below the weather"), 3 },
		{ QT_TR_NOOP("Below windows and status"), 4 }, { QT_TR_NOOP("Below the timer"), 5 } });

	bool show = kind == Kind::Show;
	ui->lineFile->setVisible(show);
	ui->labelFile->setVisible(show);
	ui->pushFile->setVisible(show);
	ui->checkScroll->setVisible(show);
	ui->checkTransparentColor->setVisible(show);
	ui->groupDuration->setVisible(!show);
	ui->groupPosition->setTitle(show ? tr("Position") : tr("Destination"));
}

PictureWidget::~PictureWidget() {
	delete ui;
}

int PictureWidget::param(int index) const {
	return index < static_cast<int>(m_cmd->parameters.size()) ? m_cmd->parameters[index] : 0;
}

void PictureWidget::setParam(int index, int value) {
	resizeCommandList(index);
	m_cmd->parameters[index] = value;
}

void PictureWidget::setParamBits(int index, int mask, int value) {
	setParam(index, (param(index) & ~mask) | (value & mask));
}

void PictureWidget::bindSpin(QSpinBox* spin, int index) {
	{
		QSignalBlocker blocker(spin);
		spin->setValue(param(index));
	}
	connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, [this, index](int value) {
		setParam(index, value);
	});
}

void PictureWidget::bindCheck(QCheckBox* check, int index) {
	{
		QSignalBlocker blocker(check);
		check->setChecked(param(index) > 0);
	}
	connect(check, &QCheckBox::toggled, this, [this, index](bool checked) {
		setParam(index, checked ? 1 : 0);
		updateEnabled();
	});
}

void PictureWidget::bindFlag(QCheckBox* check, int index, int bit) {
	{
		QSignalBlocker blocker(check);
		check->setChecked((param(index) & bit) != 0);
	}
	connect(check, &QCheckBox::toggled, this, [this, index, bit](bool checked) {
		setParamBits(index, bit, checked ? bit : 0);
	});
}

void PictureWidget::bindCombo(QComboBox* combo, int index, std::function<void(int)> changed) {
	{
		QSignalBlocker blocker(combo);
		int value = param(index);
		int i = combo->findData(value);
		if (i == -1) {
			// Value of another engine version or patch, keep it selectable
			combo->addItem(tr("Other (%1)").arg(value), value);
			i = combo->count() - 1;
		}
		combo->setCurrentIndex(i);
	}
	connect(combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, combo, index, changed](int i) {
		int value = combo->itemData(i).toInt();
		setParam(index, value);
		if (changed) {
			changed(value);
		}
		updateEnabled();
	});
}

void PictureWidget::setData(lcf::rpg::EventCommand* cmd) {
	const bool show = m_kind == Kind::Show;
	const bool rpg2k3 = m_project.database().system.ldb_id == 2003;

	// Complete a new or truncated command, longer ones are kept as they are
	const auto& defaults = show ? show_defaults : move_defaults;
	size_t min_size = rpg2k3 ? defaults.size() : (show ? 14 : 16);
	if (cmd->parameters.size() < min_size) {
		std::vector<int32_t> params(cmd->parameters.begin(), cmd->parameters.end());
		for (size_t i = params.size(); i < min_size; ++i) {
			params.push_back(defaults[i]);
		}
		cmd->parameters = lcf::DBArray<int32_t>(params.begin(), params.end());
	}

	EventCommandBaseWidget::setData(cmd);

	m_v112 = static_cast<int>(cmd->parameters.size()) >= (show ? show_v112_size : move_v112_size);

	// Picture
	bindSpin(ui->spinNumber, 0);
	if (m_v112 && (param(17) & 0xF) != 0) {
		ui->labelNumber->setText(tr("Number (variable):"));
	}
	if (show) {
		ui->lineFile->setText(ToQString(cmd->string));
		connect(ui->pushFile, &QToolButton::clicked, this, &PictureWidget::pickFile);
	}

	// Position: the coordinates or the variable IDs share their parameters
	bool by_variable = (param(1) & 0xFF) != 0;
	ui->radioVariable->setChecked(by_variable);
	ui->radioConstant->setChecked(!by_variable);
	if (by_variable) {
		ui->comboVarX->setCurrentIndex(std::max(param(2) - 1, 0));
		ui->comboVarY->setCurrentIndex(std::max(param(3) - 1, 0));
	} else {
		ui->spinX->setValue(param(2));
		ui->spinY->setValue(param(3));
	}
	connect(ui->radioConstant, &QRadioButton::toggled, this, [this] { writePosition(); updateEnabled(); });
	connect(ui->spinX, qOverload<int>(&QSpinBox::valueChanged), this, [this] { writePosition(); });
	connect(ui->spinY, qOverload<int>(&QSpinBox::valueChanged), this, [this] { writePosition(); });
	connect(ui->comboVarX->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writePosition(); });
	connect(ui->comboVarY->comboBox(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { writePosition(); });

	// The origin is an extension of the Maniac Patch, which needs RPG Maker 2003
	int origin = static_cast<uint32_t>(param(1)) >> 8;
	ui->labelOrigin->setVisible(rpg2k3 || origin != 0);
	ui->comboOrigin->setVisible(rpg2k3 || origin != 0);
	{
		QSignalBlocker blocker(ui->comboOrigin);
		int i = ui->comboOrigin->findData(origin);
		if (i == -1) {
			ui->comboOrigin->addItem(tr("Other (%1)").arg(origin), origin);
			i = ui->comboOrigin->count() - 1;
		}
		ui->comboOrigin->setCurrentIndex(i);
	}
	connect(ui->comboOrigin, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
		int value = ui->comboOrigin->itemData(i).toInt();
		setParam(1, (param(1) & 0xFF) | (value << 8));
	});

	if (show) {
		bindCheck(ui->checkScroll, 4);
		bindCheck(ui->checkTransparentColor, 7);
	}

	// Magnification and transparency
	bindSpin(ui->spinMagnify, 5);
	bindSpin(ui->spinTop, 6);

	// RPG Maker 2003 has a transparency for the bottom half. The editor of v1.12 has a
	// single transparency, Show Picture still has both and Move Picture uses the top one.
	int bottom_index = show ? 14 : 16;
	bool linked = show && m_v112;
	bool has_bottom = rpg2k3 && !m_v112;
	ui->labelBottom->setVisible(has_bottom);
	ui->spinBottom->setVisible(has_bottom);
	if (has_bottom) {
		ui->labelTop->setText(tr("Top transparency:"));
		{
			QSignalBlocker blocker(ui->spinBottom);
			ui->spinBottom->setValue(param(bottom_index) & 0xFF);
		}
		connect(ui->spinBottom, qOverload<int>(&QSpinBox::valueChanged), this, [this, bottom_index](int value) {
			setParamBits(bottom_index, 0xFF, value);
		});
	}
	if (linked) {
		connect(ui->spinTop, qOverload<int>(&QSpinBox::valueChanged), this, [this, bottom_index](int value) {
			setParamBits(bottom_index, 0xFF, value);
		});
	}

	// Color
	bindSpin(ui->spinRed, 8);
	bindSpin(ui->spinGreen, 9);
	bindSpin(ui->spinBlue, 10);
	bindSpin(ui->spinSat, 11);

	// Effect
	if (param(12) == 3) {
		ui->comboEffect->addItem(tr("Fixed angle (Maniac Patch)"), 3);
		ui->spinPower->setRange(-99999, 99999);
	}
	// RPG Maker leaves anything in the speed when there is no effect
	bool power_valid = param(13) >= ui->spinPower->minimum() && param(13) <= ui->spinPower->maximum();
	{
		QSignalBlocker blocker(ui->spinPower);
		ui->spinPower->setValue(power_valid ? param(13) : 0);
	}
	connect(ui->spinPower, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
		setParam(13, value);
	});
	bindCombo(ui->comboEffect, 12, [this](int) {
		// Keep the shown speed, not a leftover outside of its range
		if (param(13) != ui->spinPower->value()) {
			setParam(13, ui->spinPower->value());
		}
	});

	if (!show) {
		bindSpin(ui->spinDuration, 14);
		// The Maniac Patch adds more options in the other bits
		{
			QSignalBlocker blocker(ui->checkWait);
			ui->checkWait->setChecked((param(15) & 1) != 0);
		}
		connect(ui->checkWait, &QCheckBox::toggled, this, [this](bool checked) {
			setParamBits(15, 1, checked ? 1 : 0);
		});
	}

	// RPG Maker 2003 v1.12
	ui->groupExtra->setVisible(show && m_v112);
	if (show && m_v112) {
		{
			QSignalBlocker b1(ui->checkSpritesheet), b2(ui->spinCols), b3(ui->spinRows);
			ui->checkSpritesheet->setChecked(param(22) > 0);
			ui->spinCols->setValue(std::max(param(22), 1));
			ui->spinRows->setValue(std::max(param(23), 1));
		}
		connect(ui->checkSpritesheet, &QCheckBox::toggled, this, [this](bool checked) {
			setParam(22, checked ? ui->spinCols->value() : 0);
			setParam(23, checked ? ui->spinRows->value() : 0);
			updateEnabled();
		});
		connect(ui->spinCols, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) { setParam(22, value); });
		connect(ui->spinRows, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) { setParam(23, value); });
		bindCombo(ui->comboFrameMode, 24);
		bindSpin(ui->spinFrame, 25);
		bindCheck(ui->checkPlayOnce, 26);
		bindCombo(ui->comboMapLayer, 27);
		bindCombo(ui->comboBattleLayer, 28);
		bindFlag(ui->checkEraseMap, 29, flag_erase_map);
		bindFlag(ui->checkEraseBattle, 29, flag_erase_battle);
		bindFlag(ui->checkTint, 29, flag_tint);
		bindFlag(ui->checkFlash, 29, flag_flash);
		bindFlag(ui->checkShake, 29, flag_shake);
	}

	updateEnabled();
}

void PictureWidget::writePosition() {
	if (ui->radioConstant->isChecked()) {
		setParamBits(1, 0xFF, 0);
		setParam(2, ui->spinX->value());
		setParam(3, ui->spinY->value());
	} else {
		if ((param(1) & 0xFF) == 0) {
			setParamBits(1, 0xFF, 1);
		}
		setParam(2, ui->comboVarX->currentIndex() + 1);
		setParam(3, ui->comboVarY->currentIndex() + 1);
	}
}

void PictureWidget::updateEnabled() {
	bool constant = ui->radioConstant->isChecked();
	ui->spinX->setEnabled(constant);
	ui->spinY->setEnabled(constant);
	ui->comboVarX->setEnabled(!constant);
	ui->comboVarY->setEnabled(!constant);

	ui->spinPower->setEnabled(ui->comboEffect->currentData().toInt() != 0);

	bool spritesheet = ui->checkSpritesheet->isChecked();
	ui->spinCols->setEnabled(spritesheet);
	ui->spinRows->setEnabled(spritesheet);
	ui->comboFrameMode->setEnabled(spritesheet);
	ui->spinFrame->setEnabled(spritesheet);
	ui->checkPlayOnce->setEnabled(spritesheet && ui->comboFrameMode->currentData().toInt() == 2);
}

void PictureWidget::pickFile() {
	auto* widget = new PickerBackdropWidget(this);
	PickerDialog dialog(m_project, FileFinder::FileType::Image, widget, this);
	connect(&dialog, &PickerDialog::fileSelected, this, [this](const QString& baseName) {
		m_cmd->string = ToDBString(baseName);
		ui->lineFile->setText(baseName);
	});
	dialog.setDirectoryAndFile(PICTURE, ToQString(m_cmd->string));
	dialog.exec();
}
