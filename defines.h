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

#ifndef DEFINES_H
#define DEFINES_H

#include <qcolor.h>


class Level
{
 public:
    enum Type { Easy = 0, Normal, Expert, NbLevels, Custom = NbLevels };
    struct Data {
        uint        width, height, nbMines;
        const char *label, *wwLabel, *i18nLabel;
    };

    Level(Type);
    Level(uint width, uint height, uint nbMines);

    static const Data &data(Type type) { return DATA[type]; }

    uint width() const   { return _width; }
    uint height() const  { return _height; }
    uint nbMines() const { return _nbMines; }
    Type type() const;
    const Data &data() const { return data(type()); }
    static uint maxNbMines(uint width, uint height) { return width*height - 2;}

 private:
    static const Data DATA[NbLevels+1];
    uint _width, _height, _nbMines;
};

class KMines
{
 public:
    enum GameState   { Playing = 0, Paused, GameOver, Stopped, Replaying,
                       Init, NB_STATES };
    static const char *STATES[NB_STATES];
    enum MouseAction { Reveal = 0, AutoReveal, Mark, UMark, None };
    enum MouseButton { LeftButton = 0, MidButton, RightButton,
                       NB_MOUSE_BUTTONS };
    static const char *MOUSE_CONFIG_NAMES[NB_MOUSE_BUTTONS];

    enum CaseState { Covered, Uncovered, Uncertain, Marked, Exploded, Error };
    struct Case {
        bool      mine;
        CaseState state;
    };

    enum Color { FlagColor = 0, ExplosionColor, ErrorColor, NB_COLORS };
    static const char *COLOR_CONFIG_NAMES[NB_COLORS];
    enum NumberColor { NB_N_COLORS = 8 };
    static const char *N_COLOR_CONFIG_NAMES[NB_N_COLORS];
    enum Mood { Normal = 0, Stressed, Happy, Sad, Sleeping, NbMoods };
};

#endif
