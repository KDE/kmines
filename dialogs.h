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
	DigitalClock( QWidget *parent=0, const char *name=0 );
	
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
	Custom( int *, int *, int *,
		    QWidget *parent=0, const char *name=0 );
  
 private slots:
	void widthChanged(int);
	void heightChanged(int);
	void nbminesChanged(int);
  
 private:
	QScrollBar *sw, *sh, *sm;
	QLabel *lw, *lh, *lm;
  
	int  *nb_w, *nb_h, *nb_m;
};


/* highscores dialog */
class WHighScores : public QDialog
{
 Q_OBJECT
	  
 public:
	WHighScores( bool, int, int, int,
				 QWidget *parent=0, const char *name=0);

 private slots:
	void writeName();
  
 private:
	KConfig *kconf;
	QLineEdit *qle;
	QLabel *lab;
	QPushButton *pb;
	
	void showHS(bool,int,int,int);
};

/* options dialog */
class Options : public QDialog
{
 Q_OBJECT 
  
 public:
	Options(QWidget *parent=0, const char *name=0);
	
 private slots:
	void changeUMark(int);
  
 private:
	int um;
	KConfig *kconf;
};


#endif
