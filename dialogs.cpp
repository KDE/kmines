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
: LCDClock(parent, "digital_clock")
{
    setFrameStyle(Panel | Sunken);
    setDefaultColors(white, black);
}

KExtHighscores::Score DigitalClock::score() const
{
    KExtHighscores::Score score(KExtHighscores::Won);
    score.setData("score", time());
    score.setData("nb_actions", _nbActions);
    return score;
}

void DigitalClock::timeoutClock()
{
    LCDClock::timeoutClock();

    if (_cheating) setColor(white);
    else if ( _first<score() ) setColor(red);
    else if ( _last<score() ) setColor(blue);
    else setColor(white);
}

void DigitalClock::start()
{
    LCDClock::start();
    if ( !_cheating ) setColor(red);
}

void DigitalClock::reset(const KExtHighscores::Score &first,
                         const KExtHighscores::Score &last)
{
    _nbActions = 0;
    _first = first;
    _last = last;
    _cheating = false;
    LCDClock::reset();
    resetColor();
}

void DigitalClock::setCheating()
{
    _cheating = true;
    setColor(white);
}

//-----------------------------------------------------------------------------
KRangedUIConfig *createWidth(KUIConfigCollection *col)
{
    return new KRangedUIConfig(KRangedUIConfig::IntInput,
                               KMines::OP_GROUP, "custom width",
                               Level::data(Level::Custom).width,
                               Level::MIN_CUSTOM_SIZE, Level::MAX_CUSTOM_SIZE,
                               col, i18n("Width"));
}

KRangedUIConfig *createHeight(KUIConfigCollection *col)
{
    return new KRangedUIConfig(KRangedUIConfig::IntInput,
                               KMines::OP_GROUP, "custom height",
                               Level::data(Level::Custom).height,
                               Level::MIN_CUSTOM_SIZE, Level::MAX_CUSTOM_SIZE,
                               col, i18n("Height"));
}

KRangedUIConfig *createMines(KUIConfigCollection *col)
{
    return new KRangedUIConfig(KRangedUIConfig::IntInput,
                              KMines::OP_GROUP, "custom mines",
                              Level::data(Level::Custom).nbMines,
                              0, Level::MAX_CUSTOM_SIZE*Level::MAX_CUSTOM_SIZE,
                              col, " " //#### dummy string necessary
                              );
}

CustomConfig::CustomConfig()
    : KUIConfigWidget(i18n("Custom Game"), "configure")
{
	QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    KIntNumInput *in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    _width = createWidth(UIConfigCollection());
    _width->associate(in);
    top->addWidget(in);

    in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    _height = createHeight(UIConfigCollection());
    _height->associate(in);
	top->addWidget(in);

    in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    _mines = createMines(UIConfigCollection());
    _mines->associate(in);
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

    connect(UIConfigCollection(), SIGNAL(modified()), SLOT(updateNbMines()));
    QTimer::singleShot(0, this, SLOT(updateNbMines()));
}

void CustomConfig::updateNbMines()
{
    Level level(_width->value().toUInt(), _height->value().toUInt(),
                _mines->value().toUInt());
	_mines->setRange(1, level.maxNbMines());
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
    uint w = createWidth(0)->configValue().toUInt();
    uint h = createHeight(0)->configValue().toUInt();
    uint n = createMines(0)->configValue().toUInt();
    return Level(w, h, n);
}

//-----------------------------------------------------------------------------
const char *BINDING_LABELS[3]
    = { I18N_NOOP("Left button"), I18N_NOOP("Mid button"),
        I18N_NOOP("Right button") };
const char *OP_MOUSE_BINDINGS[3] = { "mouse left", "mouse mid", "mouse right"};
const char *ACTION_BINDINGS[4] =
    { I18N_NOOP("reveal"), I18N_NOOP("autoreveal"),
      I18N_NOOP("toggle flag"), I18N_NOOP("toggle ? mark") };

KUIConfig *createUMark(KUIConfigCollection *col)
{
    return new KSimpleUIConfig(KSimpleUIConfig::CheckBox,
                               KMines::OP_GROUP, "? mark", true,
                               col, i18n("Enable ? mark"));
}

KUIConfig *createKeyboard(KUIConfigCollection *col)
{
    return new KSimpleUIConfig(KSimpleUIConfig::CheckBox,
                               KMines::OP_GROUP, "keyboard game", false,
                               col, i18n("Enable keyboard"));
}

KUIConfig *createPauseFocus(KUIConfigCollection *col)
{
    return new KSimpleUIConfig(KSimpleUIConfig::CheckBox,
                               KMines::OP_GROUP, "paused if lose focus", true,
                               col, i18n("Pause if window lose focus"));
}

KMultiUIConfig *createMouseBinding(KUIConfigCollection *col, uint i)
{
    KMultiUIConfig *s =
        new KMultiUIConfig(KMultiUIConfig::ReadOnlyComboBox, 4,
                          KMines::OP_GROUP, OP_MOUSE_BINDINGS[i],
                          ACTION_BINDINGS[i], col, i18n(BINDING_LABELS[i]));
    for (uint j=0; j<4; j++)
        s->map(j, ACTION_BINDINGS[j], i18n(ACTION_BINDINGS[j]));
    return s;
}

KUIConfig *createMagicReveal(KUIConfigCollection *col)
{
    return new KSimpleUIConfig(KSimpleUIConfig::CheckBox,
                               KMines::OP_GROUP, "magic reveal", false,
                               col, i18n("\"Magic\" reveal"));
}

GameConfig::GameConfig()
    : KUIConfigWidget(i18n("Game"), "misc")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QCheckBox *cb = new QCheckBox(this);
    KUIConfig *set = createUMark(UIConfigCollection());
    set->associate(cb);
    top->addWidget(cb);

    cb = new QCheckBox(this);
    set = createKeyboard(UIConfigCollection());
    set->associate(cb);
    top->addWidget(cb);

    cb = new QCheckBox(this);
    set = createPauseFocus(UIConfigCollection());
    set->associate(cb);
    top->addWidget(cb);

    cb = new QCheckBox(this);
    _magic = createMagicReveal(UIConfigCollection());
    _magic->associate(cb);
    connect(_magic, SIGNAL(modified()), SLOT(magicRevealToggled()));
    QWhatsThis::add(cb, i18n("Set flags and reveal cases for the non-trivial "
                             "cases."));
    top->addWidget(cb);

	top->addSpacing(2 * KDialog::spacingHint());

	QVGroupBox *gb = new QVGroupBox(i18n("Mouse Bindings"), this);
	top->addWidget(gb);
	QGrid *grid = new QGrid(2, gb);
	grid->setSpacing(10);
    for (uint i=0; i<3; i++) {
        QLabel *l = new QLabel(grid);
        QComboBox *cb = new QComboBox(false, grid);
        set = createMouseBinding(UIConfigCollection(), i);
        set->setProxyLabel(l);
        set->associate(cb);
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

bool GameConfig::readUMark()
{
    return createUMark(0)->configValue().toBool();
}

bool GameConfig::readKeyboard()
{
    return createKeyboard(0)->configValue().toBool();
}

bool GameConfig::readPauseFocus()
{
    return createPauseFocus(0)->configValue().toBool();
}

KMines::MouseAction GameConfig::readMouseBinding(MouseButton mb)
{
    return (MouseAction)createMouseBinding(0, mb)->configId();
}

bool GameConfig::readMagicReveal()
{
    return createMagicReveal(0)->configValue().toBool();
}

//-----------------------------------------------------------------------------
const uint MIN_CASE_SIZE = 8;
const uint DEF_CASE_SIZE = 20;
const uint MAX_CASE_SIZE = 100;

struct ColorData {
    const char *entry;
    const char *label;
    QColor      def;
};

const ColorData COLOR_DATA[KMines::NB_COLORS] = {
    { "flag color",      I18N_NOOP("Flag color"),      Qt::red },
    { "explosion color", I18N_NOOP("Explosion color"), Qt::red },
    { "error color",     I18N_NOOP("Error color"),     Qt::red }
};

const QColor DEFAULT_NUMBER_COLOR[KMines::NB_NUMBER_COLORS] = {
    Qt::blue, Qt::darkGreen, Qt::darkYellow, Qt::darkMagenta, Qt::red,
    Qt::darkRed, Qt::black, Qt::black
};

KRangedUIConfig *createCaseSize(KUIConfigCollection *col)
{
    return new KRangedUIConfig(KRangedUIConfig::IntInput,
                               KMines::OP_GROUP, "case size", DEF_CASE_SIZE,
                               MIN_CASE_SIZE, MAX_CASE_SIZE,
                               col, i18n("Case size"));
}

KUIConfig *createColor(KUIConfigCollection *col, uint i)
{
    return new KSimpleUIConfig(KSimpleUIConfig::ColorButton,
                               KMines::OP_GROUP, COLOR_DATA[i].entry,
                               COLOR_DATA[i].def,
                               col, i18n(COLOR_DATA[i].label));
}

KUIConfig *createNumberColor(KUIConfigCollection *col, uint i)
{
    return new KSimpleUIConfig(KSimpleUIConfig::ColorButton,
                               KMines::OP_GROUP, QString("color #%1").arg(i),
                               DEFAULT_NUMBER_COLOR[i], col,
                               i18n("%n mine color", "%n mines color", i+1));
}

AppearanceConfig::AppearanceConfig()
    : KUIConfigWidget(i18n("Appearance"), "appearance")
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    KIntNumInput *in = new KIntNumInput(this);
    in->setRange(0, 100); // #### to have a slider
    KUIConfig *set = createCaseSize(UIConfigCollection());
    set->associate(in);
    top->addWidget(in);

    top->addSpacing(2 * KDialog::spacingHint());

    QGrid *grid = new QGrid(2, this);
    top->addWidget(grid);

    for (uint i=0; i<NB_COLORS; i++) {
        QLabel *l = new QLabel(grid);
        KColorButton *cb = new KColorButton(grid);
        cb->setFixedWidth(100);
        set = createColor(UIConfigCollection(), i);
        set->setProxyLabel(l);
        set->associate(cb);
    }

	for (uint i=0; i<NB_NUMBER_COLORS; i++) {
		QLabel *l = new QLabel(grid);
        KColorButton *cb = new KColorButton(grid);
        cb->setFixedWidth(100);
        set = createNumberColor(UIConfigCollection(), i);
        set->setProxyLabel(l);
        set->associate(cb);
	}
}

KMines::CaseProperties AppearanceConfig::readCaseProperties()
{
    CaseProperties cp;
    cp.size = createCaseSize(0)->configValue().toUInt();
    for (uint i=0; i<NB_COLORS; i++)
        cp.colors[i] = createColor(0, i)->configValue().toColor();
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
        cp.numberColors[i] = createNumberColor(0, i)->configValue().toColor();
	return cp;
}
