#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
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
	
	uint sec() const { return _sec; }
	uint min() const { return _min; }
	bool better() const { return ( toSec(_sec, _min)<max_secs ); }
	
	static uint toSec(uint sec, uint min) { return sec + min*60; }
	void setMaxTime(uint totalSecs) { max_secs = totalSecs; }
	
 protected:
	void timerEvent( QTimerEvent * );
	
 public slots:
	void zero();
	void freeze() { stop = TRUE; }
	void start()  { stop = FALSE; }
	
 private:
	uint _sec, _min, max_secs;
	bool stop;
	
	void showTime();
};

/**** custom dialog **********************************************************/
class CustomDialog : public KDialog
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
	WHighScores(QWidget *parent, const Score *score = 0);

	static uint time(uint mode);

 private slots:
	void writeName();
  
 private:
	uint         mode;
	QLineEdit   *qle;
	QPushButton *pb;
};

#endif // DIALOGS_H
