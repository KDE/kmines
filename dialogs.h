#ifndef DIALOGS_H
#define DIALOGS_H

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <knuminput.h>
#include <kcolorbutton.h>
#include "ghighscores.h"
#include "gsettings.h"
#include "gmisc_ui.h"

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
class DigitalClock : public LCDClock
{
 Q_OBJECT
 public:
    DigitalClock(QWidget *parent);

    void reset(const KExtHighscores::Score &first,
               const KExtHighscores::Score &last);

    KExtHighscores::Score score() const;

 public slots:
    void incActions() { _nbActions++; }
    void start();

 private slots:
    void timeoutClock();

 private:
    KExtHighscores::Score _first, _last;
	uint                  _nbActions;
};

//-----------------------------------------------------------------------------
class CustomSettings : public KSettingWidget, public KMines
{
 Q_OBJECT
 public:
	CustomSettings();

    static Level readLevel();

 private slots:
    void updateNbMines();
    void typeChosen(int);

 private:
	KIntNumInput *_width, *_height, *_mines;
    QComboBox    *_gameType;
};

//-----------------------------------------------------------------------------
class GameSettings : public KSettingWidget, public KMines
{
 Q_OBJECT
 public:
    GameSettings();

    static bool readUMark();
    static bool readKeyboard();
    static bool readPauseFocus();
    static MouseAction readMouseBinding(MouseButton);
};

class AppearanceSettings : public KSettingWidget, public KMines
{
 Q_OBJECT
 public:
    AppearanceSettings();

    static CaseProperties readCaseProperties();
};

#endif
