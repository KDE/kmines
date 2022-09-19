/*
    SPDX-FileCopyrightText: 1996-2004 Nicolas HADACEK <hadacek@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// own
#include "kmines_version.h"
#include "mainwindow.h"
// KF
#include <KAboutData>
#include <KCrash>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <KLocalizedString>
#include <KDBusService>
#include <KSharedConfig>
// Qt
#include <QApplication>
#include <QCommandLineParser>


int main(int argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Fixes blurry icons with fractional scaling
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kmines"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kminesrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kminesui.rc"));
    if(migrate.migrate())
    {
        // update the configuration cache
        KSharedConfig::openConfig()->reparseConfiguration();
    }
#endif
    KLocalizedString::setApplicationDomain("kmines");

    KAboutData aboutData(QStringLiteral("kmines"), i18n("KMines"),
                         QStringLiteral(KMINES_VERSION_STRING),
                         i18n("KMines is a classic minesweeper game"),
                         KAboutLicense::GPL,
                         i18n("(c) 1996-2005, Nicolas Hadacek\n(c) 2001, Mikhail Kourinny\n(c) 2006-2007, Mauricio Piacentini\n(c) 2007, Dmitry Suzdalev"),
                         QString(),
                         QStringLiteral("https://apps.kde.org/kmines"));
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
        auto *mw = new KMinesMainWindow;
        mw->show();
    }
    
    return app.exec();
}
