#include "dialogs.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qvgroupbox.h>

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
	state = TRUE;
	setState(FALSE);
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
		if ( toSec(_sec, _min)==max_secs ) setState(TRUE);
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
	
	stop = TRUE;
	_sec = 0; _min = 0;
	startTimer(1000); //  1 seconde

	setState(FALSE);
	showTime();
}

//-----------------------------------------------------------------------------
DialogBase::DialogBase(const QString &caption, int buttonMask,
					   ButtonCode defaultButton,
					   QWidget *parent, const char *name)
: KDialogBase(Plain, caption, buttonMask ,defaultButton, parent, name, TRUE,
			  TRUE)
{
	// top layout
	top = new QVBoxLayout(plainPage(), spacingHint());
	top->setResizeMode(QLayout::Fixed);
	
	// title
	QLabel *lab = new QLabel(caption, plainPage());
	QFont f( font() );
	f.setBold(TRUE);
	lab->setFont(f);
	lab->setAlignment(AlignCenter);
	lab->setFrameStyle(QFrame::Panel | QFrame::Raised);
	top->addWidget(lab);
	top->addSpacing(2*spacingHint());
}

//-----------------------------------------------------------------------------
CustomDialog::CustomDialog(Level &_lev, QWidget *parent)
: DialogBase(i18n("Customize your game"), Ok|Cancel, Cancel, parent),
  lev(_lev), initLev(_lev)
{
	// width
	KIntNumInput *ki = new KIntNumInput(lev.width, plainPage());
	ki->setLabel(i18n("Width"));
	ki->setRange(8, 50);
	connect(ki, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
	top->addWidget(ki);

	// height
	ki = new KIntNumInput(lev.height, plainPage());
	ki->setLabel(i18n("Height"));
	ki->setRange(8, 50);
	connect(ki, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
	top->addWidget(ki);

	// mines
	km = new KIntNumInput(lev.nbMines, plainPage());
	connect(km, SIGNAL(valueChanged(int)), SLOT(nbMinesChanged(int)));
	top->addWidget(km);
	nbMinesChanged(lev.nbMines);
}

void CustomDialog::widthChanged(int n)
{
	lev.width = (uint)n;
	nbMinesChanged(lev.nbMines);
}

void CustomDialog::heightChanged(int n)
{
	lev.height = (uint)n;
	nbMinesChanged(lev.nbMines);
}

void CustomDialog::nbMinesChanged(int n)
{
	lev.nbMines = (uint)n;
	uint nb = lev.width * lev.height;
	km->setRange(1, nb - 2);
	km->setLabel(i18n("Mines (%1%)").arg(100*n/nb));
	enableButton(Ok, lev.width!=initLev.width || lev.height!=initLev.height
				 || lev.nbMines!=initLev.nbMines);
}

//-----------------------------------------------------------------------------
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
: DialogBase(i18n("Hall of Fame"), Close, Close, parent), qle(0)
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
	f.setBold(TRUE);

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
		bool no_score = FALSE;
		
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
			connect(qle, SIGNAL(returnPressed()), SLOT(writeName()));
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

	if (score) enableButton(Close, FALSE);
}

void WHighScores::writeName()
{
	KConfig *conf = kapp->config();
	conf->setGroup(HS_GRP[type]);
	QString str = qle->text();
	if ( str.length() ) conf->writeEntry(HS_NAME, str);
	conf->sync();
	str = conf->readEntry(HS_NAME);
	qle->setText(str);
	enableButton(Close, TRUE);
}

void WHighScores::reject()
{
	if ( qle && qle->isEnabled() ) {
		qle->setEnabled(FALSE);
		focusNextPrevChild(TRUE); // sort of hack (wonder why its call in
		                          // setEnabled(FALSE) does nothing ...)
	} else DialogBase::reject();
}

//-----------------------------------------------------------------------------
OptionDialog::OptionDialog(QWidget *parent)
: DialogBase(i18n("Settings"), Ok|Cancel, Cancel, parent)
{
	ni = new KIntNumInput(readCaseSize(), plainPage());
	ni->setRange(MIN_CASE_SIZE, MAX_CASE_SIZE);
	ni->setLabel(i18n("Case size"));
	top->addWidget(ni);
	top->addSpacing(spacingHint());

	um = new QCheckBox(i18n("Enable ? mark"), plainPage());
	um->setChecked(readUMark());
	top->addWidget(um);
	
	keyb = new QCheckBox(i18n("Enable keyboard"), plainPage());
	keyb->setChecked(readKeyboard());
	top->addWidget(keyb);
	top->addSpacing(spacingHint());

	QVGroupBox *gb = new QVGroupBox(i18n("Mouse bindings"), plainPage());
	top->addWidget(gb);
	QGrid *grid = new QGrid(2, gb);
	grid->setSpacing(spacingHint());
	QLabel *lab = new QLabel(i18n("Left button"), grid);
	cb[Left] = new QComboBox(FALSE, grid);
	lab = new QLabel(i18n("Mid button"), grid);
	cb[Mid] = new QComboBox(FALSE, grid);
	lab = new QLabel(i18n("Right button"), grid);
	cb[Right] = new QComboBox(FALSE, grid);

	for (uint i=0; i<3; i++) {
		cb[i]->insertItem(i18n("reveal"), 0);
		cb[i]->insertItem(i18n("toggle mark"), 1);
		cb[i]->insertItem(i18n("autoreveal"), 2);
		cb[i]->insertItem(i18n("toggle ? mark"), 3);
		cb[i]->setCurrentItem(readMouseBinding((MouseButton)i));
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
	conf->writeEntry(OP_CASE_SIZE, ni->value());
	conf->writeEntry(OP_UMARK, um->isChecked());
	conf->writeEntry(OP_KEYBOARD, keyb->isChecked());
	for (uint i=0; i<3; i++)
		conf->writeEntry(OP_MOUSE_BINDINGS[i], cb[i]->currentItem());
	
	DialogBase::accept();
}

uint OptionDialog::readCaseSize()
{
	uint cs = config()->readUnsignedNumEntry(OP_CASE_SIZE, CASE_SIZE);
	return QMAX(QMIN(cs, MAX_CASE_SIZE), MIN_CASE_SIZE);
}

bool OptionDialog::readUMark()
{
	return config()->readBoolEntry(OP_UMARK, TRUE);
}

bool OptionDialog::readKeyboard()
{
	return config()->readBoolEntry(OP_KEYBOARD, TRUE);
}

GameType OptionDialog::readLevel()
{
	GameType lev = (GameType)config()->readUnsignedNumEntry(OP_LEVEL, 0);
	return lev>=Custom ? Easy : lev;
}

void OptionDialog::writeLevel(GameType lev)
{
	if ( lev>=Custom ) return;
	config()->writeEntry(OP_LEVEL, (uint)lev);
}

bool OptionDialog::readMenuVisible()
{
	return config()->readBoolEntry(OP_MENUBAR, TRUE);
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
