#include "dialogs.moc"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qhbox.h>

#include <kapp.h>
#include <klocale.h>

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
	QPalette p = palette();
	p.setColor(QColorGroup::Foreground, white);
	setPalette(p);
	state = true;
	setState(false);
}

void LCDNumber::setState(bool _state)
{
	if ( _state==state ) return;
	QPalette p = palette();
	if (_state) p.setColor(QColorGroup::Background, red);
	else p.setColor(QColorGroup::Background, black);
	setPalette(p);
	state = _state;
}

//-----------------------------------------------------------------------------
DigitalClock::DigitalClock(QWidget *parent, const char *name)
: LCDNumber(parent, name), max_secs(0)
{}

void DigitalClock::timerEvent( QTimerEvent *)
{
 	if (!stop) {
		_sec++;
		if (_sec==60) {
			/* so waiting one hour don't do a restart timer at 00:00 */
			if ( _min<60 ) _min++;
			_sec = 0;
		}
		showTime();
		if ( toSec(_sec, _min)==max_secs ) setState(true);
	}
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
	startTimer(1000); //  1 seconde

	setState(false);
	showTime();
}

//-----------------------------------------------------------------------------
const uint MIN_CUSTOM_SIZE = 8;
const uint MAX_CUSTOM_SIZE = 50;

CustomDialog::CustomDialog(Level &_lev, QWidget *parent)
: KDialogBase(Plain, i18n("Customize your game"), Ok|Cancel, Cancel,
			  parent, 0, true, true),
  lev(_lev), initLev(_lev)
{
	lev.type = Custom;

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
const char *HS_NAME   = "Name";
const char *HS_MIN    = "Min";
const char *HS_SEC    = "Sec";
const char *HS_GRP[NbLevels-1] =
    { "Easy level", "Normal level", "Expert level" };

uint WHighScores::time(GameType type)
{
	KConfig *conf = kapp->config();
	conf->setGroup(HS_GRP[type]);

	int sec = conf->readNumEntry(HS_SEC, 59);
	int min = conf->readNumEntry(HS_MIN, 59);
	if ( sec<0 || sec>59 || min<0 || min>59 )
		return DigitalClock::toSec(59, 59);
	return DigitalClock::toSec(sec, min);
}

WHighScores::WHighScores(QWidget *parent, const Score *score)
: KDialogBase(Plain, i18n("Hall of Fame"), Close, Close,
			  parent, 0, true, true),
  qle(0)
{
	KConfig *conf = kapp->config();

	if (score) { // set highscores
		type = score->type;
		conf->setGroup(HS_GRP[type]);
		conf->writeEntry(HS_NAME, i18n("Anonymous")); // default
		conf->writeEntry(HS_MIN, score->min);
		conf->writeEntry(HS_SEC, score->sec);
	}

	QLabel *lab;
	QFont f( font() );
	f.setBold(true);

	QVBoxLayout *top = new QVBoxLayout(plainPage(), spacingHint());

/* Grid layout */
	QGridLayout *gl = new QGridLayout(3, 4, spacingHint());
	top->addLayout(gl);

	/* level names */
	QString str;
	for(int k=0; k<3; k++) {
		if ( k==0 ) str = i18n("Easy");
		else if ( k==1 ) str = i18n("Normal");
		else str = i18n("Expert");
		lab = new QLabel(str, plainPage());
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 0);

		lab = new QLabel(":", plainPage());
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 1);
		
		conf->setGroup(HS_GRP[k]);
		int min = conf->readNumEntry(HS_MIN, 0);
		int sec = conf->readNumEntry(HS_SEC, 0);
		bool no_score = false;
		
		if ( !score || (k!=type) ) {
			lab = new QLabel(plainPage());
			lab->setFont(f);
			QString name = conf->readEntry(HS_NAME, "");
			no_score = name.isEmpty() || (!min && !sec);
			
			if (no_score) {
				lab->setText(i18n("no score for this level"));
				lab->setMinimumSize( lab->sizeHint() );
				gl->addWidget(lab, k, 2);
				continue;
			} else {
				lab->setText(name);
				lab->setMinimumSize( lab->sizeHint() );
				gl->addWidget(lab, k, 2);
			}
		} else {
			qle = new QLineEdit(plainPage());
			qle->setMaxLength(10);
			qle->setFont(f);
			qle->setMinimumSize(qle->fontMetrics().maxWidth()*10,
								qle->sizeHint().height());
			qle->setFocus();
			gl->addWidget(qle, k, 2);
		}

		if (min) {
			if (sec) str = i18n("in %1 minutes and %2 seconds.")
						 .arg(min).arg(sec);
			else str = i18n("in %1 minutes.").arg(min);
		} else str = i18n("in %1 seconds.").arg(sec);

		lab = new QLabel(str, plainPage());
		lab->setAlignment(AlignCenter);
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 3);
	}

	if (score) setButtonText(Close, i18n("Set name"));
	_close = !score;
}

void WHighScores::reject()
{
	if (_close) KDialogBase::reject();
	else {
		KConfig *conf = kapp->config();
		conf->setGroup(HS_GRP[type]);
		QString str = qle->text();
		if ( str.length() ) conf->writeEntry(HS_NAME, str);
		conf->sync();
		str = conf->readEntry(HS_NAME);
		qle->setText(str);
		setButtonText(Close, i18n("Close"));
		_close = true;
		qle->setEnabled(false);
	}
}

//-----------------------------------------------------------------------------
const char *OP_GRP             = "Options";
const char *OP_UMARK           = "? mark";
const char *OP_MENUBAR         = "menubar visible";
const char *OP_LEVEL           = "Level";
const char *OP_CUSTOM_WIDTH    = "custom width";
const char *OP_CUSTOM_HEIGHT   = "custom height";
const char *OP_CUSTOM_MINES    = "custom mines";
const char *OP_CASE_SIZE       = "case size";
const char *OP_KEYBOARD        = "keyboard game";
const char *OP_MOUSE_BINDINGS[3] =
    { "mouse left", "mouse mid", "mouse right" };
const char *OP_NUMBER_COLOR    = "color #";
const char *OP_FLAG_COLOR      = "flag color";
const char *OP_EXPLOSION_COLOR = "explosion color";
const char *OP_ERROR_COLOR     = "error color";

#define NCName(i) QString("%1%2").arg(OP_NUMBER_COLOR).arg(i)

const uint MIN_CASE_SIZE     = 20;
const uint DEFAULT_CASE_SIZE = MIN_CASE_SIZE;
const uint MAX_CASE_SIZE     = 100;

const QColor DEFAULT_NUMBER_COLORS[NB_NUMBER_COLORS] =
   { Qt::blue, Qt::darkGreen, Qt::darkYellow, Qt::darkMagenta, Qt::red,
	 Qt::darkRed, Qt::black, Qt::black };
const QColor DEFAULT_FLAG_COLOR      = Qt::red;
const QColor DEFAULT_EXPLOSION_COLOR = Qt::red;
const QColor DEFAULT_ERROR_COLOR     = Qt::red;


OptionDialog::OptionDialog(QWidget *parent)
: KDialogBase(Tabbed, i18n("Settings"), Ok|Cancel|Default, Ok,
			  parent, 0, true, true)
{
	mainPage();
	casePage();
}

void OptionDialog::mainPage()
{
	QFrame *page = addPage(i18n("Main settings"));
	QVBoxLayout *top = new QVBoxLayout(page, spacingHint());

	ni = new KIntNumInput(readCaseSize(), page);
	ni->setRange(MIN_CASE_SIZE, MAX_CASE_SIZE);
	ni->setLabel(i18n("Case size"));
	top->addWidget(ni);
	top->addSpacing(spacingHint());

	um = new QCheckBox(i18n("Enable ? mark"), page);
	um->setChecked(readUMark());
	top->addWidget(um);
	
	keyb = new QCheckBox(i18n("Enable keyboard"), page);
	keyb->setChecked(readKeyboard());
	top->addWidget(keyb);
	top->addSpacing(spacingHint());

	QVGroupBox *gb = new QVGroupBox(i18n("Mouse bindings"), page);
	top->addWidget(gb);
	QGrid *grid = new QGrid(2, gb);
	grid->setSpacing(spacingHint());
	QLabel *lab = new QLabel(i18n("Left button"), grid);
	cb[Left] = new QComboBox(false, grid);
	lab = new QLabel(i18n("Mid button"), grid);
	cb[Mid] = new QComboBox(false, grid);
	lab = new QLabel(i18n("Right button"), grid);
	cb[Right] = new QComboBox(false, grid);
	top->addStretch(1);

	for (uint i=0; i<3; i++) {
		cb[i]->insertItem(i18n("reveal"), 0);
		cb[i]->insertItem(i18n("autoreveal"), 1);
		cb[i]->insertItem(i18n("toggle mark"), 2);
		cb[i]->insertItem(i18n("toggle ? mark"), 3);
		cb[i]->setCurrentItem(readMouseBinding((MouseButton)i));
	}
}

void OptionDialog::casePage()
{
	QFrame *page = addPage(i18n("Color settings"));
	QVBoxLayout *top = new QVBoxLayout(page, spacingHint());

	CaseProperties cp = readCaseProperties();
	
	QHBox *hbox = new QHBox(page);
	top->addWidget(hbox);
	(void)new QLabel(i18n("Flag color"), hbox);
	flagButton = new KColorButton(cp.flagColor, hbox);
	
	hbox = new QHBox(page);
	top->addWidget(hbox);
	(void)new QLabel(i18n("Explosion color"), hbox);
	explosionButton = new KColorButton(cp.explosionColor, hbox);
	
	hbox = new QHBox(page);
	top->addWidget(hbox);
	(void)new QLabel(i18n("Error color"), hbox);
	errorButton = new KColorButton(cp.errorColor, hbox);

	numberButtons.resize(NB_NUMBER_COLORS);
	for (uint i=0; i<NB_NUMBER_COLORS; i++) {
		hbox = new QHBox(page);
		top->addWidget(hbox);
		(void)new QLabel(i==0 ? i18n("One mine color")
						 : i18n("%1 mines color").arg(i+1), hbox);
		numberButtons[i] = new KColorButton(cp.numberColors[i], hbox);
	}
}

KConfig *OptionDialog::config()
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	return conf;
}

void OptionDialog::accept()
{
	KConfig *conf = config();
	conf->writeEntry(OP_UMARK, um->isChecked());
	conf->writeEntry(OP_KEYBOARD, keyb->isChecked());
	for (uint i=0; i<3; i++)
		conf->writeEntry(OP_MOUSE_BINDINGS[i], cb[i]->currentItem());
	conf->writeEntry(OP_CASE_SIZE, ni->value());
	conf->writeEntry(OP_FLAG_COLOR, flagButton->color());
	conf->writeEntry(OP_EXPLOSION_COLOR, explosionButton->color());
	conf->writeEntry(OP_ERROR_COLOR, errorButton->color());
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
		conf->writeEntry(NCName(i), numberButtons[i]->color());
	
	KDialogBase::accept();
}

bool OptionDialog::readUMark()
{
	return config()->readBoolEntry(OP_UMARK, true);
}

bool OptionDialog::readKeyboard()
{
	return config()->readBoolEntry(OP_KEYBOARD, false);
}

Level OptionDialog::readLevel()
{
	Level l;
	l.type = (GameType)config()->readUnsignedNumEntry(OP_LEVEL, 0);
	if ( l.type>Custom ) l.type = Easy;

	if ( l.type==Custom ) {
		l.width  = config()->readUnsignedNumEntry(OP_CUSTOM_WIDTH, 0);
		l.width  = QMAX(QMIN(l.width, MAX_CUSTOM_SIZE), MIN_CUSTOM_SIZE);
		l.height = config()->readUnsignedNumEntry(OP_CUSTOM_HEIGHT, 0);		
		l.height = QMAX(QMIN(l.height, MAX_CUSTOM_SIZE), MIN_CUSTOM_SIZE);
		l.nbMines  = config()->readUnsignedNumEntry(OP_CUSTOM_MINES, 0);
		l.nbMines  = QMAX(QMIN(l.nbMines,
							  CustomDialog::maxNbMines(l.width, l.height)), 1);
	} else l = LEVELS[l.type];
		
	return l;
}

void OptionDialog::writeLevel(const Level &l)
{
	if ( l.type==Custom ) {
		config()->writeEntry(OP_CUSTOM_WIDTH, l.width);
		config()->writeEntry(OP_CUSTOM_HEIGHT, l.height);
		config()->writeEntry(OP_CUSTOM_MINES, l.nbMines);
	}
	config()->writeEntry(OP_LEVEL, (uint)l.type);
}

bool OptionDialog::readMenuVisible()
{
	return config()->readBoolEntry(OP_MENUBAR, true);
}

void OptionDialog::writeMenuVisible(bool visible)
{
	config()->writeEntry(OP_MENUBAR, visible);
}

MouseAction OptionDialog::readMouseBinding(MouseButton mb)
{
	MouseAction ma = (MouseAction)config()
		->readUnsignedNumEntry(OP_MOUSE_BINDINGS[mb], mb);
	return ma>UMark ? Reveal : ma;
}

QColor OptionDialog::readColor(const QString & key, QColor defaultColor)
{
	return config()->readColorEntry(key, &defaultColor);
}

uint OptionDialog::readCaseSize()
{
	uint cs = config()->readUnsignedNumEntry(OP_CASE_SIZE, DEFAULT_CASE_SIZE);
	cs = QMAX(QMIN(cs, MAX_CASE_SIZE), MIN_CASE_SIZE);
	return cs;
}

CaseProperties OptionDialog::readCaseProperties()
{
	CaseProperties cp;
	cp.size = readCaseSize();
	cp.flagColor = readColor(OP_FLAG_COLOR, DEFAULT_FLAG_COLOR);
	cp.explosionColor = readColor(OP_EXPLOSION_COLOR, DEFAULT_EXPLOSION_COLOR);
	cp.errorColor = readColor(OP_ERROR_COLOR, DEFAULT_ERROR_COLOR);
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
		cp.numberColors[i] = readColor(NCName(i), DEFAULT_NUMBER_COLORS[i]);
	return cp;
}

void OptionDialog::slotDefault()
{
	ni->setValue(DEFAULT_CASE_SIZE);
	um->setChecked(true);
	keyb->setChecked(true);
	for (uint i=0; i<3; i++) cb[i]->setCurrentItem(i);
	flagButton->setColor(DEFAULT_FLAG_COLOR);
	explosionButton->setColor(DEFAULT_EXPLOSION_COLOR);
	errorButton->setColor(DEFAULT_ERROR_COLOR);
	for (uint i=0; i<NB_NUMBER_COLORS; i++)
		numberButtons[i]->setColor(DEFAULT_NUMBER_COLORS[i]);
}
