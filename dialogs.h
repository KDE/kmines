/*
 * Copyright (c) 1996-2003 Nicolas HADACEK (hadacek@kde.org)
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

#include "defines.h"


class KComboBox;
class KIntNumInput;

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
class CustomConfig : public QWidget, public KMines
{
 Q_OBJECT
 public:
    CustomConfig();

    static Level level();
    static const uint maxWidth;
    static const uint minWidth;
    static const uint maxHeight;
    static const uint minHeight;

    void init() { updateNbMines(); }

 private slots:
    void typeChosen(int);
    void updateNbMines();

 private:
    bool _block;
    KIntNumInput *_width, *_height, *_mines;
    KComboBox    *_gameType;

    static const uint defaultWidth;
    static const uint defaultHeight;
    static const uint defaultNbMines;
};

//-----------------------------------------------------------------------------
class GameConfig : public QWidget, public KMines
{
 Q_OBJECT
 public:
    GameConfig();

    static bool isUncertainMarkEnabled();
    static bool isKeyboardEnabled();
    static bool isPauseFocusEnabled();
    static bool isMagicRevealEnabled();
    static MouseAction mouseAction(MouseButton);

    static Level::Type level();
    static void saveLevel(Level::Type);

    void init() { _magicDialogEnabled = true; }

 private slots:
    void magicModified(bool);

 private:
    bool _magicDialogEnabled;

    static const char *MOUSE_BUTTON_LABELS[NB_MOUSE_BUTTONS];
    static const char *MOUSE_CONFIG_NAMES[NB_MOUSE_BUTTONS];
    static const char *MOUSE_ACTION_LABELS[NB_MOUSE_ACTIONS];
};

class AppearanceConfig : public QWidget, public KMines
{
 Q_OBJECT
 public:
    AppearanceConfig();

    static uint caseSize();
    static QColor color(Color);
    static QColor nColor(uint);

    static bool isMenubarVisible();
    static void saveMenubarVisible(bool visible);

 private:
    static const char *COLOR_LABELS[NB_COLORS];
    static const char *COLOR_CONFIG_NAMES[NB_COLORS];
    static const char *COLOR_DEFAULTS[NB_COLORS];
    static const char *N_COLOR_CONFIG_NAMES[NB_N_COLORS];
    static const char *N_COLOR_DEFAULTS[NB_N_COLORS];
};

#endif
