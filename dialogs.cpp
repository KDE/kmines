#include "dialogs.h"
#include "dialogs.moc"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>

#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>

#include "bitmaps/smile"
#include "bitmaps/smile_happy"
#include "bitmaps/smile_ohno"
#include "bitmaps/smile_stress"


//-----------------------------------------------------------------------------
Smiley::Smiley(QWidget *parent, const char *name)
: QPushButton("", parent, name), normal(smile_xpm), stressed(smile_stress_xpm),
  happy(smile_happy_xpm), sad(smile_ohno_xpm)
{}

void Smiley::setMood(Mood mood)
{
	switch (mood) {
	case Normal:   setPixmap(normal);   break;
	case Stressed: setPixmap(stressed); break;
	case Happy:    setPixmap(happy);    break;
	case Sad:      setPixmap(sad);      break;
	}
}

//-----------------------------------------------------------------------------
LCDNumber::LCDNumber(QWidget *parent, const char *name)
: QLCDNumber(parent, name)
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setSegmentStyle(Flat);
    setColor(white);
}

void LCDNumber::setColor(QColor color)
{
	QPalette p = palette();
	p.setColor(QColorGroup::Background, black);
    p.setColor(QColorGroup::Foreground, color);
	setPalette(p);
}

//-----------------------------------------------------------------------------
DigitalClock::DigitalClock(QWidget *parent, const char *name)
: LCDNumber(parent, name)
{}

void DigitalClock::timerEvent( QTimerEvent *)
{
 	if ( stop ) return;

    if ( _min==59 && _sec==59 ) return; // waiting an hour do not restart timer
    _sec++;
    if (_sec==60) {
        _min++;
        _sec = 0;
    }
    showTime();

    if ( score()<=_lastScore ) setColor(white);
    else if ( score()<=_firstScore ) setColor(blue);
    else setColor(red);
}

void DigitalClock::showTime()
{
	char s[6] = "00:00";
	if (_min>=10) s[0] += _min / 10;
	s[1] += _min % 10;
	if (_sec>=10) s[3] += _sec / 10;
	s[4] += _sec % 10;

	display(s);
}

void DigitalClock::zero()
{
	killTimers();

	stop = true;
	_sec = 0; _min = 0;
	startTimer(1000); // one second

	setColor(white);
	showTime();
}

//-----------------------------------------------------------------------------
const uint MIN_CUSTOM_SIZE = 8;
const uint MAX_CUSTOM_SIZE = 50;

CustomDialog::CustomDialog(LevelData &_lev, QWidget *parent)
: KDialogBase(Plain, i18n("Customize your game"), Ok|Cancel, Cancel,
			  parent, 0, true, true),
  lev(_lev), initLev(_lev)
{
	lev.level = Custom;

	QVBoxLayout *top = new QVBoxLayout(plainPage(), spacingHint());

	// width
	KIntNumInput *ki = new KIntNumInput(lev.width, plainPage());
	ki->setLabel(i18n("Width"));
	ki->setRange(MIN_CUSTOM_SIZE, MAX_CUSTOM_SIZE);
	connect(ki, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
	top->addWidget(ki);

	// height
	ki = new KIntNumInput(lev.height, plainPage());
	ki->setLabel(i18n("Height"));
	ki->setRange(MIN_CUSTOM_SIZE, MAX_CUSTOM_SIZE);
	connect(ki, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
	top->addWidget(ki);

	// mines
	km = new KIntNumInput(lev.nbMines, plainPage());
	connect(km, SIGNAL(valueChanged(int)), SLOT(nbMinesChanged(int)));
	top->addWidget(km);
	updateNbMines();
}

void CustomDialog::widthChanged(int n)
{
	lev.width = (uint)n;
	updateNbMines();
}

void CustomDialog::heightChanged(int n)
{
	lev.height = (uint)n;
	updateNbMines();
}

void CustomDialog::nbMinesChanged(int n)
{
	lev.nbMines = (uint)n;
	updateNbMines();
}

void CustomDialog::updateNbMines()
{
	km->setRange(1, maxNbMines(lev.width, lev.height));
	uint nb = lev.width * lev.height;
	km->setLabel(i18n("Mines (%1%)").arg(100*lev.nbMines/nb));
	enableButton(Ok, lev.width!=initLev.width || lev.height!=initLev.height
				 || lev.nbMines!=initLev.nbMines);
}

uint CustomDialog::maxNbMines(uint width, uint height)
{
	return width*height - 2;
}

//-----------------------------------------------------------------------------
const char *OP_GRP = "Options";
const char *OP_UMARK             = "? mark";
const char *OP_CASE_SIZE         = "case size";
const char *OP_KEYBOARD          = "keyboard game";
const char *OP_MOUSE_BINDINGS[3] =
    { "mouse left", "mouse mid", "mouse right" };

const uint MIN_CASE_SIZE     = 20;
const uint DEFAULT_CASE_SIZE = MIN_CASE_SIZE;
const uint MAX_CASE_SIZE     = 100;

const char *OP_NUMBER_COLOR    = "color #";
const char *OP_FLAG_COLOR      = "flag color";
const char *OP_EXPLOSION_COLOR = "explosion color";
const char *OP_ERROR_COLOR     = "error color";
#define NCName(i) QString("%1%2").arg(OP_NUMBER_COLOR).arg(i)

const QColor DEFAULT_NUMBER_COLORS[NB_NUMBER_COLORS] =
   { Qt::blue, Qt::darkGreen, Qt::darkYellow, Qt::darkMagenta, Qt::red,
	 Qt::darkRed, Qt::black, Qt::black };
const QColor DEFAULT_FLAG_COLOR      = Qt::red;
const QColor DEFAULT_EXPLOSION_COLOR = Qt::red;
const QColor DEFAULT_ERROR_COLOR     = Qt::red;

const char *OP_MENUBAR         = "menubar visible";
const char *OP_LEVEL           = "Level";
const char *OP_CUSTOM_WIDTH    = "custom width";
const char *OP_CUSTOM_HEIGHT   = "custom height";
const char *OP_CUSTOM_MINES    = "custom mines";

OptionDialog::OptionDialog(QWidget *parent)
: KDialogBase(IconList, i18n("Configure"), Ok|Cancel|Default, Ok,
			  parent, "option_dialog", true, true)
{
    // game
    QFrame *page = addPage(i18n("Game"), QString::null,
                           BarIcon("misc", KIcon::SizeLarge));
	QVBoxLayout *top = new QVBoxLayout(page, spacingHint());

    _caseSize = new KIntNumInput(readCaseSize(), page);
	_caseSize->setRange(MIN_CASE_SIZE, MAX_CASE_SIZE);
	_caseSize->setLabel(i18n("Case size"));
	top->addWidget(_caseSize);
	top->addSpacing(10);

	_umark = new QCheckBox(i18n("Enable ? mark"), page);
	_umark->setChecked(readUMark());
	top->addWidget(_umark);

	_keyb = new QCheckBox(i18n("Enable keyboard"), page);
	_keyb->setChecked(readKeyboard());
	top->addWidget(_keyb);
	top->addSpacing(10);

	QVGroupBox *gb = new QVGroupBox(i18n("Mouse bindings"), page);
	top->addWidget(gb);
	QGrid *grid = new QGrid(2, gb);
	grid->setSpacing(10);
	QLabel *lab = new QLabel(i18n("Left button"), grid);
	_cb[Left] = new QComboBox(false, grid);
	lab = new QLabel(i18n("Mid button"), grid);
	_cb[Mid] = new QComboBox(false, grid);
	lab = new QLabel(i18n("Right button"), grid);
	_cb[Right] = new QComboBox(false, grid);
	top->addStretch(1);

	for (uint i=0; i<3; i++) {
		_cb[i]->insertItem(i18n("reveal"), 0);
		_cb[i]->insertItem(i18n("autoreveal"), 1);
		_cb[i]->insertItem(i18n("toggle mark"), 2);
		_cb[i]->insertItem(i18n("toggle ? mark"), 3);
		_cb[i]->setCurrentItem(readMouseBinding((MouseButton)i));
	}

    // colors
    page = addPage(i18n("Colors"), QString::null,
                   BarIcon("colorize", KIcon::SizeLarge));
	top = new QVBoxLayout(page, spacingHint());

    CaseProperties cp = OptionDialog::readCaseProperties();

	QHBox *hbox = new QHBox(page);
	top->addWidget(hbox);
	(void)new QLabel(i18n("Flag color"), hbox);
	_flag = new KColorButton(cp.flagColor, hbox);

	hbox = new QHBox(page);
	top->addWidget(hbox);
	(void)new QLabel(i18n("Explosion color"), hbox);
	_explosion = new KColorButton(cp.explosionColor, hbox);

	hbox = new QHBox(page);
	top->addWidget(hbox);
	(void)new QLabel(i18n("Error color"), hbox);
	_error = new KColorButton(cp.errorColor, hbox);

	_numbers.resize(NB_NUMBER_COLORS);
	for (uint i=0; i<NB_NUMBER_COLORS; i++) {
		hbox = new QHBox(page);
		top->addWidget(hbox);
		(void)new QLabel(i==0 ? i18n("One mine color")
						 : i18n("%1 mines color").arg(i+1), hbox);
		_numbers[i] = new KColorButton(cp.numberColors[i], hbox);
	}

    // highscores
    highscores = new HighscoresOption(this);
}

KConfig *OptionDialog::config()
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	return conf;
}

void OptionDialog::accept()
{
    if ( !highscores->accept() ) return;

    KConfig *conf = config();
	conf->writeEntry(OP_CASE_SIZE, _caseSize->value());
	conf->writeEntry(OP_UMARK, _umark->isChecked());
	conf->writeEntry(OP_KEYBOARD, _keyb->isChecked());
	for (uint i=0; i<3; i++)
		conf->writeEntry(OP_MOUSE_BINDINGS[i], _cb[i]->currentItem());

    conf->writeEntry(OP_FLAG_COLOR, _flag->color());
	conf->writeEntry(OP_EXPLOSION_COLOR, _explosion->color());
	conf->writeEntry(OP_ERROR_COLOR, _error->color());
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
		conf->writeEntry(NCName(i), _numbers[i]->color());

    KDialogBase::accept();
}

void OptionDialog::slotDefault()
{
    _caseSize->setValue(DEFAULT_CASE_SIZE);
    _umark->setChecked(true);
    _keyb->setChecked(false);

    _flag->setColor(DEFAULT_FLAG_COLOR);
	_explosion->setColor(DEFAULT_EXPLOSION_COLOR);
	_error->setColor(DEFAULT_ERROR_COLOR);
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
		_numbers[i]->setColor(DEFAULT_NUMBER_COLORS[i]);
}

LevelData OptionDialog::readLevel()
{
	LevelData l;
	l.level = (Level)config()->readUnsignedNumEntry(OP_LEVEL, 0);
	if ( l.level>Custom ) l.level = Easy;

	if ( l.level==Custom ) {
		l.width  = config()->readUnsignedNumEntry(OP_CUSTOM_WIDTH, 0);
		l.width  = QMAX(QMIN(l.width, MAX_CUSTOM_SIZE), MIN_CUSTOM_SIZE);
		l.height = config()->readUnsignedNumEntry(OP_CUSTOM_HEIGHT, 0);
		l.height = QMAX(QMIN(l.height, MAX_CUSTOM_SIZE), MIN_CUSTOM_SIZE);
		l.nbMines  = config()->readUnsignedNumEntry(OP_CUSTOM_MINES, 0);
		l.nbMines  = QMAX(QMIN(l.nbMines,
							  CustomDialog::maxNbMines(l.width, l.height)), 1);
	} else l = LEVELS[l.level];

	return l;
}

void OptionDialog::writeLevel(const LevelData &l)
{
	if ( l.level==Custom ) {
		config()->writeEntry(OP_CUSTOM_WIDTH, l.width);
		config()->writeEntry(OP_CUSTOM_HEIGHT, l.height);
		config()->writeEntry(OP_CUSTOM_MINES, l.nbMines);
	}
	config()->writeEntry(OP_LEVEL, (uint)l.level);
}

bool OptionDialog::readMenuVisible()
{
	return config()->readBoolEntry(OP_MENUBAR, true);
}

void OptionDialog::writeMenuVisible(bool visible)
{
	config()->writeEntry(OP_MENUBAR, visible);
}

CaseProperties OptionDialog::readCaseProperties()
{
	CaseProperties cp;
	cp.size = readCaseSize();
	cp.flagColor = readColor(OP_FLAG_COLOR, DEFAULT_FLAG_COLOR);
	cp.explosionColor
        = readColor(OP_EXPLOSION_COLOR, DEFAULT_EXPLOSION_COLOR);
	cp.errorColor
        = readColor(OP_ERROR_COLOR, DEFAULT_ERROR_COLOR);
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
		cp.numberColors[i]
            = readColor(NCName(i), DEFAULT_NUMBER_COLORS[i]);
	return cp;
}

bool OptionDialog::readUMark()
{
	return config()->readBoolEntry(OP_UMARK, true);
}

bool OptionDialog::readKeyboard()
{
	return config()->readBoolEntry(OP_KEYBOARD, false);
}

MouseAction OptionDialog::readMouseBinding(MouseButton mb)
{
	MouseAction ma = (MouseAction)config()
                     ->readUnsignedNumEntry(OP_MOUSE_BINDINGS[mb], mb);
	return ma>UMark ? Reveal : ma;
}

uint OptionDialog::readCaseSize()
{
	uint cs = config()
              ->readUnsignedNumEntry(OP_CASE_SIZE, DEFAULT_CASE_SIZE);
	cs = QMAX(QMIN(cs, MAX_CASE_SIZE), MIN_CASE_SIZE);
	return cs;
}

QColor OptionDialog::readColor(const QString & key, QColor defaultColor)
{
	return config()->readColorEntry(key, &defaultColor);
}
