/*
 * Copyright (c) 1996-2004 Nicolas HADACEK <hadacek@kde.org>
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

#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "version.h"
#include "mainwindow.h"


static const char *DESCRIPTION
    = I18N_NOOP("KMines is a classic minesweeper game");

int main(int argc, char **argv)
{
    KAboutData aboutData("kmines", 0, ki18n("KMines"), LONG_VERSION,
						 ki18n(DESCRIPTION), KAboutData::License_GPL,
						 ki18n(COPYLEFT), KLocalizedString(), HOMEPAGE);
    aboutData.addAuthor(ki18n("Nicolas Hadacek"), KLocalizedString(), EMAIL);
    aboutData.addAuthor(ki18n("Mauricio Piacentini"),
                        ki18n("Code refactoring and SVG support. Current maintainer"),
                        "mauricio@tabuleiro.com");
    aboutData.addAuthor(ki18n("Dmitry Suzdalev"),
                        ki18n("Rewrite to use QGraphicsView framework. Current maintainer"),
                        "dimsuz@gmail.com");
    aboutData.addCredit(ki18n("Andreas Zehender"), ki18n("Smiley pixmaps"));
    aboutData.addCredit(ki18n("Mikhail Kourinny"), ki18n("Solver/Adviser"));
    aboutData.addCredit(ki18n("Thomas Capricelli"), ki18n("Magic reveal mode"));
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    KGlobal::locale()->insertCatalog("libkdegames");

    if ( a.isSessionRestored() )
        RESTORE(KMinesMainWindow)
    else {
        KMinesMainWindow *mw = new KMinesMainWindow;
        mw->show();
    }
    return a.exec();
}
