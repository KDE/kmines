/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
#include <kcombobox.h>

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
    setDefaultColors(white, black);
}

KExtHighscores::Score DigitalClock::score() const
{
    KExtHighscores::Score score(KExtHighscores::Won);
    score.setData("score", 3600 - seconds());
    score.setData("nb_actions", _nbActions);
    return score;
}

void DigitalClock::timeoutClock()
{
    KGameLCDClock::timeoutClock();

    if (_cheating) setColor(white);
    else if ( _first<score() ) setColor(red);
    else if ( _last<score() ) setColor(blue);
    else setColor(white);
}

void DigitalClock::start()
{
    KGameLCDClock::start();
    if ( !_cheating ) setColor(red);
}

void DigitalClock::reset(const KExtHighscores::Score &first,
                         const KExtHighscores::Score &last)
{
    _nbActions = 0;
    _first = first;
    _last = last;
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
CustomConfig::CustomConfig()
    : KConfigWidget(i18n("Custom Game"), "configure")
{
	QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    KIntNumInput *in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    _width = configCollection()->createConfigItem("custom width", in);
    top->addWidget(in);

    in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    _height = configCollection()->createConfigItem("custom height", in);
	top->addWidget(in);

    in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    _mines = configCollection()->createConfigItem("custom mines", in);
	top->addWidget(in);

    top->addSpacing(2 * KDialog::spacingHint());

    // combo to choose level
    QHBoxLayout *hbox = new QHBoxLayout(top);
    QLabel *label = new QLabel(i18n("Choose level"), this);
    hbox->addWidget(label);
    _gameType = new KComboBox(false, this);
    connect(_gameType, SIGNAL(activated(int)), SLOT(typeChosen(int)));
    for (uint i=0; i<=Level::NbLevels; i++)
        _gameType->insertItem(i18n(Level::data((Level::Type)i).i18nLabel));
    hbox->addWidget(_gameType);

    connect(configCollection(), SIGNAL(modified()), SLOT(updateNbMines()));
    QTimer::singleShot(0, this, SLOT(updateNbMines()));
}

void CustomConfig::updateNbMines()
{
    Level level(_width->value().toUInt(), _height->value().toUInt(),
                _mines->value().toUInt());
	static_cast<KRangedConfigItem *>(_mines)->setRange(1, level.maxNbMines());
	uint nb = level.width() * level.height();
	_mines->setText(i18n("Mines (%1%)").arg(100*level.nbMines()/nb));
    _gameType->setCurrentItem(level.type());
}

void CustomConfig::typeChosen(int i)
{
    blockSignals(true);
    Level level((Level::Type)i);
    _width->setValue(level.width());
    _height->setValue(level.height());
    _mines->setValue(level.nbMines());
    blockSignals(false);
    updateNbMines();
}

Level CustomConfig::readLevel()
{
    uint w = KConfigCollection::configItemValue("custom width").toUInt();
    uint h = KConfigCollection::configItemValue("custom height").toUInt();
    uint n = KConfigCollection::configItemValue("custom mines").toUInt();
    return Level(w, h, n);
}

//-----------------------------------------------------------------------------
GameConfig::GameConfig()
    : KConfigWidget(i18n("Game"), "misc")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QCheckBox *cb = new QCheckBox(this);
    configCollection()->createConfigItem("uncertain mark", cb);
    top->addWidget(cb);

    cb = new QCheckBox(this);
    configCollection()->createConfigItem("keyboard game", cb);
    top->addWidget(cb);

    cb = new QCheckBox(this);
    configCollection()->createConfigItem("pause focus", cb);
    top->addWidget(cb);

    cb = new QCheckBox(this);
    _magic = configCollection()->createConfigItem("magic reveal", cb);
    connect(_magic, SIGNAL(modified()), SLOT(magicRevealToggled()));
    top->addWidget(cb);

	top->addSpacing(2 * KDialog::spacingHint());

	QVGroupBox *gb = new QVGroupBox(i18n("Mouse Bindings"), this);
	top->addWidget(gb);
	QGrid *grid = new QGrid(2, gb);
	grid->setSpacing(10);
    for (uint i=0; i<NB_MOUSE_BUTTONS; i++) {
        QLabel *l = new QLabel(grid);
        QComboBox *cb = new QComboBox(false, grid);
        KConfigItem *set =
            configCollection()->createConfigItem(MOUSE_CONFIG_NAMES[i], cb);
        set->setProxyLabel(l);
	}
}

void GameConfig::magicRevealToggled()
{
    if ( _magic->value().toBool() )
        KMessageBox::information(this,
                         i18n("When the \"magic\" reveal is on, "
                              "you lose the ability to enter the highscores."),
                         QString::null, "magic_reveal_warning");
}

//-----------------------------------------------------------------------------
AppearanceConfig::AppearanceConfig()
    : KConfigWidget(i18n("Appearance"), "appearance")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    KIntNumInput *in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    configCollection()->createConfigItem("case size", in);
    top->addWidget(in);

    top->addSpacing(2 * KDialog::spacingHint());

    QGrid *grid = new QGrid(2, this);
    top->addWidget(grid);

    for (uint i=0; i<NB_COLORS; i++) {
        QLabel *l = new QLabel(grid);
        KColorButton *cb = new KColorButton(grid);
        cb->setFixedWidth(100);
        KConfigItem *set =
            configCollection()->createConfigItem(COLOR_CONFIG_NAMES[i], cb);
        set->setProxyLabel(l);
    }

	for (uint i=0; i<NB_N_COLORS; i++) {
		QLabel *l = new QLabel(grid);
        KColorButton *cb = new KColorButton(grid);
        cb->setFixedWidth(100);
        KConfigItem *set =
            configCollection()->createConfigItem(N_COLOR_CONFIG_NAMES[i], cb);
        set->setProxyLabel(l);
	}
}
