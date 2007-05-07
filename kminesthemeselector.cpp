/*
    Copyright (C) 2007 Mauricio Piacentini  <mauricio@tabuleiro.com>

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

#include "kminesthemeselector.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <QPainter>
#include <KConfigSkeleton>
#include <knewstuff2/engine.h>

#include "kminestheme.h"

KMinesThemeSelector::KMinesThemeSelector( QWidget* parent, KConfigSkeleton * aconfig )
        : QWidget( parent )
{
    setupUi(this);
    setupData(aconfig);
}

void KMinesThemeSelector::setupData(KConfigSkeleton * aconfig)
{
    //Get our currently configured Tileset entry
    KConfig * config = aconfig->config();
    KConfigGroup group = config->group("General");
    QString initialGroup = group.readEntry("Theme");

    //The lineEdit widget holds our bg path, but the user does not manipulate it directly
    kcfg_Theme->hide();

    KMinesTheme bg;

    //Now get our tilesets into a list
    QStringList themesAvailable;
    KGlobal::dirs()->findAllResources("gametheme", QString("*.desktop"), KStandardDirs::Recursive, themesAvailable);
    QString namestr("Name");
    int numvalidentries = 0;
    for (int i = 0; i < themesAvailable.size(); ++i)
    {
        KMinesTheme * atheme = new KMinesTheme();
        QString themepath = themesAvailable.at(i);
        if (atheme->load(themepath)) {
            themeMap.insert(atheme->authorProperty(namestr), atheme);
            themeList->addItem(atheme->authorProperty(namestr));
            //Find if this is our currently configured Theme
            if (themepath==initialGroup) {
                //Select current entry
                themeList->setCurrentRow(numvalidentries);
                updatePreview();
            }
            ++numvalidentries;
        } else {
            delete atheme;
        }
    }
    
    connect(themeList, SIGNAL(currentItemChanged ( QListWidgetItem * , QListWidgetItem * )), this, SLOT(updatePreview()));
    connect(getNewButton, SIGNAL(clicked()), this, SLOT(openKNewStuffDialog()));
}

void KMinesThemeSelector::updatePreview()
{
    KMinesTheme * seltheme = themeMap.value(themeList->currentItem()->text());
        //Sanity checkings. Should not happen.
    if (!seltheme) return;
    if (seltheme->path()==kcfg_Theme->text()) {
        return;
    }
    QString authstr("Author");
    QString contactstr("AuthorEmail");
    QString descstr("Description");
    kcfg_Theme->setText(seltheme->path());
    themeAuthor->setText(seltheme->authorProperty(authstr));
    themeContact->setText(seltheme->authorProperty(contactstr));
    themeDescription->setText(seltheme->authorProperty(descstr));

    //Draw the preview
    //TODO here: add code to maintain aspect ration?
    themePreview->setPixmap(seltheme->preview());

}

void KMinesThemeSelector::openKNewStuffDialog()
{
    KNS::Entry::List entries = KNS::Engine::download();
}

#include "kminesthemeselector.moc"
