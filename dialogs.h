/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

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
    bool cheating() const { return _cheating; }

    KExtHighscores::Score score() const;

 public slots:
    void incActions() { _nbActions++; }
    void start();
    void setCheating();

 private slots:
    void timeoutClock();

 private:
    KExtHighscores::Score _first, _last;
	uint                  _nbActions;
    bool                  _cheating;
};

//-----------------------------------------------------------------------------
class CustomConfig : public KUIConfigWidget
{
 Q_OBJECT
 public:
	CustomConfig();

    static Level readLevel();

 private slots:
    void updateNbMines();
    void typeChosen(int);

 private:
	KRangedUIConfig *_width, *_height, *_mines;
    QComboBox       *_gameType;
};

//-----------------------------------------------------------------------------
class GameConfig : public KUIConfigWidget
{
 Q_OBJECT
 public:
    GameConfig();

    static bool readUMark();
    static bool readKeyboard();
    static bool readPauseFocus();
    static bool readMagicReveal();
    static KMines::MouseAction readMouseBinding(KMines::MouseButton);

 private slots:
    void magicRevealToggled();

 private:
    KUIConfig *_magic;
};

class AppearanceConfig : public KUIConfigWidget
{
 Q_OBJECT
 public:
    AppearanceConfig();

    static KMines::CaseProperties readCaseProperties();
};

#endif
