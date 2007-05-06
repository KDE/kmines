/*
    Copyright (C) 2006 Mauricio Piacentini  <mauricio@tabuleiro.com>

    KMines is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __KMINESTHEMESELECTOR_H_
#define __KMINESTHEMESELECTOR_H_

#include <QMap>
#include "ui_kminesthemeselector.h"

class KMinesTheme;
class KConfigSkeleton;

class KMinesThemeSelector : public QWidget, public Ui::KMinesThemeSelector
{
Q_OBJECT
public:
    explicit KMinesThemeSelector( QWidget* parent, KConfigSkeleton * config );

    void setupData(KConfigSkeleton * config);

    QMap<QString, KMinesTheme *> themeMap;
public slots:
    void updatePreview();
    void openKNewStuffDialog();
};

#endif
