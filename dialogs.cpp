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

#include <qpixmap.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <knuminput.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kdialogbase.h>

#include "settings.h"

#include "bitmaps/smile"
#include "bitmaps/smile_happy"
#include "bitmaps/smile_ohno"
#include "bitmaps/smile_stress"
#include "bitmaps/smile_sleep"


//-----------------------------------------------------------------------------
const char **Smiley::XPM_NAMES[NbMoods] = {
    smile_xpm, smile_stress_xpm, smile_happy_xpm, smile_ohno_xpm,
    smile_sleep_xpm
};

void Smiley::setMood(Mood mood)
{
    QPixmap p(XPM_NAMES[mood]);
    setPixmap(p);
}

//-----------------------------------------------------------------------------
DigitalClock::DigitalClock(QWidget *parent)
: KGameLCDClock(parent, "digital_clock")
{
    setFrameStyle(Panel | Sunken);
    setDefaultBackgroundColor(black);
    setDefaultColor(white);
}

KExtHighscore::Score DigitalClock::score() const
{
    KExtHighscore::Score score(KExtHighscore::Won);
    score.setScore(3600 - seconds());
    score.setData("nb_actions", _nbActions);
    return score;
}

void DigitalClock::timeoutClock()
{
    KGameLCDClock::timeoutClock();

    if ( _cheating || _customGame ) setColor(white);
    else if ( _first<score() ) setColor(red);
    else if ( _last<score() ) setColor(blue);
    else setColor(white);
}

void DigitalClock::start()
{
    KGameLCDClock::start();
    if ( !_cheating && !_customGame ) setColor(red);
}

void DigitalClock::reset(bool customGame)
{
    _nbActions = 0;
    _customGame = customGame;
    if ( !customGame ) {
        _first = KExtHighscore::firstScore();
        _last = KExtHighscore::lastScore();
    }
    _cheating = false;
    KGameLCDClock::reset();
    resetColor();
}

void DigitalClock::setCheating()
{
    _cheating = true;
    setColor(white);
}

//-----------------------------------------------------------------------------

const uint CustomConfig::maxWidth = 50;
const uint CustomConfig::minWidth = 5;
const uint CustomConfig::maxHeight = 50;
const uint CustomConfig::minHeight = 5;

CustomConfig::CustomConfig()
    : QWidget(0, "custom_config_widget"), _block(false)
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    _width = new KIntNumInput(this, "kcfg_CustomWidth");
    _width->setLabel(i18n("Width:"));
    _width->setRange(minWidth, maxWidth);
    connect(_width, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_width);

    _height = new KIntNumInput(this, "kcfg_CustomHeight");
    _height->setLabel(i18n("Height:"));
    _height->setRange(minWidth, maxWidth);
    connect(_height, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_height);

    _mines = new KIntNumInput(this, "kcfg_CustomMines");
    _mines->setLabel(i18n("No. of mines:"));
    _mines->setRange(1, Level::maxNbMines(maxWidth, maxHeight));
    connect(_mines, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_mines);

    top->addSpacing(2 * KDialog::spacingHint());

    // combo to choose level
    QHBoxLayout *hbox = new QHBoxLayout(top);
    QLabel *label = new QLabel(i18n("Choose level:"), this);
    hbox->addWidget(label);
    _gameType = new KComboBox(false, this);
    connect(_gameType, SIGNAL(activated(int)), SLOT(typeChosen(int)));
    for (uint i=0; i<=Level::NB_TYPES; i++)
        _gameType->insertItem(i18n(Level::LABELS[i]));
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
    _mines->setLabel(i18n("Mines (%1%):")
                     .arg( (100*l.nbMines()) / (l.width() * l.height()) ));
    _gameType->setCurrentItem(l.type());
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
    : QWidget(0, "game_config_widget"), _magicDialogEnabled(false)
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QCheckBox *cb = new QCheckBox(i18n("Enable ? mark"), this, "kcfg_UncertainMark");
    top->addWidget(cb);

    cb = new QCheckBox(i18n("Enable keyboard"), this, "kcfg_KeyboardGame");
    top->addWidget(cb);

    cb = new QCheckBox(i18n("Pause if window loses focus"), this, "kcfg_PauseFocus");
    top->addWidget(cb);

    cb = new QCheckBox(i18n("\"Magic\" reveal"), this, "kcfg_MagicReveal");
    QWhatsThis::add(cb, i18n("Set flags and reveal squares where they are trivial."));
    connect(cb, SIGNAL(toggled(bool)), SLOT(magicModified(bool)));
    top->addWidget(cb);

    top->addSpacing(2 * KDialog::spacingHint());

    QHBoxLayout *hbox = new QHBoxLayout(top);
    QVGroupBox *gb = new QVGroupBox(i18n("Mouse Bindings"), this);
    hbox->addWidget(gb);
    QGrid *grid = new QGrid(2, gb);
    grid->setSpacing(KDialog::spacingHint());
    for (uint i=0; i< Settings::EnumButton::COUNT; i++) {
        (void)new QLabel(i18n(MOUSE_BUTTON_LABELS[i]), grid);
        QComboBox *cb = new QComboBox(false, grid, MOUSE_CONFIG_NAMES[i]);
        for (uint k=0; k< (Settings::EnumMouseAction::COUNT-1); k++)
            cb->insertItem(i18n(MOUSE_ACTION_LABELS[k]));
        cb->setCurrentItem(i);
    }
    hbox->addStretch(1);

    top->addStretch(1);
}

void GameConfig::magicModified(bool on)
{
    if ( !_magicDialogEnabled || !on ) return;
    KMessageBox::information(this, i18n("When the \"magic\" reveal is on, you lose the ability to enter the highscores."), QString::null, "magic_reveal_warning");
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
    : QWidget(0, "appearance_config_widget")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QHBoxLayout *hbox = new QHBoxLayout(top);
    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(KDialog::spacingHint());
    hbox->addWidget(grid);
    for (uint i=0; i<Settings::EnumType::COUNT; i++) {
        (void)new QLabel(i18n(COLOR_LABELS[i]), grid);
        KColorButton *cb = new KColorButton(grid, COLOR_CONFIG_NAMES[i]);
        cb->setFixedWidth(100);
    }
    for (uint i=0; i<NB_N_COLORS; i++) {
        (void)new QLabel(i18n("%n mine color:", "%n mines color:", i+1), grid);
        KColorButton *cb = new KColorButton(grid, N_COLOR_CONFIG_NAMES[i]);
        cb->setFixedWidth(100);
    }
    hbox->addStretch(1);

    top->addStretch(1);
}

