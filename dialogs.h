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
#include <klocale.h>

#include "defines.h"
#include "highscores.h"

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

	void setColor(QColor);
};

//-----------------------------------------------------------------------------
class DigitalClock : public LCDNumber
{
 Q_OBJECT

 public:
	DigitalClock(QWidget *parent, const char *name = 0);

    void setBestScores(uint first, uint last)
        { _firstScore = first; _lastScore = last; }
    uint score() const { return 3600 - (_min*60 + _sec); }

 protected:
	void timerEvent( QTimerEvent * );

 public slots:
	void zero();
	void freeze() { stop = true;  }
	void start()  { stop = false; }

 private:
	uint _sec, _min, _firstScore, _lastScore;
	bool stop;

	void showTime();
};

//-----------------------------------------------------------------------------
class CustomDialog : public KDialogBase
{
 Q_OBJECT

 public:
	CustomDialog(LevelData &lev, QWidget *parent);

	static uint maxNbMines(uint width, uint height);

 private slots:
	void widthChanged(int);
	void heightChanged(int);
	void nbMinesChanged(int);

 private:
	KIntNumInput *km;
	LevelData    &lev, initLev;

	void updateNbMines();
};

//-----------------------------------------------------------------------------
class OptionDialog : public KDialogBase
{
 Q_OBJECT

 public:
	OptionDialog(QWidget *parent);

    static uint readCaseSize();
    static bool readUMark();
    static bool readKeyboard();
    static bool readPauseFocus();
    static MouseAction readMouseBinding(MouseButton);

    static QColor readColor(const QString & key, QColor defaultColor);

	static CaseProperties readCaseProperties();
	static LevelData readLevel();
	static void writeLevel(const LevelData &);
	static bool readMenuVisible();
	static void writeMenuVisible(bool visible);

 private slots:
	void accept();
    void slotDefault();

 private:
    KIntNumInput *_caseSize;
	QCheckBox    *_umark, *_keyb, *_focus;
	QComboBox    *_cb[3];

    KColorButton *_flag, *_explosion, *_error;
	QArray<KColorButton *> _numbers;

    HighscoresOption *highscores;

    static KConfig *config();
};

#endif // DIALOGS_H
