#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <kdialogbase.h>
#include <knuminput.h>
#include <kconfig.h>

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
	void freeze() { stop = TRUE; }
	void start()  { stop = FALSE; }
	
 private:
	uint _sec, _min, max_secs;
	bool stop;
	
	void showTime();
};

//-----------------------------------------------------------------------------
class DialogBase : public KDialogBase
{
 Q_OBJECT

 public:
    DialogBase(const QString &caption, int buttonMask,
			   ButtonCode defaultButton,
			   QWidget *parent, const char *name = 0);

 protected:
	QVBoxLayout *top;
};

//-----------------------------------------------------------------------------
class CustomDialog : public DialogBase
{
 Q_OBJECT
	
 public:
	CustomDialog(Level &lev, QWidget *parent);

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
class WHighScores : public DialogBase
{
 Q_OBJECT
	
 public:
	WHighScores(QWidget *parent, const Score *score = 0);
	static uint time(GameType);

 private slots:
	void writeName();
	void reject();

 private:
	GameType   type;
	QLineEdit *qle;
};

//-----------------------------------------------------------------------------
class OptionDialog : public DialogBase
{
 Q_OBJECT

 public:
	OptionDialog(QWidget *parent);

	static uint readCaseSize();
	static bool readUMark();
	static bool readKeyboard();
	static GameType readLevel();
	static void writeLevel(GameType);
	static bool readMenuVisible();
	static void writeMenuVisible(bool visible);
	static MouseAction readMouseBinding(MouseButton);

 private slots:
	void accept();

 private:
	KIntNumInput *ni;
	QCheckBox    *um, *keyb;
	QComboBox    *cb[3];

	static KConfig *config();
};

#endif // DIALOGS_H
