/*
    Copyright (C) 2007 Mauricio Piacentini   <mauricio@tabuleiro.com>

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

#ifndef _KMINESTHEME_H_
#define _KMINESTHEME_H_

class KMinesThemePrivate;
class QString;
class QPixmap;

class KMinesTheme 
{
  public:
    KMinesTheme();
    ~KMinesTheme();

    /**
     * Load the default theme file. Called "default.desktop"
     * @return true if the theme files and properties could be loaded
     */
    bool loadDefault();
    /**
     * Load a specific theme file. e.g. "classic.desktop"
     * @return true if the theme files and properties could be loaded
     */
    bool load(const QString &file);
    QString path() const;
    QString graphics() const;
    QPixmap preview() const;
    QString authorProperty(const QString &key) const;

 private:
    friend class KMinesThemePrivate;
    KMinesThemePrivate *const d;
};

#endif
