#include "dialogs.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
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
  lev(&_lev)
{
	QLabel      *lab;
	QScrollBar  *scb;
	QHBoxLayout *hbl;

/* Width */
	/* labels */
	hbl = new QHBoxLayout(spacingHint());
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Width"), plainPage());
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lab = new QLabel(plainPage());
	lab->setNum((int)_lev.width);
	lab->setAlignment( AlignRight );
	lab->setFixedSize( lab->fontMetrics().maxWidth()*2,
					   lab->sizeHint().height());
	connect(this, SIGNAL(setWidth(int)), lab, SLOT(setNum(int)));
	hbl->addWidget(lab);

	/* scrollbar */
	scb = new QScrollBar(8, 50, 1, 5, _lev.width,
						 QScrollBar::Horizontal, plainPage());
	scb->setMinimumWidth( scb->sizeHint().width() );
	scb->setFixedHeight( scb->sizeHint().height() );
	connect(scb, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
	top->addWidget(scb);
	top->addSpacing(2*spacingHint());
	
/* Height */
	/* labels */
	hbl = new QHBoxLayout(spacingHint());
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Height"), plainPage());
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lab = new QLabel(plainPage());
	lab->setNum((int)_lev.height);
	lab->setAlignment( AlignRight );
	lab->setFixedSize( lab->fontMetrics().maxWidth()*2,
					   lab->sizeHint().height());
	connect(this, SIGNAL(setHeight(int)), lab, SLOT(setNum(int)));
	hbl->addWidget(lab);

	/* scrollbar */
	scb = new QScrollBar(8, 50, 1, 5, _lev.height,
						 QScrollBar::Horizontal, plainPage());
	scb->setMinimumWidth( scb->sizeHint().width() );
	scb->setFixedHeight( scb->sizeHint().height() );
	connect(scb, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
    top->addWidget(scb);
	top->addSpacing(2*spacingHint());

/* Mines */
	/* labels */
	hbl = new QHBoxLayout(spacingHint());
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Mines"), plainPage());
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lab = new QLabel(" ", plainPage());
	lab->setAlignment( AlignRight );
	lab->setFixedSize(lab->fontMetrics().maxWidth()*11,
					  lab->sizeHint().height());
	connect(this, SIGNAL(setNbMines(const QString &)), lab,
			SLOT(setText(const QString &)));
	hbl->addWidget(lab);

	/* scrollbar */
	sm = new QScrollBar(1, _lev.width*_lev.height, 1, 5, _lev.nbMines,
						 QScrollBar::Horizontal, plainPage());
	sm->setMinimumWidth( sm->sizeHint().width() );
	sm->setFixedHeight( sm->sizeHint().height() );
	connect(sm, SIGNAL(valueChanged(int)), SLOT(nbMinesChanged(int)));
	top->addWidget(sm);

	nbMinesChanged(_lev.nbMines);

	enableButton(Ok, FALSE);
}

void CustomDialog::widthChanged(int n)
{
	lev->width = (uint)n;
	emit setWidth(n);
	nbMinesChanged(lev->nbMines);
}
  
void CustomDialog::heightChanged(int n)
{
	lev->height = (uint)n;
	emit setHeight(n);
	nbMinesChanged(lev->nbMines);
}
  
void CustomDialog::nbMinesChanged(int n)
{
	lev->nbMines = (uint)n;
	uint nb = lev->width * lev->height;
	sm->setRange(1, nb - 2);
	emit setNbMines(i18n("%1 (%2%)").arg(n).arg(100*n/nb));
	enableButton(Ok, TRUE);
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
		mode = score->mode;
		conf->setGroup(HS_GRP[mode]);
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
	for(uint k=0; k<3; k++) {
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
		
		if ( !score || (k!=mode) ) {
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
	conf->setGroup(HS_GRP[mode]);
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
OptionDialog::OptionDialog(uint &caseSize, QWidget *parent)
: DialogBase(i18n("Game settings"), Ok|Cancel, Cancel, parent),
  cs(caseSize)
{
	KIntNumInput *ni = new KIntNumInput(i18n("Case size"), MIN_CASE_SIZE, MAX_CASE_SIZE, 1,
										caseSize, QString::null, 10, true,
										plainPage());
	top->addWidget(ni);
	connect(ni, SIGNAL(valueChanged(int)), SLOT(changed(int)));
}

void OptionDialog::changed(int nb)
{
	cs = nb;
}

void OptionDialog::accept()
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	conf->writeEntry(OP_CASE_SIZE, cs);
	DialogBase::accept();
}

uint OptionDialog::caseSize()
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	uint cs = conf->readUnsignedNumEntry(OP_CASE_SIZE, CASE_SIZE);
	cs = QMAX(QMIN(cs, MAX_CASE_SIZE), MIN_CASE_SIZE);
	return cs;
}
