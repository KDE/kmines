#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qdialog.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qlayout.h>
#include <kdialog.h>

#include "defines.h"

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
	CustomDialog(Level &lev, QWidget *parent);
  
 private slots:
	void widthChanged(int);
	void heightChanged(int);
	void nbMinesChanged(int);
	
 signals:
	void setWidth(int);
	void setHeight(int);
	void setNbMines(const QString &);
  
 private:
	QScrollBar *sm;
	Level      *lev;
};


/**** highscores dialog ******************************************************/
class WHighScores : public KDialog
{
 Q_OBJECT
	  
 public:
	WHighScores(bool show, int sec, int min, uint mode, bool *res,
				QWidget *parent);

 private slots:
	void writeName();
  
 private:
	uint mode;
	QLineEdit   *qle;
	QGridLayout *gl;
	QPushButton *pb;
};

#endif // DIALOGS_H
