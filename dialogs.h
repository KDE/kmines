#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <kdialogbase.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kcolorbtn.h>

#include "defines.h"

//-----------------------------------------------------------------------------
class Smiley : public QPushButton
{
 Q_OBJECT

 public:
    Smiley(QWidget *parent, const char *name = 0);
	enum Mood { Normal, Stressed, Happy, Sad };

 public slots:
    void setMood(Smiley::Mood);

 private:
	QPixmap normal, stressed, happy, sad;
};

//-----------------------------------------------------------------------------
class LCDNumber : public QLCDNumber
{
 Q_OBJECT

 public:
	LCDNumber(QWidget *parent, const char *name = 0);
	void setState(bool state);

 private:
	bool state;
};

//-----------------------------------------------------------------------------
class DigitalClock : public LCDNumber
{
 Q_OBJECT
	
 public:
	DigitalClock(QWidget *parent, const char *name = 0);
	
	uint sec() const { return _sec; }
	uint min() const { return _min; }
	bool better() const { return ( toSec(_sec, _min)<max_secs ); }
	
	static uint toSec(uint sec, uint min) { return sec + min*60; }
	void setMaxTime(uint totalSecs) { max_secs = totalSecs; }
	
 protected:
	void timerEvent( QTimerEvent * );
	
 public slots:
	void zero();
	void freeze() { stop = true;  }
	void start()  { stop = false; }
	
 private:
	uint _sec, _min, max_secs;
	bool stop;
	
	void showTime();
};

//-----------------------------------------------------------------------------
class CustomDialog : public KDialogBase
{
 Q_OBJECT
	
 public:
	CustomDialog(Level &lev, QWidget *parent);

	static uint maxNbMines(uint width, uint height);

 private slots:
	void widthChanged(int);
	void heightChanged(int);
	void nbMinesChanged(int);

 private:
	KIntNumInput *km;
	Level        &lev, initLev;

	void updateNbMines();
};

//-----------------------------------------------------------------------------
class WHighScores : public KDialogBase
{
 Q_OBJECT
	
 public:
	WHighScores(QWidget *parent, const Score *score = 0);
	static uint time(GameType);

 private slots:
	void reject();

 private:
	GameType   type;
	QLineEdit *qle;
	bool       _close;
};

//-----------------------------------------------------------------------------
class OptionDialog : public KDialogBase
{
 Q_OBJECT

 public:
	OptionDialog(QWidget *parent);

	static CaseProperties readCaseProperties();
	static bool readUMark();
	static bool readKeyboard();
	static Level readLevel();
	static void writeLevel(const Level &);
	static bool readMenuVisible();
	static void writeMenuVisible(bool visible);
	static MouseAction readMouseBinding(MouseButton);

 private slots:
	void accept();

 private:
	KIntNumInput *ni;
	QCheckBox    *um, *keyb;
	QComboBox    *cb[3];
	KColorButton *flagButton, *explosionButton, *errorButton;
	QArray<KColorButton *> numberButtons;

	static KConfig *config();
	static uint readCaseSize();
	static QColor readColor(const QString & key, QColor defaultColor);
	
	void mainPage();
	void casePage();
	void slotDefault();
};

#endif // DIALOGS_H
