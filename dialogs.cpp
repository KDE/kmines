#include <qmsgbox.h>
#include "dialogs.h"

#include <stdio.h>
#include <stdlib.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qfileinf.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qfont.h>

#include <kapp.h>
#include <kkeyconf.h>

#include "dialogs.moc"

/* Digital Clock */

DigitalClock::DigitalClock( QWidget *parent, const char *name )
: QLCDNumber( parent, name )
{
	initMetaObject();
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

/* Customize dialog */
Custom::Custom( int *nb_width, int *nb_height, int *nb_mines,
			    QWidget *parent, const char *name )
: QDialog( parent, name, TRUE)
{
	initMetaObject();
	
	nb_w = nb_width;
	nb_h = nb_height;
	nb_m = nb_mines;
  
	setCaption("Custom Level");
  
	QLabel *tw, *th, *tm;
  
	tw = new QLabel(this);
	tw->setGeometry(5,10,LABEL_W,LABEL_H);
	tw->setAlignment( AlignCenter );
	tw->setText("Width :");
	lw = new QLabel(this);
	lw->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lw->setAlignment( AlignCenter );
	lw->setGeometry(200-LABEL_W-5,10,LABEL_W,LABEL_H);
	lw->setNum(*nb_w);
	sw = new QScrollBar(8,50,1,5,*nb_w,QScrollBar::Horizontal,this);
	sw->setGeometry(5,40,200-2*5,15);  
	connect( sw,   SIGNAL(valueChanged(int)),
			 this, SLOT(widthChanged(int)) );
  
	th = new QLabel(this);
	th->setGeometry(5,70,LABEL_W+5,LABEL_H);
	th->setAlignment( AlignCenter );
	th->setText("Height :");
	lh = new QLabel(this);
	lh->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lh->setAlignment( AlignCenter );
	lh->setGeometry(200-LABEL_W-5,70,LABEL_W,LABEL_H);
	lh->setNum(*nb_h);
	sh = new QScrollBar(8,50,1,5,*nb_h,QScrollBar::Horizontal,this);
	sh->setGeometry(5,100,200-2*5,15);
	connect( sh,   SIGNAL(valueChanged(int)),
			 this, SLOT(heightChanged(int)) );
  
	tm = new QLabel(this);
	tm->setGeometry(5,130,LABEL_W,LABEL_H);
	tm->setAlignment( AlignCenter );
	tm->setText("Mines :");
	lm = new QLabel(this);
	lm->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lm->setAlignment( AlignCenter );
	lm->setGeometry(200-2*LABEL_W-5,130,2*LABEL_W,LABEL_H);
	sm = new QScrollBar(1,(*nb_w)*(*nb_h)-1,1,5,*nb_m,QScrollBar::Horizontal,this);
	sm->setGeometry(5,160,200-2*5,15);  nbminesChanged(*nb_m);
	connect( sm,   SIGNAL(valueChanged(int)),
			 this, SLOT(nbminesChanged(int)) );
					  
	QPushButton *ok, *cancel;
	ok = new QPushButton(this);
	ok->setText("Ok");
	ok->setGeometry( 10, 200, 80,30 );
	connect( ok, SIGNAL(clicked()), SLOT(accept()) );
	cancel = new QPushButton(this);
	cancel->setText("Cancel");
	cancel->setGeometry( 120, 200, 80,30 );
	connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

	D_OKCANCEL_KEY("custom", this);
	
	adjustSize();
	setFixedSize(width(), height());
}
  

void Custom::widthChanged(int new_width)
{
	*nb_w = new_width;
	lw->setNum(*nb_w);
	nbminesChanged(*nb_m);
}
  
  
void Custom::heightChanged(int new_height)
{
	*nb_h = new_height;  lh->setNum(*nb_h);
	nbminesChanged(*nb_m);
}
  
  
void Custom::nbminesChanged(int new_nbmines)
{
	char str[30];
	
	*nb_m = new_nbmines;
	sm->setRange(1,(*nb_w)*(*nb_h)-1);
	sprintf(str,"%d ie %d%%",*nb_m,100*(*nb_m)/((*nb_w)*(*nb_h)));
	lm->setText(str);
}
  

/* HighScore dialogs */
WHighScores::WHighScores( bool show, int new_sec, int new_min, int mode,
						  QWidget *parent, const char *name)
: QDialog(parent, name, TRUE)
{
	initMetaObject();
	kconf = kapp->getConfig();
	
//	D_CLOSE_KEY("highscores", this);
	
	showHS(show, new_sec, new_min, mode);
}

void WHighScores::showHS( bool show, int new_sec, int new_min, int mode)
{
	/* set highscore ? */
	if ( !show ) {
		kconf->setGroup(HS_GRP[mode]);
		/* a better time ? */
		if ( (new_sec + new_min*60) >= (kconf->readNumEntry(HS_SEC_KEY) 
				                        + kconf->readNumEntry(HS_MIN_KEY)*60) )
			return;
	}
	
	setCaption("High Scores");
	
	QLabel *label;
	ADD_LABEL( "Hall Of Fame", 104, 15, 160, 20);
	label->setFont( QFont("times", 18, QFont::Bold) );
	label->setAlignment( AlignCenter );
	
	#define H 55+40*i
	
	for (int i=0; i<3; i++) {
		kconf->setGroup(HS_GRP[i]);
		ADD_LABEL( HS_GRP[i], 5, H, 80, LABEL_H );
		label->setFont( QFont("times", 13) );
		ADD_LABEL( ":", 90, H, 5, LABEL_H);

		if ( show || (i!=mode) ) {
			ADD_LABEL( kconf->readEntry(HS_NAME_KEY), 105, H, 75, LABEL_H );
			label->setFont( QFont("times", 13, QFont::Bold) );
			label->show();
		} else {
			lab = new QLabel(this);
			lab->setGeometry(105, H, 75, LABEL_H);
			lab->setFont( QFont("times", 13, QFont::Bold) );
			lab->hide();
			qle = new QLineEdit(this);
			qle->setMaxLength(10);
			qle->setGeometry(105, H, 75, LABEL_H);
			qle->setFont( QFont("times", 13, QFont::Bold) ); 
			connect( qle,  SIGNAL(returnPressed()),
					 this, SLOT(writeName()) );
		}
		
		ADD_LABEL( "in", 188, H, 20, LABEL_H );
		
		if ( !show && i==mode )
			kconf->writeEntry(HS_MIN_KEY, new_min);
			
		if ( kconf->readNumEntry(HS_MIN_KEY)!=0 ) {
			ADD_LABEL(kconf->readEntry(HS_MIN_KEY), 200, H, 20, LABEL_H);
			label->setAlignment( AlignRight | AlignVCenter );
			label->setFont( QFont("times", 13, QFont::Bold) );
			ADD_LABEL( "minutes and ", 225, H, 70, LABEL_H);
		}
		
		if ( !show && i==mode )
			kconf->writeEntry(HS_SEC_KEY, new_sec);
		
		ADD_LABEL(kconf->readEntry(HS_SEC_KEY), 293, H, 20, LABEL_H);
		label->setAlignment( AlignRight | AlignVCenter );
		label->setFont( QFont("times", 13, QFont::Bold) );
		ADD_LABEL( "seconds.", 318, H, 50, LABEL_H);
	}

	pb = new QPushButton(this);
	pb->setText("Close");
	pb->setGeometry(134, 175, 100, 30);
	connect( pb ,  SIGNAL(clicked()),
			 this, SLOT(accept()) );
	
	if ( !show ) {
		pb->hide();
		/* to be in the right mode at save name time */
		kconf->setGroup(HS_GRP[mode]);
	}
	
	adjustSize();
	setFixedSize(width(), height());
	exec();
}

void WHighScores::writeName()
{
	QString str = qle->text();
	if ( str.isNull() )
	    str = "Anonymous";
	
	kconf->writeEntry(HS_NAME_KEY, str);
	
	/* show the entered highscore */
	delete qle;
	lab->setText(str); lab->show();
	pb->show();
}
	
/* Options dialog */
Options::Options( QWidget *parent, const char *name)
: QDialog(parent, name, TRUE)
{
	initMetaObject();
	kconf = kapp->getConfig();
	kconf->setGroup(OP_GRP);
	um = kconf->readNumEntry(OP_UMARK_KEY);
	
	setCaption("Options");
	
	QButtonGroup *bg = new QButtonGroup(this);
	bg->setTitle("Uncertain (?) mark");
	QRadioButton *rby = new QRadioButton( bg );
	rby->setText( "Yes" );
	rby->setGeometry( 10, 15, 40, 25 );
	QRadioButton *rbn = new QRadioButton( bg );
	rbn->setText( "No" );
	rbn->setGeometry( 10, 45, 40, 25 );
	bg->setGeometry( 10, 40, 150, 80 );
	connect( bg, SIGNAL(clicked(int)), SLOT(changeUMark(int)) );

	/* show the current value of the uncertain mark */
	if (um)
		rby->setChecked( TRUE );
	else
		rbn->setChecked( TRUE );
  
	QPushButton *pb = new QPushButton(this); 
	pb->setText("Ok");
	pb->setGeometry(60,130,50,30);      
	connect( pb,   SIGNAL(clicked()), SLOT(accept()) );

	D_CLOSE_KEY("options", this);
	
	resize(170,170);
	setFixedSize(width(), height());
	exec();
}

void Options::changeUMark(int i)
{
	kconf->writeEntry(OP_UMARK_KEY, !i);
}
