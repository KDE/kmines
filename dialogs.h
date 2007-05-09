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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <QPushButton>

#include <kgameclock.h>

#include "defines.h"
#include "settings.h"
#include "ksvgrenderer.h"
#include "kgametheme.h"

class KComboBox;
class KIntNumInput;

//-----------------------------------------------------------------------------
class Smiley : public QPushButton, public KMines
{
 Q_OBJECT
 public:
    explicit Smiley(QWidget *parent);
    ~Smiley();

 public slots:
    void setMood(Mood);
    void readSettings();

 private:
    //static const char **XPM_NAMES[NbMoods];
    QVector<QString> moodNames;
    KSvgRenderer svg;
    KGameTheme *theme;
};

//-----------------------------------------------------------------------------
class CustomConfig : public QWidget, public KMines
{
 Q_OBJECT
 public:
    CustomConfig();

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
};

//-----------------------------------------------------------------------------
class GameConfig : public QWidget, public KMines
{
 Q_OBJECT
 public:
    GameConfig();

    static void saveLevel(Level::Type);

    void init() { _magicDialogEnabled = true; }

 private slots:
    void magicModified(bool);

 private:
    bool _magicDialogEnabled;

};

class AppearanceConfig : public QWidget, public KMines
{
 Q_OBJECT
 public:
    AppearanceConfig();
};

#endif
