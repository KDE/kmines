#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qdialog.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qlayout.h>

#include <kconfig.h>

/**** digital clock widget ***************************************************/
class DigitalClock : public QLCDNumber
{
 Q_OBJECT
	
 public:
	DigitalClock(QWidget *parent);
	
	int sec() const { return time_sec; }
	int min() const { return time_min; }
	
 protected:
	void timerEvent( QTimerEvent * );
	
 public slots:
	void zero();
	void freeze();
	void start();
	
 private:
	int  time_sec, time_min;
	int  stop;      /* state */
	
	void showTime();
};

/**** custom dialog **********************************************************/
class CustomDialog : public QDialog
{
 Q_OBJECT
	  
 public:
	CustomDialog(uint *w, uint *h, uint *nb, QWidget *parent);
  
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










