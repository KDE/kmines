#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qdialog.h>
#include <qscrbar.h>
#include <qlined.h>
#include <qlcdnum.h>
#include <qlayout.h>

#include <kconfig.h>

/**** digital clock widget ***************************************************/
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

/**** custom dialog **********************************************************/
class Custom : public QDialog
{
 Q_OBJECT
	  
 public:
	Custom(uint *w, uint *h, uint *nb, QWidget *parent);
  
 private slots:
	void widthChanged(int);
	void heightChanged(int);
	void nbMinesChanged(int);
  
 private:
	QScrollBar *sw, *sh, *sm;
	QLabel *lw, *lh, *lm;
  
	uint  *nbW, *nbH, *nbM;
};


/**** highscores dialog ******************************************************/
class WHighScores : public QDialog
{
 Q_OBJECT
	  
 public:
	WHighScores(bool show, int sec, int min, uint mode, int &res,
				QWidget *parent);

 private slots:
	void writeName();
  
 private:
	uint mode;
	KConfig *kconf;
	QLineEdit *qle;
	QPushButton *pb;
	QGridLayout *gl;
	QVBoxLayout *top;
};

#endif // DIALOGS_H
