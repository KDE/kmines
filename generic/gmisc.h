/*
    This file is part of the KDE games library
    Copyright (C) 2002 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef G_MISC_H
#define G_MISC_H

#include "gsettings.h"

/**
 * This is just a convenience wrapper since we want at the moment
 * no dependance between KExtHighscores and KConfigItem.
 */
class HighscoresConfigWidget : public KConfigWidget
{
 Q_OBJECT
 public:
    HighscoresConfigWidget();
};

#endif
