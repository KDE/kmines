#include "dialogs.h"
#include "defines.h"
#include "version.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qfont.h>

#include "dialogs.moc"


#define BORDER   10
#define TBORDER  5
#define PS_INC    4

/* useful function */
void pSetCaption(const char *text, QWidget *w)
{
	
	    QString tmp = KMINES_NAME;
	    tmp += " : ";
	    tmp += text;
	    w->setCaption(tmp);
}

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
			time_min++; time_sec = 0;
		}
		/* so waiting one hour don't do a restart timer at 00:00 */
		if (time_min==60)
		    time_min = 60;
		showTime();
	}
}

void DigitalClock::showTime()
{
	static char s[6];
	
	s[0] = '0'; s[1] = '0'; s[2] = ':'; s[3] = '0'; s[4] = '0'; s[5] = 0;
	if (time_min>=10) 
	    s[0] += time_min / 10;
	s[1] += time_min % 10;
	if (time_sec>=10) 
	    s[3] += time_sec / 10;
	s[4] += time_sec % 10;
	
	display( s );
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

void DigitalClock::getTime(int  *t_sec, int *t_min)
{
	*t_sec = time_sec;
	*t_min = time_min;
}

/** Customize dialog *********************************************************/
Custom::Custom(uint *nbWidth, uint *nbHeight, uint *nbMines,
			   QWidget *parent)
: QDialog(parent, 0, TRUE), nbW(nbWidth), nbH(nbHeight), nbM(nbMines)
{
	QString tmp = i18n("Customize Game");
	pSetCaption(tmp, this);

/* top layout */
	QVBoxLayout *top = new QVBoxLayout(this, BORDER);
	
/* title */
	QLabel *title = new QLabel(tmp, this);
	QFont f( title->font() );
	QFontInfo fi(f);
	f.setPointSize(fi.pointSize()+PS_INC);
	f.setBold(TRUE);
	title->setFont(f);
	title->setAlignment(AlignCenter);
	title->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	title->setFixedSize( title->sizeHint()+QSize(2*TBORDER, TBORDER) );
	title->setBackgroundMode( QWidget::PaletteMidlight );
	top->addWidget(title);
	top->addSpacing(2*BORDER); // some additional space
	top->addStretch(1);

	QHBoxLayout *hbl;
	QLabel *lab;

/* Width */
	/* labels */
	hbl = new QHBoxLayout(BORDER);
	top->addLayout(hbl);
	
	lab = new QLabel(i18n("Width"), this);
	lab->setFixedSize( lab->sizeHint() );
	hbl->addWidget(lab);
	hbl->addStretch(1);
	
	lw = new QLabel(this);
	lw->setNum((int)*nbW);
	lw->setAlignment( AlignRight );
	lw->setFixedSize( lw->fontMetrics().maxWidth()*2, lw->sizeHint().height());
	hbl->addWidget(lw);

	/* scrollbar */
	sw = new QScrollBar(8, 50, 1, 5, *nbW,QScrollBar::Horizontal, this);
	sw->setMinimumWidth( sw->sizeHint().width() );
	sw->setFixedHeight( sw->sizeHint().height() );
	connect(sw, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
	top->addWidget(sw);
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
	
	lh = new QLabel(this);
	lh->setNum((int)*nbH);
	lh->setAlignment( AlignRight );
	lh->setFixedSize( lh->fontMetrics().maxWidth()*2, lh->sizeHint().height());
	hbl->addWidget(lh);

	/* scrollbar */
	sh = new QScrollBar(8, 50, 1, 5, *nbH ,QScrollBar::Horizontal, this);
	sh->setMinimumWidth( sh->sizeHint().width() );
	sh->setFixedHeight( sh->sizeHint().height() );
	connect(sh, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
    top->addWidget(sh);
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
	
	lm = new QLabel(" ", this);
	lm->setAlignment( AlignRight );
	lm->setFixedSize(lm->fontMetrics().maxWidth()*11, lm->sizeHint().height());
	hbl->addWidget(lm);

	/* scrollbar */
	sm = new QScrollBar(1, (*nbW)*(*nbH), 1, 5, *nbM,
						QScrollBar::Horizontal, this);
	sm->setMinimumWidth( sm->sizeHint().width() );
	sm->setFixedHeight( sm->sizeHint().height() );
	connect(sm, SIGNAL(valueChanged(int)), SLOT(nbMinesChanged(int)));
	top->addWidget(sm);
	top->addSpacing(2*BORDER);
	top->addStretch(1);

	nbMinesChanged((int)*nbM);
	
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
	
	top->activate();
	resize(0, 0); // get the minimum size
}

void Custom::widthChanged(int newWidth)
{
	*nbW = (uint)newWidth;
	lw->setNum(newWidth);
	nbMinesChanged(*nbM);
}
  
void Custom::heightChanged(int newHeight)
{
	*nbH = (uint)newHeight;
	lh->setNum(newHeight);
	nbMinesChanged(*nbM);
}
  
void Custom::nbMinesChanged(int newNbMines)
{
	char str[30];
	
	*nbM = (uint)newNbMines;
	sm->setRange(1, (*nbW)*(*nbH)-2);
	sprintf(str, "%d (%d%%)", newNbMines, 100*newNbMines/((*nbW)*(*nbH)));
	lm->setText(str);
}

/** HighScore dialog *********************************************************/
WHighScores::WHighScores(bool show, int newSec, int newMin, uint Mode,
						 int &res, QWidget *parent)
: QDialog(parent, 0, TRUE), mode(Mode)
{
	kconf = kapp->getConfig();

	/* set highscore ? */
	if ( !show ) {
		kconf->setGroup(HS_GRP[mode]);
		/* a better time ? */
		int res = kconf->readNumEntry(HS_SEC_KEY)
			      + 60*kconf->readNumEntry(HS_MIN_KEY);
		if ( (newSec + newMin*60) >= res ) return;
	}

	QString tmp = i18n("Hall of Fame");
	pSetCaption(tmp, this);

/* top layout */
	top = new QVBoxLayout(this, BORDER);
	
/* title */
	QLabel *title = new QLabel(tmp, this);
	QFont f( title->font() );
	QFontInfo fi(f);
	f.setPointSize(fi.pointSize()+PS_INC);
	f.setBold(TRUE);
	title->setFont(f);
	title->setAlignment(AlignCenter);
	title->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	title->setFixedSize( title->sizeHint()+QSize(2*TBORDER, TBORDER) );
	title->setBackgroundMode( QWidget::PaletteMidlight );
	top->addWidget(title);
	top->addSpacing(2*BORDER); // some additional space

	QLabel *lab;
	f = font();
	f.setBold(TRUE);
	
/* Grid layout */
	gl = new QGridLayout(3, 9, BORDER);
	top->addLayout(gl);

	/* level names */
	for(uint k=0; k<3; k++) {
		if ( k==0 ) tmp = i18n("Easy");
		else if ( k==1 ) tmp = i18n("Normal");
		else tmp = i18n("Expert");
		tmp += " :";
		lab = new QLabel(tmp, this);
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 0);

		kconf->setGroup(HS_GRP[k]);
		
		if ( show || (k!=mode) ) {
			lab = new QLabel(kconf->readEntry(HS_NAME_KEY), this);
			lab->setAlignment(AlignCenter);
			lab->setFont(f);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 2);
		} else {
			qle = new QLineEdit(this);
			qle->setMaxLength(10);
			qle->setFont(f);
			qle->setMinimumSize(qle->fontMetrics().maxWidth()*10,
								qle->sizeHint().height() );
			connect(qle, SIGNAL(returnPressed()), SLOT(writeName()));
			gl->addWidget(qle, k, 2);
			
			kconf->writeEntry(HS_MIN_KEY, newMin);
			kconf->writeEntry(HS_SEC_KEY, newSec);
		}
		
		lab = new QLabel(i18n("in"), this);
		lab->setAlignment(AlignCenter);
		lab->setMinimumSize( lab->sizeHint() );
		gl->addWidget(lab, k, 3);
		
		int min = kconf->readNumEntry(HS_MIN_KEY);
		int sec = kconf->readNumEntry(HS_SEC_KEY);
		
		if (min) {
			lab = new QLabel(kconf->readEntry(HS_MIN_KEY), this);
			lab->setFont(f);
			lab->setAlignment(AlignCenter);
			lab->setMinimumSize( lab->sizeHint() );
			gl->addWidget(lab, k, 4);
			
			tmp = i18n("minutes");
			if ( sec==0 ) tmp += '.';
			lab = new QLabel(tmp, this);
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
			
			lab = new QLabel(kconf->readEntry(HS_SEC_KEY), this);
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
	top->addSpacing(2*BORDER);

/* button */
	QHBoxLayout *hbl = new QHBoxLayout(BORDER);
	top->addLayout(hbl);
	pb = new QPushButton(i18n("Close"), this);
	pb->setFixedSize( pb->sizeHint() );
	connect(pb, SIGNAL(clicked()), SLOT(accept()));
	hbl->addStretch(1);
	hbl->addWidget(pb);
	hbl->addStretch(1);

	if ( !show ) pb->hide();

	top->activate();
	resize(0, 0);
	
	exec();
	res = 0;
}

void WHighScores::writeName()
{
	QString str = qle->text();
	if ( str.isNull() ) str = i18n("Anonymous");
	
	kconf->setGroup(HS_GRP[mode]);
	kconf->writeEntry(HS_NAME_KEY, str);
	
	/* show the entered highscore */
	qle->hide();
	QFont f( font() );
	f.setBold(TRUE);
	QLabel *lab = new QLabel(str, this);
	lab->setFont(f);
	lab->setAlignment(AlignCenter);
	lab->setMinimumSize( lab->sizeHint() );
	gl->addWidget(lab, mode, 2);
	top->activate();
	lab->show();

	pb->show();
}
