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

#include "kminestheme.h"

#include <kstandarddirs.h>
#include <klocale.h>
#include <kconfig.h>
#include <ksvgrenderer.h>
#include <QFile>
#include <QMap>
#include <QDebug>
#include <QPixmap>

class KMinesThemePrivate
{
public:
    KMinesThemePrivate()
    {
    }

    QMap<QString, QString> authorproperties;
    QString filename;
    QString graphics;
    QPixmap preview;
    KSvgRenderer svg;
};

KMinesTheme::KMinesTheme()
    : d(new KMinesThemePrivate)
{
    static bool _inited = false;
    if (_inited)
        return;
    KGlobal::dirs()->addResourceType("kminestheme", KStandardDirs::kde_default("data") + QString::fromLatin1("kmines/themes/"));

    KGlobal::locale()->insertCatalog("kmines");
    _inited = true;
}

KMinesTheme::~KMinesTheme() {
    delete d;
}

bool KMinesTheme::loadDefault()
{
    QString idx = "default.desktop";

    QString themePath = KStandardDirs::locate("kminestheme", idx);
qDebug() << "Inside LoadDefault(), located theme at " << themePath;
    if (themePath.isEmpty()) {
        return false;
    }
    return load(themePath);
}

#define kThemeVersionFormat 1

bool KMinesTheme::load(const QString &file) {

    QString graphicsPath;
    qDebug() << "Attempting to load .desktop at " << file;

    // verify if it is a valid file first and if we can open it
    QFile themefile(file);
    if (!themefile.open(QIODevice::ReadOnly)) {
      return (false);
    }
    themefile.close();

    KConfig themeconfig(file, KConfig::OnlyLocal);
    KConfigGroup group = themeconfig.group("KMinesTheme");

    d->authorproperties.insert("Name", group.readEntry("Name"));// Returns translated data
    d->authorproperties.insert("Author", group.readEntry("Author"));
    d->authorproperties.insert("Description", group.readEntry("Description"));
    d->authorproperties.insert("AuthorEmail", group.readEntry("AuthorEmail"));

    //Version control
    int themeversion = group.readEntry("VersionFormat",0);
    //Format is increased when we have incompatible changes, meaning that older clients are not able to use the remaining information safely
    if (themeversion > kThemeVersionFormat) {
        return false;
    }

    QString graphName = group.readEntry("FileName");
    graphicsPath = KStandardDirs::locate("kminestheme", graphName);
    d->graphics = graphicsPath;
    if (graphicsPath.isEmpty()) return (false);

    d->svg.load(graphicsPath);
    if (!d->svg.isValid()) {
        qDebug() << "could not load svg";
        return( false );
    }
    
    QString previewName = group.readEntry("Preview");
    graphicsPath = KStandardDirs::locate("kminestheme", previewName);
    d->preview = QPixmap(graphicsPath);

    d->filename = file;
    return true;
}

QString KMinesTheme::path() const {
    return d->filename;
}

QString KMinesTheme::graphics() const {
  return d->graphics;
}

QPixmap KMinesTheme::preview() const {
  return d->preview;
}

QString KMinesTheme::authorProperty(const QString &key) const {
    return d->authorproperties[key];
}
