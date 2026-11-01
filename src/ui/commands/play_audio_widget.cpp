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

#include "play_audio_widget.h"
#include "ui_play_audio_widget.h"
#include "defines.h"
#include "param_bindings.h"
#include "stringizer.h"
#include "common/dbstring.h"
#include "ui/picker/picker_audio_widget.h"
#include "ui/picker/picker_dialog.h"
#include <lcf/rpg/music.h>
#include <lcf/rpg/sound.h>

PlayAudioWidget::PlayAudioWidget(ProjectData& project, QWidget *parent, bool music, bool system) :
	EventCommandBaseWidget(project, parent),
	ui(new Ui::PlayAudioWidget),
	m_music(music),
	m_system(system) {

	ui->setupUi(this);

	if (system) {
		int count = music ? Stringizer::systemBgmCount : Stringizer::systemSfxCount;
		for (int id = 0; id < count; ++id) {
			ui->comboContext->addItem(music ? Stringizer::systemBgmName(id) : Stringizer::systemSfxName(id), id);
		}
	} else {
		ui->groupContext->hide();
	}

	if (!music) {
		ui->groupFile->setTitle(tr("Sound effect"));
		ui->labelFadeIn->hide();
		ui->spinFadeIn->hide();
	}

	connect(ui->pushPick, &QPushButton::clicked, this, &PlayAudioWidget::pickFile);
}

PlayAudioWidget::~PlayAudioWidget() {
	delete ui;
}

void PlayAudioWidget::setData(lcf::rpg::EventCommand* cmd) {
	// Volume, tempo and balance follow the fade-in time of Play BGM.
	// Change System BGM and SFX have the music or sound effect changed before them.
	int context = m_system ? 1 : 0;
	int first = context + (m_music ? 1 : 0);
	EventCommandBaseWidget::setData(cmd);
	resizeCommandList(first + 2);

	ui->lineFile->setText(ToQString(m_cmd->string));
	if (m_system) {
		ParamBindings::bindComboData(ui->comboContext, m_cmd, 0);
	}
	if (m_music) {
		bindSpin(ui->spinFadeIn, context, 1000);
	}
	bindSpin(ui->spinVolume, first);
	bindSpin(ui->spinTempo, first + 1);
	bindSpin(ui->spinBalance, first + 2);
}

void PlayAudioWidget::bindSpin(QSpinBox* spin, int index, int factor) {
	// Connected after setting the value: values out of range are kept until edited
	spin->setValue(m_cmd->parameters[index] / factor);
	connect(spin, qOverload<int>(&QSpinBox::valueChanged), this, [this, index, factor](int value) {
		m_cmd->parameters[index] = value * factor;
	});
}

void PlayAudioWidget::pickFile() {
	QString name;
	PickerAudioWidget* widget;
	if (m_music) {
		lcf::rpg::Music music;
		music.fadein = ui->spinFadeIn->value() * 1000;
		music.volume = ui->spinVolume->value();
		music.tempo = ui->spinTempo->value();
		music.balance = ui->spinBalance->value();
		widget = new PickerAudioWidget(music, this);
	} else {
		lcf::rpg::Sound sound;
		sound.volume = ui->spinVolume->value();
		sound.tempo = ui->spinTempo->value();
		sound.balance = ui->spinBalance->value();
		widget = new PickerAudioWidget(sound, this);
	}

	PickerDialog dialog(m_project, m_music ? FileFinder::FileType::Music : FileFinder::FileType::Sound, widget, this);
	connect(&dialog, &PickerDialog::fileSelected, this, [&](const QString& baseName) {
		name = baseName;
	});
	dialog.setDirectoryAndFile(m_music ? MUSIC : SOUND, ui->lineFile->text());
	if (dialog.exec() != QDialog::Accepted || name.isEmpty()) {
		return;
	}

	ui->lineFile->setText(name);
	m_cmd->string = ToDBString(name);
	// The spin boxes write the parameters when the values differ
	if (m_music) {
		ui->spinFadeIn->setValue(widget->fadeInTime() / 1000);
	}
	ui->spinVolume->setValue(widget->volume());
	ui->spinTempo->setValue(widget->tempo());
	ui->spinBalance->setValue(widget->balance());
}
