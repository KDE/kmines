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
: QLCDNumber(parent, 0)
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
}

void DigitalClock::timerEvent( QTimerEvent *)
{
 	if (!stop) {
		time_sec++;
		if (time_sec==60) {
			/* so waiting one hour don't do a restart timer at 00:00 */
			if ( time_min<60 ) time_min++;
			time_sec = 0;
		}
		showTime();
	}
}

void DigitalClock::showTime()
{
	static char s[6];
	
	s[0] = '0'; s[1] = '0'; s[2] = ':'; s[3] = '0'; s[4] = '0'; s[5] = 0;
	if (time_min>=10) s[0] += time_min / 10;
	s[1] += time_min % 10;
	if (time_sec>=10) s[3] += time_sec / 10;
	s[4] += time_sec % 10;
	
	display(s);
}

void DigitalClock::zero()
{
	killTimers();
	
	stop = TRUE;
	time_sec = 0; time_min = 0;
	startTimer(1000);
  
	showTime();
}

void DigitalClock::start()
{
	stop = FALSE;
}

void DigitalClock::freeze()
{
	stop = TRUE;
}

/** Customize dialog *********************************************************/
CustomDialog::CustomDialog(Level &_lev, QWidget *parent)
: QDialog(parent, 0, TRUE), lev(&_lev)
{
	QLabel      *lab;
	QScrollBar  *scb;
	QHBoxLayout *hbl;
		
	QString str = i18n("Customize Game");
	setCaption(i18n("kmines: %1").arg(str));

/* top layout */
	QVBoxLayout *top = new QVBoxLayout(this, BORDER);
	
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
	emit setNbMines(i18n("%1 (%2%%)").arg(n).arg(100*n/nb));
}

/** HighScore dialog *********************************************************/
WHighScores::WHighScores(bool show, int newSec, int newMin, uint Mode,
						 bool *res, QWidget *parent)
: KDialog(parent, 0, TRUE), mode(Mode)
{
	KConfig *conf = kapp->getConfig();
	conf->setGroup(HS_GRP[mode]);

	/* set highscore ? */
	if ( !show ) {
		ASSERT( newSec || newMin );
		int r = conf->readNumEntry(HS_SEC, 59)
			    + 60*conf->readNumEntry(HS_MIN, 59);
		bool better = ( (newSec + newMin*60)<r );
		if (res) *res = better;
		if ( !better ) return;
		
		conf->writeEntry(HS_NAME, i18n("Anonymous")); // default
		conf->writeEntry(HS_MIN, newMin);
		conf->writeEntry(HS_SEC, newSec);
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
	gl = new QGridLayout(3, 9, BORDER);
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
		
		if ( show || (k!=mode) ) {
			lab = new QLabel(this);
			lab->setFont(f);
			QString name = conf->readEntry(HS_NAME, "");
			no_score = name.isEmpty() || (!min && !sec);
			
			if (no_score) { // no score for this level
				lab->setText(i18n("no score for this level"));
				lab->setMinimumSize( lab->sizeHint() );
				gl->addMultiCellWidget(lab, k, k, 2, 8);
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

		if ( !no_score ) {
			lab = new QLabel(i18n("in"), this);
			lab->setAlignment(AlignCenter);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 3);
		}
		
		if (min) {
			lab = new QLabel(conf->readEntry(HS_MIN), this);
			lab->setFont(f);
			lab->setAlignment(AlignCenter);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 4);
			
			str = i18n("minutes");
			if ( sec==0 ) str += '.';
			lab = new QLabel(str, this);
			lab->setAlignment(AlignCenter);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 5);
		}	
			
		if (sec) {
			if (min) {
				lab = new QLabel(i18n("and"), this);
				lab->setAlignment(AlignCenter);
				lab->setMinimumSize( lab->sizeHint() );
				gl->addWidget(lab, k, 6);
			}
			
			lab = new QLabel(conf->readEntry(HS_SEC), this);
			lab->setFont(f);
			lab->setAlignment(AlignCenter);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 7);
			
			lab = new QLabel(i18n("seconds."), this);
			lab->setAlignment(AlignCenter);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 8);
		}	
	}

/* button */
	pb = new QPushButton(i18n("Close"), this);
	pb->setFixedSize( pb->sizeHint() );
	connect(pb, SIGNAL(clicked()), SLOT(accept()));
	if (show) pb->setFocus();
	else pb->setEnabled(FALSE);
	top->addSpacing(2*BORDER);
	top->addWidget(pb);
	
	exec();
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
	qle->setMinimumSize( qle->fontMetrics().boundingRect(str).width(),
						 qle->sizeHint().height() );

	pb->setEnabled(TRUE);
	pb->setFocus(); //cannot set focus here ... it quits ...
}
