/*
    This file is part of the KDE games library
    Copyright (C) 2001 Nicolas Hadacek (hadacek@kde.org)

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

#include "ghighscores_gui.h"
#include "ghighscores_gui.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qheader.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qgrid.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kurllabel.h>
#include <kopenwith.h>
#include <krun.h>

#include "ghighscores_internal.h"


namespace KExtHighscores
{

//-----------------------------------------------------------------------------
ShowItem::ShowItem(QListView *list, bool highlight)
    : KListViewItem(list), _highlight(highlight)
{}

void ShowItem::paintCell(QPainter *p, const QColorGroup &cg,
                         int column, int width, int align)
{
    QColorGroup cgrp(cg);
    if (_highlight) cgrp.setColor(QColorGroup::Text, red);
    KListViewItem::paintCell(p, cgrp, column, width, align);
}

//-----------------------------------------------------------------------------
ScoresList::ScoresList(QWidget *parent)
    : KListView(parent)
{
    setSelectionMode(QListView::NoSelection);
    setItemMargin(3);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    header()->setClickEnabled(false);
    header()->setMovingEnabled(false);
}

void ScoresList::addLine(const ItemArray &items, int index, bool highlight)
{
    QListViewItem *line = (index==-1 ? 0 : new ShowItem(this, highlight));
    int k = -1;
    for (uint i=0; i<items.size(); i++) {
        const ItemContainer &item = *items[i];
        if ( !item.item()->isVisible() || !showColumn(item) ) continue;
        k++;
        if (line) line->setText(k, itemText(item, index));
        else {
            addColumn( item.item()->label() );
            setColumnAlignment(k, item.item()->alignment());
        }
    }
}

//-----------------------------------------------------------------------------
HighscoresList::HighscoresList(const ItemArray &array, int highlight,
                               QWidget *parent)
    : ScoresList(parent)
{
    uint nb = array.nbEntries();
    for (int j=nb; j>=0; j--)
        addLine(array, (j==(int)nb ? -1 : j), j==highlight);
}

QString HighscoresList::itemText(const ItemContainer &item, uint row) const
{
    return item.pretty(row);
}

//-----------------------------------------------------------------------------
HighscoresWidget::HighscoresWidget(int localRank,
         QWidget *parent, const ScoreInfos &score, const PlayerInfos &player,
         int spacingHint, bool WWHSAvailable, const QString &highscoresURL,
         const QString &playersURL)
    : QWidget(parent, "show_highscores_widget")
{
    QVBoxLayout *vbox = new QVBoxLayout(this, spacingHint);

    QTabWidget *tw = new QTabWidget(this);
    vbox->addWidget(tw);

    QWidget *w;
    if ( score.nbEntries()==0 ) {
        QLabel *lab = new QLabel(i18n("no score entry"), this);
        lab->setAlignment(AlignCenter);
        w = lab;
    } else w = new HighscoresList(score, localRank, this);
    tw->addTab(w, i18n("Best &Scores"));

    w = new HighscoresList(player, player.id(), this);
    tw->addTab(w, i18n("&Players"));

    if (WWHSAvailable) {
        KURLLabel *urlLabel = new KURLLabel(highscoresURL,
                                     i18n("View world-wide highscores"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);

        urlLabel = new KURLLabel(playersURL,
                                 i18n("View world-wide players"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);
    }
}

void HighscoresWidget::showURL(const QString &url) const
{
    KFileOpenWithHandler foo;
    (void)new KRun(KURL(url));
}

//-----------------------------------------------------------------------------
MultipleScoresList::MultipleScoresList(const ScoreList &scores,
                                       QWidget *parent)
    : ScoresList(parent), _scores(scores)
{
    Q_ASSERT( scores.size()!=0 );

    const ItemArray &items = scores[0].items();
    addLine(items, -1, false);
    for (uint i=0; i<scores.size(); i++) addLine(items, i, false);
}

QString MultipleScoresList::itemText(const ItemContainer &item, uint row) const
{
    QString name = item.name();
    if ( name=="rank" ) {
        if ( _scores[row].type()==Won ) return i18n("Winner");
        return QString::null;
    }
    return item.item()->pretty(row, _scores[row].data(name));
}

bool MultipleScoresList::showColumn(const ItemContainer &item) const
{
    return ( item.name()!="date" );
}

//-----------------------------------------------------------------------------
HighscoresSettingsWidget::HighscoresSettingsWidget(const PlayerInfos &infos,
                                           bool WWHSAvailable, QWidget *parent)
    : SettingsWidget(i18n("Highscores"), "highscores", parent),
      _ok(true), _infos(infos), _WWHEnabled(0)
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialog::spacingHint());

    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(KDialog::spacingHint());
    top->addWidget(grid);

    (void)new QLabel(i18n("Nickname"), grid);
    _nickname = new QLineEdit(grid);
    _nickname->setMaxLength(16);

    (void)new QLabel(i18n("Comment"), grid);
    _comment = new QLineEdit(grid);
    _comment->setMaxLength(50);

    if (WWHSAvailable) {
        _WWHEnabled
            = new QCheckBox(i18n("world-wide highscores enabled"), this);
        top->addWidget(_WWHEnabled);
    }
}

void HighscoresSettingsWidget::load()
{
    _nickname->setText(_infos.isAnonymous() ? QString::null : _infos.name());
    _comment->setText(_infos.comment());
    if (_WWHEnabled) _WWHEnabled->setChecked(_infos.isWWEnabled());
}

void HighscoresSettingsWidget::save()
{
    bool enabled = (_WWHEnabled ? _WWHEnabled->isChecked() : false);

    // do not bother the user with "nickname empty" if he has not
    // messed with nickname settings ...
    if ( _nickname->text().isEmpty() && !_infos.isAnonymous() && !enabled ) {
        _ok = true;
        return;
    }

    _ok = kHighscores->modifySettings(_nickname->text().lower(),
                              _comment->text(), enabled, (QWidget *)parent());
}

}; // namespace
