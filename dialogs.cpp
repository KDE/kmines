#include "dialogs.h"

#include <stdio.h>
#include <stdlib.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qmsgbox.h>
#include <qfileinf.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qfont.h>

#include <kapp.h>
#include <kkeyconf.h>

#include "dialogs.moc"

/*****************/
/* Digital Clock */
DigitalClock::DigitalClock(QWidget *parent)
: QLCDNumber(parent, 0)
{
//	initMetaObject();
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

/********************/
/* Customize dialog */
Custom::Custom(uint *nb_width, uint *nb_height, uint *nb_mines,
			   QWidget *parent)
: QDialog(parent, 0, TRUE)
{
//	initMetaObject();
	
	nb_w = nb_width;
	nb_h = nb_height;
	nb_m = nb_mines;
  
	setCaption(i18n("Custom Level"));

	QFontMetrics fm = this->fontMetrics();
	int frame_w = 7; int frame_h = 7;
	int dec1_h = 5; int dec2_h = 20;
	int dec3_h = 30; int scroll_h = 15;
	int label_h = fm.height() + 2*2;
	int button_h = fm.height() + 2*7;
	int H = 2*frame_h + 3*(label_h + dec1_h + scroll_h) 
		    + 2*dec2_h + dec3_h + button_h;
	
	int text_label_w = QMAX(QMAX(fm.width(i18n("Width :")),
								 fm.width(i18n("Height :"))),
							fm.width(i18n("Mines :")));
	int nb_label_w = 2*fm.maxWidth() + 2*1;
	int nb2_label_w = 5*fm.maxWidth() + fm.width(" ie ") + fm.width("%") + 2*1;
	int button_w = QMAX(fm.width(i18n("Ok")),
						fm.width(i18n("Cancel"))) + 2*7;
	
	int minw_button = 2*button_w + 30;
	int minw_label = text_label_w + nb2_label_w + 30;
	int W = QMAX(minw_button, minw_label) + 2*frame_w;
	
	setFixedSize(W, H);
	
	int temp = frame_h; int dec = label_h+dec1_h;
	QLabel *label;
	ADD_LABEL( i18n("Width :"), frame_w, temp, text_label_w, label_h );
	label->setAlignment( AlignCenter );
	lw = new QLabel(this);
	lw->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lw->setAlignment( AlignCenter );
	lw->setGeometry(W-nb_label_w-frame_w, temp, nb_label_w, label_h);
	lw->setNum((int)*nb_w);
	sw = new QScrollBar(8, 50, 1, 5, *nb_w,QScrollBar::Horizontal, this);
	sw->setGeometry(frame_w, temp+dec, W-2*frame_w, scroll_h);
	connect(sw, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));

	temp += dec + scroll_h+dec2_h; 
	ADD_LABEL( i18n("Height :"), frame_w, temp, text_label_w, label_h);
	label->setAlignment( AlignCenter );
	lh = new QLabel(this);
	lh->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lh->setAlignment( AlignCenter );
	lh->setGeometry(W-nb_label_w-frame_w, temp, nb_label_w, label_h);
	lh->setNum((int)*nb_h);
	sh = new QScrollBar(8, 50, 1, 5, *nb_h,QScrollBar::Horizontal, this);
	sh->setGeometry(frame_w, temp+dec, W-2*frame_w, scroll_h);
	connect(sh, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));

	temp += dec + scroll_h+dec2_h;
	ADD_LABEL( i18n("Mines :"), frame_w, temp, text_label_w, label_h);
	label->setAlignment( AlignCenter );
	lm = new QLabel(this);
	lm->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lm->setAlignment( AlignCenter );
	lm->setGeometry(W-nb2_label_w-frame_w, temp, nb2_label_w, label_h);
	sm = new QScrollBar(1, (*nb_w)*(*nb_h)-1, 1, 5,
						*nb_m,QScrollBar::Horizontal, this);
	sm->setGeometry(frame_w, temp+dec, W-2*frame_w, scroll_h);
	nbminesChanged((int)*nb_m);
	connect(sm, SIGNAL(valueChanged(int)), SLOT(nbminesChanged(int)));

	temp = (W-2*button_w)/3;
	dec = H-frame_h-button_h;
	QPushButton *ok = new QPushButton(this);
	ok->setText(i18n("Ok"));
	ok->setGeometry(temp, dec, button_w, button_h);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));
	QPushButton *cancel = new QPushButton(this);
	cancel->setText(i18n("Cancel"));
	cancel->setGeometry(2*temp+button_w, dec, button_w, button_h);
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	D_OKCANCEL_KEY(K_CUSTOM, this);
}

void Custom::widthChanged(uint new_width)
{
	*nb_w = new_width;
	lw->setNum((int)*nb_w);
	nbminesChanged(*nb_m);
}
  
void Custom::heightChanged(uint new_height)
{
	*nb_h = new_height;
	lh->setNum((int)*nb_h);
	nbminesChanged(*nb_m);
}
  
void Custom::nbminesChanged(uint new_nbmines)
{
	char str[30];
	
	*nb_m = new_nbmines;
	sm->setRange(1, (*nb_w)*(*nb_h)-1);
	sprintf(str, "%d ie %d%%", *nb_m,100*(*nb_m)/((*nb_w)*(*nb_h)));
	lm->setText(str);
}

/*********************/
/* HighScore dialogs */
WHighScores::WHighScores(bool show, int new_sec, int new_min, int mode,
						 int &res,
						 QWidget *parent)
: QDialog(parent, 0, TRUE)
{
//	initMetaObject();
	kconf = kapp->getConfig();
	
//	D_CLOSE_KEY(K_HS, this);
	
	res = showHS(show, new_sec, new_min, mode);
}

int WHighScores::showHS( bool show, int new_sec, int new_min, int mode)
{
	/* set highscore ? */
	if ( !show ) {
		kconf->setGroup(HS_GRP[mode]);
		/* a better time ? */
		int res = kconf->readNumEntry(HS_SEC_KEY) + 60*kconf->readNumEntry(HS_MIN_KEY);
		if ( (new_sec + new_min*60) >= res ) return res;
	}
	
	setCaption(i18n("High Scores"));

	/* set dialog layout */
	int frame_w = 10; int frame_h = 10;
	int dec1_h = 20; int dec2_h = 10; int dec3_h = 20;
	
	QFont f1( font() );
	QFontMetrics fm1( f1 );
	int level_label_w = QMAX(QMAX(fm1.width(HS_GRP[0]), fm1.width(HS_GRP[1])),
							 fm1.width(HS_GRP[2])) +2*3;
	int in_label_w = fm1.width(i18n("in"));
	int minutes_label_w = fm1.width(i18n("minutes and "));
	int seconds_label_w = fm1.width(i18n("seconds."));
	int button_w = fm1.width(i18n("Close")) + 2*15;
	int button_h = fm1.height() + 2*10;
	int space_w = fm1.width(" ");

	f1.setBold(TRUE);
	QFontMetrics fm2(f1);
	int label_h = QMAX(fm1.height(), fm2.height()) + 2*2;
	int nb_label_w = 2*fm2.maxWidth();
	int nb2_label_w = 2*fm2.maxWidth();
	int name_label_w = 10*fm2.maxWidth();
	
	QFont f2( font() );
	QFontInfo info(f2);
	f2.setPointSize(info.pointSize()+6);
	f2.setBold(TRUE);
	QFontMetrics fm3(f2);
	int title_w = fm3.width(i18n("Hall of Fame"));
	int title_h = fm3.height();
	
	int H = 2*frame_h + 4*label_h + dec1_h + 2*dec2_h + dec3_h + button_h;
	int W = 2*frame_w + 9*space_w + level_label_w + name_label_w
		    + in_label_w + nb_label_w + minutes_label_w + nb2_label_w
		    + seconds_label_w;
	setFixedSize(W, H);
	
	QLabel *label;
	ADD_LABEL(i18n("Hall of Fame"), (W-title_w)/2, frame_h, title_w, title_h);
	label->setFont(f2);
	label->setAlignment(AlignCenter);
	
	int temp2;
	int temp = frame_h + title_h + dec1_h;
	int dec = label_h + dec2_h;
	#define HDEC temp+i*dec
	for (int i=0; i<3; i++) {
		temp2 = frame_w;
		kconf->setGroup(HS_GRP[i]);
		ADD_LABEL(HS_GRP[i], temp2, HDEC, level_label_w, label_h);
		label->setFrameStyle( QFrame::Panel | QFrame::Sunken );

		temp2 += level_label_w + 4*space_w;
		if ( show || (i!=mode) ) {
			ADD_LABEL(kconf->readEntry(HS_NAME_KEY), temp2, HDEC,
					  name_label_w, label_h);
			label->setAlignment(AlignCenter);
			label->setFont(f1);
		} else {
			lab = new QLabel(this);
			lab->setGeometry(temp2, HDEC, name_label_w, label_h);
			lab->setFont(f1);
			lab->setAlignment(AlignCenter);
			lab->hide();
			qle = new QLineEdit(this);
			qle->setMaxLength(10);
			qle->setGeometry(temp2, HDEC, name_label_w, label_h);
			qle->setFont(f1); 
			connect(qle, SIGNAL(returnPressed()), SLOT(writeName()));
		}
		
		temp2 += name_label_w + space_w;
		ADD_LABEL(i18n("in"), temp2, HDEC, in_label_w, label_h);
		
		if ( !show && i==mode )
			kconf->writeEntry(HS_MIN_KEY, new_min);
			
		temp2 += in_label_w + space_w;
		if ( kconf->readNumEntry(HS_MIN_KEY)!=0 ) {
			ADD_LABEL(kconf->readEntry(HS_MIN_KEY), temp2, HDEC,
					  nb_label_w, label_h);
			label->setAlignment(AlignCenter);
			label->setFont(f1);
			ADD_LABEL(i18n("minutes and "), temp2+nb_label_w+space_w, HDEC,
					  minutes_label_w, label_h);
		}
		
		if ( !show && i==mode )
			kconf->writeEntry(HS_SEC_KEY, new_sec);

		temp2 += nb_label_w + minutes_label_w + 2*space_w;
		ADD_LABEL(kconf->readEntry(HS_SEC_KEY), temp2, HDEC,
				  nb2_label_w, label_h);
		label->setAlignment(AlignCenter);
		label->setFont(f1);
		temp2 += nb2_label_w + space_w;
		ADD_LABEL(i18n("seconds."), temp2, HDEC, seconds_label_w, label_h);
	}

	temp2 = (W-button_w)/2;
	pb = new QPushButton(this);
	pb->setText(i18n("Close"));
	pb->setGeometry(temp2, temp+2*dec+label_h+dec3_h, button_w, button_h);
	connect(pb, SIGNAL(clicked()), SLOT(accept()));
	
	if ( !show ) {
		pb->hide();
		/* to be in the right mode at save name time */
		kconf->setGroup(HS_GRP[mode]);
	}

	exec();
	return 0;
}

void WHighScores::writeName()
{
	QString str = qle->text();
	if ( str.isNull() )
	    str = i18n("Anonymous");
	
	kconf->writeEntry(HS_NAME_KEY, str);
	
	/* show the entered highscore */
	delete qle;
	lab->setText(str); lab->show();
	pb->show();
}
	
/* Options dialog */
Options::Options(QWidget *parent)
: QDialog(parent, 0, TRUE)
{
//	initMetaObject();
	kconf = kapp->getConfig();
	kconf->setGroup(OP_GRP);
	um = kconf->readNumEntry(OP_UMARK_KEY);
	
	setCaption(i18n("Options"));
	
	QFontMetrics fm = fontMetrics();
	int frame_w = 10; int frame_h = 10;
	int op1_w = fm.width(i18n("Uncertain (?) mark"));
	int label_h = fm.height();
	int yes_w = fm.width(i18n("Yes"));
	int no_w = fm.width(i18n("No"));
	int button_w = fm.width(i18n("Ok")) + 2*10;
	int button_h = fm.height() + 2*7;
	int dec1_w = 10; int dec1_h = 15;
	int dec2_h = 20;
	
	int H = 2*frame_h + 3*label_h + 3*dec1_h + dec2_h + button_h;
	int W = QMAX(QMAX(op1_w, yes_w), no_w) + 2*dec1_w + 2*frame_w;
	setFixedSize(W, H);
	
	QButtonGroup *bg = new QButtonGroup(this);
	bg->setTitle(i18n("Uncertain (?) mark"));
	bg->setGeometry(frame_w, frame_h, W-2*frame_w, 3*label_h+3*dec1_h);
	QRadioButton *rby = new QRadioButton(bg);
	rby->setText(i18n("Yes"));
	rby->setGeometry(dec1_w, label_h+dec1_h, yes_w+20, label_h);
	QRadioButton *rbn = new QRadioButton(bg);
	rbn->setText(i18n("No"));
	rbn->setGeometry(dec1_w, 2*label_h+2*dec1_h, no_w+20, label_h);
	connect(bg, SIGNAL(clicked(int)), SLOT(changeUMark(int)));

	/* show the current value of the uncertain mark */
	if (um) rby->setChecked( TRUE );
	else rbn->setChecked( TRUE );
  
	QPushButton *pb = new QPushButton(this); 
	pb->setText(i18n("Ok"));
	pb->setGeometry((W-button_w)/2, H-frame_h-button_h, button_w, button_h);
	connect(pb, SIGNAL(clicked()), SLOT(accept()));

	D_CLOSE_KEY(K_OP, this);

	exec();
}

void Options::changeUMark(int i)
{
	kconf->writeEntry(OP_UMARK_KEY, !i);
}

/* WReplay dialog */
WReplay::WReplay(const QString &msg1, const QString &msg2,
				 const QPixmap &happy, const QPixmap &ohno,
				 QWidget *parent)
: QDialog(parent, 0, TRUE)
{
	QFontMetrics fm1( font() ); 
	int frame_dec = 10;
	int label_h = fm1.height();
	QLabel *l; QButton *b;
	
	int label1_w = fm1.width(msg1);
	int label1_h = 2*label_h+frame_dec;
	if ( !msg2.isNull() ) label1_w = QMAX(label1_w, fm1.width(msg2));
	label1_w += frame_dec;
	
	QString msgt = i18n("Try again");
	QString msgq = i18n("Quit");
	int button_w = happy.width();
	int button_h = happy.height();
	int label2_w = fm1.width(msgt);
	int label3_w = fm1.width(msgq);
	
	int wdec = QMAX(button_w, QMAX(label2_w,label3_w));
	int W = QMAX(2*wdec+frame_dec, label1_w) + 2*frame_dec;
	int H = 9*frame_dec/2 + label1_h + button_h + label_h;
	int wdec2 = (W - 2*wdec)/3;
	resize(W,H);
	
	QString msg = msg1; 
	if ( !msg2.isNull() ) { msg += "\n"; msg += msg2; }
	l = new QLabel(msg, this);
	l->setGeometry((W-label1_w)/2, frame_dec, label1_w, label1_h);
	l->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	
	b = new QPushButton(this);
	connect(b, SIGNAL(clicked()), SLOT(accept()));
	b->setGeometry(wdec2+(wdec-button_w)/2, 3*frame_dec+label1_h,
				   button_w, button_h);
	b->setPixmap(happy);
	
	b = new QPushButton(this);
	connect(b, SIGNAL(clicked()), SLOT(reject()));
	b->setGeometry(2*wdec2+(wdec-button_w)/2+wdec, 3*frame_dec+label1_h,
				   button_w, button_h);
	b->setPixmap(ohno);
	
	l = new QLabel(msgt, this);
	l->setGeometry(wdec2+(wdec-label2_w)/2, 7*frame_dec/2+label1_h+button_h,
				   label2_w, label_h);
	l = new QLabel(msgq, this);
	l->setGeometry(2*wdec2+(wdec-label3_w)/2+wdec, 7*frame_dec/2+label1_h+button_h,
				   label3_w, label_h);
}
