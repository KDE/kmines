#include "dialogs.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qfont.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

#include "version.h"

#define BORDER   10
#define TBORDER   5
#define PS_INC    2

/** Digital Clock ************************************************************/
DigitalClock::DigitalClock(QWidget *parent)
: QLCDNumber(parent, 0), max_secs(0)
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
}

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
		if ( toSec(_sec, _min)==max_secs ) {
			QPalette p = palette();
			p.setColor(QColorGroup::Background, QColor(200, 0, 0));
			setPalette(p);
		}
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

	setPalette(kapp->palette());
	showTime();
}

/** Customize dialog *********************************************************/
CustomDialog::CustomDialog(Level &_lev, QWidget *parent)
: KDialog(parent, 0, TRUE), lev(&_lev)
{
	QLabel      *lab;
	QScrollBar  *scb;
	QHBoxLayout *hbl;
		
	QString str = i18n("Customize Game");
	setCaption(i18n("kmines: %1").arg(str));

/* top layout */
	QVBoxLayout *top = new QVBoxLayout(this, BORDER);
	top->setResizeMode(QLayout::Fixed);
	
/* title */
	lab = new QLabel(str, this);
	QFont f( lab->font() );
	QFontInfo fi(f);
	f.setPointSize(fi.pointSize()+PS_INC);
	f.setBold(TRUE);
	lab->setFont(f);
	lab->setAlignment(AlignCenter);
	lab->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	lab->setFixedSize( lab->sizeHint() + QSize(2*TBORDER, TBORDER) );
	lab->setBackgroundMode( QWidget::PaletteMidlight );
	top->addWidget(lab);
	top->addSpacing(2*BORDER); // some additional space
	top->addStretch(1);

/* Width */
	/* labels */
	hbl = new QHBoxLayout(BORDER);
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Width"), this);
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lab = new QLabel(this);
	lab->setNum((int)_lev.width);
	lab->setAlignment( AlignRight );
	lab->setFixedSize( lab->fontMetrics().maxWidth()*2, lab->sizeHint().height());
	connect(this, SIGNAL(setWidth(int)), lab, SLOT(setNum(int)));
	hbl->addWidget(lab);

	/* scrollbar */
	scb = new QScrollBar(8, 50, 1, 5, _lev.width, QScrollBar::Horizontal, this);
	scb->setMinimumWidth( scb->sizeHint().width() );
	scb->setFixedHeight( scb->sizeHint().height() );
	connect(scb, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
	top->addWidget(scb);
	top->addSpacing(2*BORDER);
	top->addStretch(1);
	
/* Height */
	/* labels */
	hbl = new QHBoxLayout(BORDER);
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Height"), this);
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lab = new QLabel(this);
	lab->setNum((int)_lev.height);
	lab->setAlignment( AlignRight );
	lab->setFixedSize( lab->fontMetrics().maxWidth()*2, lab->sizeHint().height());
	connect(this, SIGNAL(setHeight(int)), lab, SLOT(setNum(int)));
	hbl->addWidget(lab);

	/* scrollbar */
	scb = new QScrollBar(8, 50, 1, 5, _lev.height, QScrollBar::Horizontal, this);
	scb->setMinimumWidth( scb->sizeHint().width() );
	scb->setFixedHeight( scb->sizeHint().height() );
	connect(scb, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
    top->addWidget(scb);
	top->addSpacing(2*BORDER);
	top->addStretch(1);

/* Mines */
	/* labels */
	hbl = new QHBoxLayout(BORDER);
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Mines"), this);
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lab = new QLabel(" ", this);
	lab->setAlignment( AlignRight );
	lab->setFixedSize(lab->fontMetrics().maxWidth()*11, lab->sizeHint().height());
	connect(this, SIGNAL(setNbMines(const QString &)), lab, SLOT(setText(const QString &)));
	hbl->addWidget(lab);

	/* scrollbar */
	sm = new QScrollBar(1, _lev.width*_lev.height, 1, 5, _lev.nbMines,
						 QScrollBar::Horizontal, this);
	sm->setMinimumWidth( sm->sizeHint().width() );
	sm->setFixedHeight( sm->sizeHint().height() );
	connect(sm, SIGNAL(valueChanged(int)), SLOT(nbMinesChanged(int)));
	top->addWidget(sm);
	top->addSpacing(2*BORDER);
	top->addStretch(1);

	nbMinesChanged(_lev.nbMines);
	
/* buttons */
	hbl = new QHBoxLayout(BORDER);
	top->addLayout(hbl);
	
	QPushButton *pok = new QPushButton(i18n("OK"), this);
	connect(pok, SIGNAL(clicked()), SLOT(accept()));
	QPushButton *pcancel = new QPushButton(i18n("Cancel"), this);
	connect(pcancel, SIGNAL(clicked()), SLOT(reject()));
	int minW = QMAX(pok->sizeHint().width(), pcancel->sizeHint().width());
	int minH = pok->sizeHint().height();
	pok->setFixedSize(minW, minH);
	pok->setFocus();
	pcancel->setFixedSize(minW, minH);
	hbl->addStretch(1);
	hbl->addWidget(pok, 0, AlignBottom);
	hbl->addSpacing(BORDER);
	hbl->addWidget(pcancel, 0, AlignBottom);
	hbl->addStretch(1);
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
}

/** HighScore dialog *********************************************************/
uint WHighScores::time(uint mode)
{
	KConfig *conf = kapp->getConfig();
	conf->setGroup(HS_GRP[mode]);

	int sec = conf->readNumEntry(HS_SEC, 59);
	int min = conf->readNumEntry(HS_MIN, 59);
	if ( sec<0 || sec>59 || min<0 || min>59 ) return 0;
	return DigitalClock::toSec(sec, min);
}

WHighScores::WHighScores(QWidget *parent, const Score *score)
: KDialog(parent, 0, TRUE), mode((score ? score->mode : 0))
{
	KConfig *conf = kapp->getConfig();
	conf->setGroup(HS_GRP[mode]);

	if (score) { // set highscores
		conf->writeEntry(HS_NAME, i18n("Anonymous")); // default
		conf->writeEntry(HS_MIN, score->min);
		conf->writeEntry(HS_SEC, score->sec);
	}

	QLabel *lab;
	QString str = i18n("Hall of Fame");
	setCaption(i18n("%1: %2").arg(KMINES_NAME).arg(str));

/* top layout */
	QVBoxLayout *top = new QVBoxLayout(this, BORDER);
	top->setResizeMode( QLayout::Fixed );
	
/* title */
	lab = new QLabel(str, this);
	QFont f( lab->font() );
	QFontInfo fi(f);
	f.setPointSize(fi.pointSize()+PS_INC);
	f.setBold(TRUE);
	lab->setFont(f);
	lab->setAlignment(AlignCenter);
	lab->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	lab->setFixedSize( lab->sizeHint()+QSize(2*TBORDER, TBORDER) );
	lab->setBackgroundMode( QWidget::PaletteMidlight );
	top->addWidget(lab);
	top->addSpacing(2*BORDER); // some additional space

	f = font();
	f.setBold(TRUE);
	
/* Grid layout */
	QGridLayout *gl = new QGridLayout(3, 4, BORDER);
	top->addLayout(gl);

	/* level names */
	for(uint k=0; k<3; k++) {
		if ( k==0 ) str = i18n("Easy");
		else if ( k==1 ) str = i18n("Normal");
		else str = i18n("Expert");
		lab = new QLabel(str, this);
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 0);

		lab = new QLabel(":", this);
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 1);
		
		conf->setGroup(HS_GRP[k]);
		int min = conf->readNumEntry(HS_MIN, 0);
		int sec = conf->readNumEntry(HS_SEC, 0);
		bool no_score = FALSE;
		
		if ( !score || (k!=mode) ) {
			lab = new QLabel(this);
			lab->setFont(f);
			QString name = conf->readEntry(HS_NAME, "");
			no_score = name.isEmpty() || (!min && !sec);
			
			if (no_score) { // no score for this level
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
			qle = new QLineEdit(this);
			qle->setMaxLength(10);
			qle->setFont(f);
			qle->setMinimumSize(qle->fontMetrics().maxWidth()*10,
								qle->sizeHint().height() );
			qle->setFocus();
			connect(qle, SIGNAL(returnPressed()), SLOT(writeName()));
			gl->addWidget(qle, k, 2);
		}

		QString str;
		if (min) {
			if (sec) str = i18n("in %1 minutes and %2 seconds.").arg(min).arg(sec);
			else str = i18n("in %1 minutes.").arg(min);
		} else str = i18n("in %1 seconds.").arg(sec);

		lab = new QLabel(str, this);
		lab->setAlignment(AlignCenter);
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 3);
	}

/* button */
	pb = new QPushButton(i18n("Close"), this);
	pb->setFixedSize( pb->sizeHint() );
	connect(pb, SIGNAL(clicked()), SLOT(accept()));
	if (score) pb->setEnabled(FALSE);
	else pb->setFocus();
	top->addSpacing(2*BORDER);
	top->addWidget(pb);
}

void WHighScores::writeName()
{
	KConfig *conf = kapp->getConfig();
	conf->setGroup(HS_GRP[mode]);
	QString str = qle->text();
	if ( str.length() ) conf->writeEntry(HS_NAME, str);
	
	// show the entered highscore
	str = conf->readEntry(HS_NAME);
	qle->setText(str);
	qle->setEnabled(FALSE);
	qle->clearFocus();

	pb->setEnabled(TRUE);
//	pb->setFocus(); //cannot set focus here ... it quits ...
}
