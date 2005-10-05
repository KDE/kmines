/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
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

#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include <kexthighscore.h>
#include <kdemacros.h>

namespace KExtHighscore
{

class KDE_EXPORT ExtManager : public Manager
{
 public:
    ExtManager();

 private:
    QString gameTypeLabel(uint gameTye, LabelType) const;
    void convertLegacy(uint gameType);
    bool isStrictlyLess(const Score &s1, const Score &s2) const;
};

}

#endif
