#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <knuminput.h>

#include "gsettings.h"
#include "defines.h"
#include "highscores.h"


//-----------------------------------------------------------------------------
class Smiley : public QPushButton
{
 Q_OBJECT
 public:
    Smiley(QWidget *parent, const char *name = 0)
        : QPushButton(QString::null, parent, name) {}
	enum Mood { Normal = 0, Stressed, Happy, Sad, Sleeping, NbPixmaps };

 public slots:
    void setMood(Smiley::Mood);

 private:
    static const char **XPM_NAMES[NbPixmaps];
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
class CustomDialog : public KDialogBase, public KMines
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
class GameSettingsWidget : public BaseSettingsWidget, public KMines
{
 Q_OBJECT
 public:
    GameSettingsWidget(BaseSettingsDialog *);

    void readConfig();
    bool writeConfig();

    static bool readUMark();
    static bool readKeyboard();
    static bool readPauseFocus();
    static MouseAction readMouseBinding(MouseButton);

 public slots:
    void setDefault();

 private:
    QCheckBox *_umark, *_keyb, *_focus;
	QComboBox *_cb[3];
};

class AppearanceSettingsWidget : public BaseSettingsWidget, public KMines
{
 Q_OBJECT
 public:
    AppearanceSettingsWidget(BaseSettingsDialog *);

    void readConfig();
    bool writeConfig();

    static CaseProperties readCaseProperties();
    static void writeCaseSize(uint size);

 public slots:
    void setDefault();

 private:
    KIntNumInput                 *_caseSize;
    SettingsColorButton          *_flag, *_explosion, *_error;
    QPtrVector<SettingsColorButton>  _numbers;

    static uint readCaseSize();
    static QColor readColor(const QString & key, QColor defaultColor);
};

class SettingsDialog : public BaseSettingsDialog, public KMines
{
 Q_OBJECT
 public:
	SettingsDialog(QWidget *parent);

    static KConfig *config();

	static LevelData readLevel();
	static void writeLevel(const LevelData &);
	static bool readMenuVisible();
	static void writeMenuVisible(bool visible);
};

#endif
