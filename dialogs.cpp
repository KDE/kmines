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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
class ConfigGroupSaver : public KConfigGroupSaver
{
public:
    ConfigGroupSaver() : KConfigGroupSaver(kapp->config(), "Options") {}
};

const uint CustomConfig::defaultWidth = 10;
const uint CustomConfig::defaultHeight = 10;
const uint CustomConfig::defaultNbMines = 20;
const uint CustomConfig::maxWidth = 50;
const uint CustomConfig::minWidth = 5;
const uint CustomConfig::maxHeight = 50;
const uint CustomConfig::minHeight = 5;

CustomConfig::CustomConfig()
    : QWidget(0, "custom_config_widget"), _block(false)
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    _width = new KIntNumInput(this, "custom width");
    _width->setLabel(i18n("Width:"));
    _width->setRange(minWidth, maxWidth);
    _width->setValue(defaultWidth);
    connect(_width, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_width);

    _height = new KIntNumInput(this, "custom height");
    _height->setLabel(i18n("Height:"));
    _height->setRange(minWidth, maxWidth);
    _height->setValue(defaultHeight);
    connect(_height, SIGNAL(valueChanged(int)), SLOT(updateNbMines()));
    top->addWidget(_height);

    _mines = new KIntNumInput(this, "custom mines");
    _mines->setLabel(i18n("No. of mines:"));
    _mines->setRange(1, Level::maxNbMines(maxWidth, maxHeight));
    _mines->setValue(defaultNbMines);
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
                     .arg(100*l.nbMines()/l.width()/l.height()));
    _gameType->setCurrentItem(l.type());
    _block = false;
}

void CustomConfig::typeChosen(int i)
{
    if (_block) return;
    _block = true;
    Level::Type type = (Level::Type)i;
    if ( type==Level::Custom ) {
        _width->setValue(defaultWidth);
        _height->setValue(defaultHeight);
        _mines->setRange(1, Level::maxNbMines(defaultWidth, defaultHeight));
        _mines->setValue(defaultNbMines);
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

Level CustomConfig::level()
{
    ConfigGroupSaver cg;
    uint w = cg.config()->readUnsignedNumEntry("custom width", defaultWidth);
    w = kMin(kMax(w, minWidth), maxWidth);
    uint h = cg.config()->readUnsignedNumEntry("custom height", defaultHeight);
    h = kMin(kMax(h, minHeight), maxHeight);
    uint n = cg.config()->readUnsignedNumEntry("custom mines", defaultNbMines);
    n = kMin(kMax(n, (uint)1), Level::maxNbMines(w, h));
    return Level(w, h, n);
}

//-----------------------------------------------------------------------------
const char *GameConfig::MOUSE_BUTTON_LABELS[NB_MOUSE_BUTTONS] = {
    I18N_NOOP("Left button:"), I18N_NOOP("Middle button:"),
    I18N_NOOP("Right button:")
};
const char *GameConfig::MOUSE_CONFIG_NAMES[NB_MOUSE_BUTTONS] = {
    "mouse left", "mouse mid", "mouse right"
};
const char *GameConfig::MOUSE_ACTION_LABELS[NB_MOUSE_ACTIONS] = {
    I18N_NOOP("Reveal"), I18N_NOOP("Autoreveal"),
    I18N_NOOP("Toggle Flag"), I18N_NOOP("Toggle ? Flag")
};

GameConfig::GameConfig()
    : QWidget(0, "game_config_widget"), _magicDialogEnabled(false)
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QCheckBox *cb = new QCheckBox(i18n("Enable ? mark"), this, "uncertain mark");
    cb->setChecked(true);
    top->addWidget(cb);

    cb = new QCheckBox(i18n("Enable keyboard"), this, "keyboard game");
    cb->setChecked(false);
    top->addWidget(cb);

    cb = new QCheckBox(i18n("Pause if windows lose focus"), this, "pause focus");
    cb->setChecked(true);
    top->addWidget(cb);

    cb = new QCheckBox(i18n("\"Magic\" reveal"), this, "magic reveal");
    QWhatsThis::add(cb, i18n("Set flags and reveal cases where they are trivial."));
    cb->setChecked(false);
    connect(cb, SIGNAL(toggled(bool)), SLOT(magicModified(bool)));
    top->addWidget(cb);

    top->addSpacing(2 * KDialog::spacingHint());

    QHBoxLayout *hbox = new QHBoxLayout(top);
    QVGroupBox *gb = new QVGroupBox(i18n("Mouse Bindings"), this);
    hbox->addWidget(gb);
    QGrid *grid = new QGrid(2, gb);
    grid->setSpacing(KDialog::spacingHint());
    for (uint i=0; i<NB_MOUSE_BUTTONS; i++) {
        (void)new QLabel(i18n(MOUSE_BUTTON_LABELS[i]), grid);
        QComboBox *cb = new QComboBox(false, grid, MOUSE_CONFIG_NAMES[i]);
        for (uint k=0; k<NB_MOUSE_ACTIONS; k++)
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

bool GameConfig::isUncertainMarkEnabled()
{
    ConfigGroupSaver cg;
    return cg.config()->readBoolEntry("uncertain mark", true);
}

bool GameConfig::isKeyboardEnabled()
{
    ConfigGroupSaver cg;
    return cg.config()->readBoolEntry("keyboard game", false);
}

bool GameConfig::isPauseFocusEnabled()
{
    ConfigGroupSaver cg;
    return cg.config()->readBoolEntry("pause focus", true);
}

bool GameConfig::isMagicRevealEnabled()
{
    ConfigGroupSaver cg;
    return cg.config()->readBoolEntry("magic reveal", false);
}

KMines::MouseAction GameConfig::mouseAction(MouseButton button)
{
    ConfigGroupSaver cg;
    int a =
        cg.config()->readUnsignedNumEntry(MOUSE_CONFIG_NAMES[button], button);
    return (MouseAction)kMin(a, NB_MOUSE_ACTIONS-1);
}

Level::Type GameConfig::level()
{
    ConfigGroupSaver cg;
    uint l = cg.config()->readUnsignedNumEntry("level", Level::Easy);
    return kMin((Level::Type)l, Level::NB_TYPES);
}

void GameConfig::saveLevel(Level::Type level)
{
    ConfigGroupSaver cg;
    cg.config()->writeEntry("level", level);
}

//-----------------------------------------------------------------------------
const char *AppearanceConfig::COLOR_LABELS[NB_COLORS] = {
    I18N_NOOP("Flag color:"), I18N_NOOP("Explosion color:"),
    I18N_NOOP("Error color:")
};
const char *AppearanceConfig::COLOR_CONFIG_NAMES[NB_COLORS] = {
    "flag color", "explosion color", "error color"
};
const char *AppearanceConfig::COLOR_DEFAULTS[NB_COLORS] = {
    "#FF0000", "#FF0000", "#FF0000"
};
const char *AppearanceConfig::N_COLOR_CONFIG_NAMES[NB_N_COLORS] = {
    "color #0", "color #1", "color #2", "color #3", "color #4", "color #5",
    "color #6", "color #7"
};
const char *AppearanceConfig::N_COLOR_DEFAULTS[NB_N_COLORS] = {
    "#0000FF", "#008800", "#888800", "#880088", "#FF0000", "#880000",
    "#000000", "#000000"
};

AppearanceConfig::AppearanceConfig()
    : QWidget(0, "appearance_config_widget")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    KIntNumInput *in = new KIntNumInput(this, "case size");
    in->setLabel(i18n("Case size:"));
    in->setRange(4, 100);
    in->setValue(20);
    top->addWidget(in);

    top->addSpacing(2 * KDialog::spacingHint());

    QHBoxLayout *hbox = new QHBoxLayout(top);
    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(KDialog::spacingHint());
    hbox->addWidget(grid);
    for (uint i=0; i<NB_COLORS; i++) {
        (void)new QLabel(i18n(COLOR_LABELS[i]), grid);
        KColorButton *cb = new KColorButton(grid, COLOR_CONFIG_NAMES[i]);
        cb->setFixedWidth(100);
        cb->setColor(COLOR_DEFAULTS[i]);
    }
    for (uint i=0; i<NB_N_COLORS; i++) {
        (void)new QLabel(i18n("%n mine color:", "%n mines color:", i+1), grid);
        KColorButton *cb = new KColorButton(grid, N_COLOR_CONFIG_NAMES[i]);
        cb->setFixedWidth(100);
        cb->setColor(N_COLOR_DEFAULTS[i]);
    }
    hbox->addStretch(1);

    top->addStretch(1);
}

uint AppearanceConfig::caseSize()
{
    ConfigGroupSaver cg;
    int n = cg.config()->readUnsignedNumEntry("case size", 20);
    return kMin(kMax(4, n), 100);
}

QColor AppearanceConfig::color(Color c)
{
    ConfigGroupSaver cg;
    QColor def(COLOR_DEFAULTS[c]);
    return cg.config()->readColorEntry(COLOR_CONFIG_NAMES[c], &def);
}

QColor AppearanceConfig::nColor(uint i)
{
    ConfigGroupSaver cg;
    QColor def(N_COLOR_DEFAULTS[i]);
    return cg.config()->readColorEntry(N_COLOR_CONFIG_NAMES[i], &def);
}

bool AppearanceConfig::isMenubarVisible()
{
    ConfigGroupSaver cg;
    return cg.config()->readBoolEntry("menubar visible", true);
}

void AppearanceConfig::saveMenubarVisible(bool visible)
{
    ConfigGroupSaver cg;
    cg.config()->writeEntry("menubar visible", visible);
}
