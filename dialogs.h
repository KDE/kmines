#ifndef DIALOGS_H
#define DIALOGS_H

#include <qlabel.h>
#include <qscrollbar.h>
#include <qlineedit.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <knuminput.h>
#include <kcolorbutton.h>

#include "generic/gsettings.h"
#include "defines.h"


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
	CustomDialog(Level &, QWidget *parent);

 private slots:
	void widthChanged(int);
	void heightChanged(int);
	void nbMinesChanged(int);
    void typeChosen(int);

 private:
	KIntNumInput *_width, *_height, *_mines;
    QComboBox    *_gameType;
	Level        &_level;

	void updateNbMines();
};

//-----------------------------------------------------------------------------
class GameSettingsWidget : public SettingsWidget, public KMines
{
 Q_OBJECT
 public:
    GameSettingsWidget();

    void load();
    void save();
    void defaults();

    static bool readUMark();
    static bool readKeyboard();
    static bool readPauseFocus();
    static MouseAction readMouseBinding(MouseButton);

 private:
    QCheckBox *_umark, *_keyb, *_focus;
	QComboBox *_cb[3];
};

class AppearanceSettingsWidget : public SettingsWidget, public KMines
{
 Q_OBJECT
 public:
    AppearanceSettingsWidget();

    void load();
    void save();
    void defaults();

    static CaseProperties readCaseProperties();
    static void writeCaseSize(uint size);

 private:
    KIntNumInput             *_caseSize;
    KColorButton             *_flag, *_explosion, *_error;
    QPtrVector<KColorButton>  _numbers;

    static uint readCaseSize();
    static QColor readColor(const QString & key, QColor defaultColor);
};

class ExtSettingsDialog : public SettingsDialog, public KMines
{
 Q_OBJECT
 public:
	ExtSettingsDialog(QWidget *parent);

	static Level readLevel();
	static void writeLevel(const Level &);
	static bool readMenuVisible();
	static void writeMenuVisible(bool visible);
};

#endif
