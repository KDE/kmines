/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <qpushbutton.h>

#include <kgamelcd.h>
#include <kexthighscore.h>

#include "gsettings.h"
#include "defines.h"


class KComboBox;

//-----------------------------------------------------------------------------
class Smiley : public QPushButton, public KMines
{
 Q_OBJECT
 public:
    Smiley(QWidget *parent, const char *name = 0)
        : QPushButton(QString::null, parent, name) {}

 public slots:
    void setMood(Mood);

 private:
    static const char **XPM_NAMES[NbMoods];
};

//-----------------------------------------------------------------------------
class DigitalClock : public KGameLCDClock
{
 Q_OBJECT
 public:
    DigitalClock(QWidget *parent);

    void reset(bool customGame);

    bool cheating() const { return _cheating; }
    uint nbActions() const { return _nbActions; }
    KExtHighscore::Score score() const;

 public slots:
    void start();
    void setCheating();
    void addAction() { _nbActions++; }

 private slots:
    void timeoutClock();

 private:
    KExtHighscore::Score _first, _last;
    uint _nbActions;
    bool _cheating, _customGame;
};

//-----------------------------------------------------------------------------
class CustomConfig : public KConfigWidget, public KMines
{
 Q_OBJECT
 public:
	CustomConfig();

    static Level readLevel();

 private slots:
    void updateNbMines();
    void typeChosen(int);

 private:
	KConfigItem *_width, *_height, *_mines;
    KComboBox   *_gameType;
};

//-----------------------------------------------------------------------------
class GameConfig : public KConfigWidget, public KMines
{
 Q_OBJECT
 public:
    GameConfig();

 private slots:
    void modified(KConfigItem *);

 private:
    KConfigItem *_magic;
};

class AppearanceConfig : public KConfigWidget, public KMines
{
 Q_OBJECT
 public:
    AppearanceConfig();
};

#endif
