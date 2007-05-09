/*
 * Copyright (c) 1996-2003 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "dialogs.h"
#include "dialogs.moc"

#include <QPixmap>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>

#include <klocale.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <knuminput.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdialog.h>
#include <QGroupBox>
#include "settings.h"

#include "kstandarddirs.h"
#include <QPainter>

Smiley::Smiley(QWidget *parent)
        : QPushButton(QString(), parent) {
    theme = new KGameTheme("KGameTheme");
    moodNames.append("smile");
    moodNames.append("smile_stress");
    moodNames.append("smile_happy");
    moodNames.append("smile_ohno");
    moodNames.append("smile_sleep");

    theme->loadDefault();

    setFlat(true);
}

Smiley::~Smiley()
{
    delete theme;
}

void Smiley::readSettings()
{
  if (!theme->load(Settings::theme())) {
    theme->loadDefault();
  }
  svg.load(theme->graphics());
}

void Smiley::setMood(Mood mood)
{
    //Prob need to trigger a resize here? Use older size (fixed) for now
    QImage qiRend(iconSize(),QImage::Format_ARGB32_Premultiplied);
    qiRend.fill(0);

    if (svg.isValid()) {
//qDebug() << "rendering" << moodNames.at(mood);
            QPainter painter(&qiRend);
	    svg.render(&painter, moodNames.at(mood));
    }
    setIcon(QPixmap::fromImage(qiRend));
}

//-----------------------------------------------------------------------------

const uint CustomConfig::maxWidth = 50;
const uint CustomConfig::minWidth = 5;
const uint CustomConfig::maxHeight = 50;
const uint CustomConfig::minHeight = 5;

CustomConfig::CustomConfig()
    : _block(false)
{
    setObjectName( "custom_config_widget" );
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin( KDialog::spacingHint() );

    _width = new KIntNumInput(this);
    _width->setObjectName("kcfg_CustomWidth");
    _width->setLabel(i18n("Width:"));
    _width->setRange(minWidth, maxWidth);
    connect(_width, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_width);

    _height = new KIntNumInput(this);
    _height->setObjectName("kcfg_CustomHeight");
    _height->setLabel(i18n("Height:"));
    _height->setRange(minWidth, maxWidth);
    connect(_height, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_height);

    _mines = new KIntNumInput(this);
    _mines->setObjectName("kcfg_CustomMines");
    _mines->setLabel(i18n("No. of mines:"));
    _mines->setRange(1, Level::maxNbMines(maxWidth, maxHeight));
    connect(_mines, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_mines);

    top->addSpacing(2 * KDialog::spacingHint());

    // combo to choose level
    QHBoxLayout *hbox = new QHBoxLayout;
    top->addLayout( hbox );
    QLabel *label = new QLabel(i18n("Choose level:"), this);
    hbox->addWidget(label);
    _gameType = new KComboBox(false, this);
    connect(_gameType, SIGNAL(activated(int)), SLOT(typeChosen(int)));
    for (uint i=0; i<=Level::NB_TYPES; i++)
        _gameType->addItem(i18n(Level::LABELS[i]));
    hbox->addWidget(_gameType);
    hbox->addWidget(new QWidget(this), 1);

    top->addStretch(1);
}

void CustomConfig::updateNbMines()
{
    if (_block) return;
    _block = true;
    Level l(_width->value(), _height->value(), _mines->value());
    _mines->setRange(1, Level::maxNbMines(l.width(), l.height()));
    _mines->setLabel(i18n("Mines (%1%):",
                       (100*l.nbMines()) / (l.width() * l.height()) ));
    _gameType->setCurrentIndex(l.type());
    _block = false;
}

void CustomConfig::typeChosen(int i)
{
    if (_block) return;
    _block = true;
    Level::Type type = (Level::Type)i;
    if ( type==Level::Custom ) {
        Level level = Settings::customLevel();
        _width->setValue(level.width());
        _height->setValue(level.height());
        _mines->setRange(1, Level::maxNbMines(level.width(), level.height()));
        _mines->setValue(level.nbMines());
    } else {
        Level level(type);
        _width->setValue(level.width());
        _height->setValue(level.height());
        _mines->setRange(1, Level::maxNbMines(level.width(), level.height()));
        _mines->setValue(level.nbMines());
    }
    _block = false;
    updateNbMines();
}

//-----------------------------------------------------------------------------
static const char *MOUSE_BUTTON_LABELS[Settings::EnumButton::COUNT] = {
    I18N_NOOP("Left button:"), I18N_NOOP("Middle button:"),
    I18N_NOOP("Right button:")
};

static const char *MOUSE_CONFIG_NAMES[Settings::EnumButton::COUNT] = {
    "kcfg_leftMouseAction", "kcfg_midMouseAction",
    "kcfg_rightMouseAction"
};

static const char *MOUSE_ACTION_LABELS[Settings::EnumMouseAction::COUNT-1] = {
    I18N_NOOP("Reveal"), I18N_NOOP("Autoreveal"),
    I18N_NOOP("Toggle Flag"), I18N_NOOP("Toggle ? Flag")
};

GameConfig::GameConfig()
    : _magicDialogEnabled(false)
{
    setObjectName( "game_config_widget" );
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin( KDialog::spacingHint() );

    QCheckBox *cb = new QCheckBox(i18n("Enable ? mark"), this);
    cb->setObjectName( "kcfg_UncertainMark" );
    top->addWidget(cb);

    cb = new QCheckBox(i18n("Enable keyboard"), this);
    cb->setObjectName( "kcfg_KeyboardGame" );
    top->addWidget(cb);

    cb = new QCheckBox(i18n("Pause if window loses focus"), this);
    cb->setObjectName( "kcfg_PauseFocus" );
    top->addWidget(cb);

    cb = new QCheckBox(i18n("\"Magic\" reveal"), this);
    cb->setObjectName( "kcfg_MagicReveal" );
    cb->setWhatsThis( i18n("Set flags and reveal squares where they are trivial."));
    connect(cb, SIGNAL(toggled(bool)), SLOT(magicModified(bool)));
    top->addWidget(cb);

    top->addSpacing(2 * KDialog::spacingHint());

    QGroupBox *gb = new QGroupBox(i18n("Mouse Bindings"), this);
    top->addWidget(gb);
    QGridLayout *grid = new QGridLayout(gb);
    grid->setSpacing(KDialog::spacingHint());
    for (uint i=0; i< Settings::EnumButton::COUNT; i++) {
        grid->addWidget( new QLabel(i18n(MOUSE_BUTTON_LABELS[i]), gb), i, 0);
        QComboBox *cb = new QComboBox(gb);
        cb->setObjectName( MOUSE_CONFIG_NAMES[i] );
        for (uint k=0; k< (Settings::EnumMouseAction::COUNT-1); k++)
            cb->addItem(i18n(MOUSE_ACTION_LABELS[k]));
        cb->setCurrentIndex(i);
        grid->addWidget( cb, i, 1 );
    }

    top->addStretch(1);
}

void GameConfig::magicModified(bool on)
{
    if ( !_magicDialogEnabled || !on ) return;
    KMessageBox::information(this, i18n("When the \"magic\" reveal is on, you lose the ability to enter the highscores."), QString(), "magic_reveal_warning");
}

//-----------------------------------------------------------------------------
static const char *COLOR_LABELS[Settings::EnumType::COUNT] = {
    I18N_NOOP("Flag color:"), I18N_NOOP("Explosion color:"),
    I18N_NOOP("Error color:")
};

static const char *COLOR_CONFIG_NAMES[Settings::EnumType::COUNT] = {
    "kcfg_flagColor", "kcfg_explosionColor", "kcfg_errorColor"
};

static const char *N_COLOR_CONFIG_NAMES[KMines::NB_N_COLORS] = {
    "kcfg_MineColor0", "kcfg_MineColor1", "kcfg_MineColor2",
    "kcfg_MineColor3", "kcfg_MineColor4", "kcfg_MineColor5",
    "kcfg_MineColor6", "kcfg_MineColor7"
};

AppearanceConfig::AppearanceConfig()
{
    setObjectName( "appearance_config_widget" );
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin( KDialog::spacingHint() );

    QGridLayout *grid = new QGridLayout;
    top->addLayout( grid );
    grid->setSpacing(KDialog::spacingHint());
    for (uint i=0; i<Settings::EnumType::COUNT; i++) {
        grid->addWidget(new QLabel(i18n(COLOR_LABELS[i]), this), i, 0);
        KColorButton *cb = new KColorButton(this);
        cb->setObjectName( COLOR_CONFIG_NAMES[i]);
        cb->setFixedWidth(100);
        grid->addWidget( cb, i, 1 );
    }
    for (uint i=0; i<NB_N_COLORS; i++) {
        grid->addWidget( new QLabel(i18np("%1 mine color:", "%1 mines color:", i+1), this), Settings::EnumType::COUNT+i, 0);
        KColorButton *cb = new KColorButton(this);
        cb->setObjectName(N_COLOR_CONFIG_NAMES[i]);
        cb->setFixedWidth(100);
        grid->addWidget( cb, Settings::EnumType::COUNT+i, 1 );
    }

    top->addStretch(1);
}

