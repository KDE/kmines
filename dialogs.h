#ifndef KMINES_DI_H
#define KMINES_DI_H

#include <qlabel.h>
#include <qdialog.h>
#include <qscrbar.h>
#include <qlined.h>
#include <qlcdnum.h>

#include <kconfig.h>

#include "defines.h"

/* digital clock widget */
class DigitalClock : public QLCDNumber
{
 Q_OBJECT
	
 public:
	DigitalClock(QWidget *parent);
	
 protected:
	void timerEvent( QTimerEvent * );
	
 public slots:
	void zero();
	void freeze();
	void start();
	void getTime(int *, int *);
	
 private:
	int  time_sec, time_min;
	int  stop;      /* state */
	
	void showTime();
};

/* custom dialog */
class Custom : public QDialog
{
 Q_OBJECT
	  
 public:
	Custom(uint *w, uint *h, uint *nb, QWidget *parent);
  
 private slots:
	void widthChanged(uint);
	void heightChanged(uint);
	void nbminesChanged(uint);
  
 private:
	QScrollBar *sw, *sh, *sm;
	QLabel *lw, *lh, *lm;
  
	uint  *nb_w, *nb_h, *nb_m;
};


/* highscores dialog */
class WHighScores : public QDialog
{
 Q_OBJECT
	  
 public:
	WHighScores(bool show, int sec, int min, int mode, int &res,
				QWidget *parent);

 private slots:
	void writeName();
  
 private:
	KConfig *kconf;
	QLineEdit *qle;
	QLabel *lab;
	QPushButton *pb;
	
	int showHS(bool,int,int,int);
};

/* options dialog */
class Options : public QDialog
{
 Q_OBJECT 
  
 public:
	Options(QWidget *parent);
	
 private slots:
	void changeUMark(int);
  
 private:
	int um;
	KConfig *kconf;
};

/* Replay dialog */
class WReplay : public QDialog
{
 public:
	WReplay(const QString &msg1, const QString &msg2,
			const QPixmap &happy, const QPixmap &ohno,
			QWidget *parent);	
};
		
#endif
