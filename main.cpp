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

#include <KAboutData>
#include <KCrash>
#include <Kdelibs4ConfigMigrator>
#include <klocalizedstring.h>

#include <QApplication>
#include <QCommandLineParser>
#include <KDBusService>
#include <KSharedConfig>
#include "version.h"
#include "mainwindow.h"


static const char *DESCRIPTION
    = I18N_NOOP("KMines is a classic minesweeper game");

int main(int argc, char **argv)
{
    // Fixes blurry icons with fractional scaling
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);

    Kdelibs4ConfigMigrator migrate(QStringLiteral("kmines"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kminesrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kminesui.rc"));
    if(migrate.migrate())
    {
        // update the configuration cache
        KSharedConfig::openConfig()->reparseConfiguration();
    }
    KLocalizedString::setApplicationDomain("kmines");

    KAboutData aboutData(QStringLiteral("kmines"), i18n("KMines"), QStringLiteral(LONG_VERSION),
						 i18n(DESCRIPTION), KAboutLicense::GPL,
						 i18n(COPYLEFT), QString(), QStringLiteral(HOMEPAGE));
    aboutData.addAuthor(i18n("Nicolas Hadacek"),
                        i18n("Original author"), QStringLiteral("hadacek@kde.org"));
    aboutData.addAuthor(i18n("Mauricio Piacentini"),
                        i18n("Code refactoring and SVG support. Current maintainer"),
                        QStringLiteral("mauricio@tabuleiro.com"));
    aboutData.addAuthor(i18n("Dmitry Suzdalev"),
                        i18n("Rewrite to use QGraphicsView framework. Current maintainer"),
                        QStringLiteral("dimsuz@gmail.com"));
    aboutData.addCredit(i18n("Andreas Zehender"), i18n("Smiley pixmaps"));
    aboutData.addCredit(i18n("Mikhail Kourinny"), i18n("Solver/Adviser"));
    aboutData.addCredit(i18n("Thomas Capricelli"), i18n("Magic reveal mode"));
    aboutData.addCredit(i18n("Brian Croom"), i18n("Port to use KGameRenderer"));
    
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("kmines"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kmines")));
    KAboutData::setApplicationData(aboutData);
    KCrash::initialize();
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    KDBusService service; 
    
    if ( app.isSessionRestored() )
        kRestoreMainWindows<KMinesMainWindow>();
    else {
        KMinesMainWindow *mw = new KMinesMainWindow;
        mw->show();
    }
    
    return app.exec();
}
