/*
    This file is part of the KDE games library
    Copyright (C) 2001-02 Nicolas Hadacek (hadacek@kde.org)

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

#include <qlayout.h>
#include <qtextstream.h>
#include <qtabwidget.h>
#include <qheader.h>
#include <qgrid.h>
#include <qvgroupbox.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kurllabel.h>
#include <kopenwith.h>
#include <krun.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kio/netaccess.h>

#include "ghighscores_internal.h"
#include "ghighscores.h"


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

void ScoresList::addHeader(const ItemArray &items)
{
    addLine(items, 0, (QListViewItem *)0);
}

QListViewItem *ScoresList::addLine(const ItemArray &items, uint index,
                                   bool highlight)
{
    QListViewItem *item = new ShowItem(this, highlight);
    addLine(items, index, item);
    return item;
}

void ScoresList::addLine(const ItemArray &items, uint index,
                         QListViewItem *line)
{
    uint k = 0;
    for (uint i=0; i<items.size(); i++) {
        const ItemContainer &container = *items[i];
        if ( !container.item()->isVisible() || !showColumn(container) )
            continue;
        if (line) line->setText(k, itemText(container, index));
        else {
            addColumn( container.item()->label() );
            setColumnAlignment(k, container.item()->alignment());
        }
        k++;
    }
}

//-----------------------------------------------------------------------------
HighscoresList::HighscoresList(const ItemArray &array, int highlight,
                               QWidget *parent)
    : ScoresList(parent)
{
    addHeader(array);

    uint nb = array.nbEntries();
    QListViewItem *line = 0;
    for (int j=nb-1; j>=0; j--) {
        QListViewItem *item = addLine(array, j, j==highlight);
        if ( j==highlight ) line = item;
    }
    if (line) ensureItemVisible(line);
}

QString HighscoresList::itemText(const ItemContainer &item, uint row) const
{
    return item.pretty(row);
}

//-----------------------------------------------------------------------------
HighscoresWidget::HighscoresWidget(int localRank, QWidget *parent,
                       const QString &playersURL, const QString &scoresURL)
    : QWidget(parent, "show_highscores_widget")
{
    const ScoreInfos &s = HighscoresPrivate::scoreInfos();
    const PlayerInfos &p = HighscoresPrivate::playerInfos();

    QVBoxLayout *vbox = new QVBoxLayout(this, KDialogBase::spacingHint());

    QTabWidget *tw = new QTabWidget(this);
    vbox->addWidget(tw);

    QWidget *w;
    if ( s.nbEntries()==0 ) {
        QLabel *lab = new QLabel(i18n("no score entry"), tw);
        lab->setAlignment(AlignCenter);
        w = lab;
    } else w = new HighscoresList(s, localRank, tw);
    tw->addTab(w, i18n("Best &Scores"));

    w = new HighscoresList(p, p.id(), tw);
    tw->addTab(w, i18n("&Players"));

    HighscoresPrivate::highscores().additionnalTabs(tw);

    if ( HighscoresPrivate::isWWHSAvailable() ) {
        KURLLabel *urlLabel =
            new KURLLabel(scoresURL, i18n("View world-wide highscores"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);

        urlLabel =
            new KURLLabel(playersURL, i18n("View world-wide players"), this);
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
HighscoresDialog::HighscoresDialog(bool treeList, QWidget *parent)
    : KDialogBase(treeList ? TreeList : Plain, i18n("Highscores"),
                  KDialogBase::Close|KDialogBase::User1, KDialogBase::Close,
                  parent, "show_highscores", true, true)
{
    setButtonText(KDialogBase::User1, i18n("Export..."));
    connect(this, SIGNAL(user1Clicked()), SLOT(exportToText()));
    enableButtonSeparator(treeList);
}

void HighscoresDialog::exportToText()
{
    KURL url = KFileDialog::getSaveURL(QString::null, QString::null, this);
    if ( url.isEmpty() ) return;
    if ( KIO::NetAccess::exists(url) ) {
        KGuiItem gi = KStdGuiItem::save();
        gi.setText(i18n("Overwrite"));
        int res = KMessageBox::warningYesNo(this,
                                 i18n("The file already exists. Overwrite ?"),
                                 i18n("Export..."), gi, KStdGuiItem::cancel());
        if ( res==KMessageBox::No ) return;
    }
    KTempFile tmp;
    HighscoresPrivate::exportHighscores(*tmp.textStream());
    tmp.close();
    KIO::NetAccess::upload(tmp.name(), url);
    tmp.unlink();
}

//-----------------------------------------------------------------------------
MultipleScoresList::MultipleScoresList(const QValueList<Score> &scores,
                                       QWidget *parent)
    : ScoresList(parent), _scores(scores)
{
    Q_ASSERT( scores.size()!=0 );

    addHeader(HighscoresPrivate::scoreInfos());
    for (uint i=0; i<scores.size(); i++)
        addLine(HighscoresPrivate::scoreInfos(), i, false);
}

QString MultipleScoresList::itemText(const ItemContainer &item, uint row) const
{
    QString name = item.name();
    if ( name=="rank" ) {
        if ( _scores[row].type()==Won ) return i18n("Winner");
        return QString::null;
    }
    QVariant v = _scores[row].data(name);
    if ( name=="name" ) return v.toString();
    return item.item()->pretty(row, v);
}

bool MultipleScoresList::showColumn(const ItemContainer &item) const
{
    return ( item.name()!="date" );
}

//-----------------------------------------------------------------------------
ImplConfigWidget::ImplConfigWidget(QWidget *parent)
    : ConfigWidget(parent), _WWHEnabled(0)
{
    bool wwhs = HighscoresPrivate::isWWHSAvailable();

    QWidget *page = this;
    QTabWidget *tab = 0;
    if (wwhs) {
        QVBoxLayout *top = new QVBoxLayout(this);
        tab = new QTabWidget(this);
        top->addWidget(tab);

        // main tab
        page = new QWidget(tab);
        tab->addTab(page, i18n("Main"));
    }

    QGridLayout *pageTop = new QGridLayout(page, 2, 2,
                               KDialog::spacingHint(), KDialog::spacingHint());

    QLabel *label = new QLabel(i18n("Nickname"), page);
    pageTop->addWidget(label, 0, 0);
    _nickname = new QLineEdit(page);
    connect(_nickname, SIGNAL(textChanged(const QString &)),
            SIGNAL(modified()));
    _nickname->setMaxLength(16);
    pageTop->addWidget(_nickname, 0, 1);

    label = new QLabel(i18n("Comment"), page);
    pageTop->addWidget(label, 1, 0);
    _comment = new QLineEdit(page);
    connect(_comment, SIGNAL(textChanged(const QString &)),
            SIGNAL(modified()));
    _comment->setMaxLength(50);
    pageTop->addWidget(_comment, 1, 1);

    if (wwhs) {
        _WWHEnabled
            = new QCheckBox(i18n("World-wide highscores enabled"), page);
        connect(_WWHEnabled, SIGNAL(toggled(bool)),
                SIGNAL(modified()));
        pageTop->addMultiCellWidget(_WWHEnabled, 2, 2, 0, 1);

        // advanced tab
        QWidget *page = new QWidget(tab);
        tab->addTab(page, i18n("Advanced"));
        QVBoxLayout *pageTop = new QVBoxLayout(page,
                       KDialogBase::spacingHint(), KDialogBase::spacingHint());

        QVGroupBox *group = new QVGroupBox(i18n("Registration data"), page);
        pageTop->addWidget(group);
        QGrid *grid = new QGrid(2, group);
        grid->setSpacing(KDialogBase::spacingHint());

        label = new QLabel(i18n("Nickname"), grid);
        _registeredName = new KLineEdit(grid);
        _registeredName->setReadOnly(true);

        label = new QLabel(i18n("Key"), grid);
        _key = new KLineEdit(grid);
        _key->setReadOnly(true);

        KGuiItem gi = KStdGuiItem::clear();
        gi.setText(i18n("Remove"));
        _removeButton = new KPushButton(gi, grid);
        connect(_removeButton, SIGNAL(clicked()), SLOT(removeSlot()));
    }
}

QString ImplConfigWidget::title() const
{
    return i18n("Highscores");
}

QString ImplConfigWidget::icon() const
{
    return "highscore";
}

void ImplConfigWidget::load()
{
    const PlayerInfos &infos = HighscoresPrivate::playerInfos();
    _nickname->setText(infos.isAnonymous() ? QString::null : infos.name());
    _comment->setText(infos.comment());
    if (_WWHEnabled) {
        _WWHEnabled->setChecked(infos.isWWEnabled());
        if ( !infos.key().isEmpty() ) {
            _registeredName->setText(infos.registeredName());
            _key->setText(infos.key());
        }
        _removeButton->setEnabled(!infos.key().isEmpty());
    }
}

void ImplConfigWidget::removeSlot()
{
    KGuiItem gi = KStdGuiItem::clear();
    gi.setText(i18n("Remove"));
    int res = KMessageBox::warningYesNo(this,
       i18n("This will remove permanently your "
            "registration key. You will not be able to use anymore "
            "with the current registered nickname."),
       QString::null, gi, KStdGuiItem::cancel());
    if ( res==KMessageBox::Yes ) {
        HighscoresPrivate::playerInfos().removeKey();
        _registeredName->clear();
        _key->clear();
        _removeButton->setEnabled(false);
        _WWHEnabled->setChecked(false);
        emit modified();
    }
}

bool ImplConfigWidget::save()
{
    bool enabled = (_WWHEnabled ? _WWHEnabled->isChecked() : false);

    // do not bother the user with "nickname empty" if he has not
    // messed with nickname settings ...
    if ( _nickname->text().isEmpty()
         && !HighscoresPrivate::playerInfos().isAnonymous() && !enabled )
        return true;

    bool res = HighscoresPrivate::modifySettings(_nickname->text().lower(),
                  _comment->text(), enabled, static_cast<QWidget *>(parent()));
    if (res) load(); // update view when "apply" is clicked
    return res;
}

}; // namespace
