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
#include <kiconloader.h>

#include "ghighscores_internal.h"
#include "ghighscores.h"
#include "ghighscores_tab.h"


namespace KExtHighscore
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
ScoresList::ScoresList(const ItemArray &items, QWidget *parent)
    : KListView(parent), _items(items)
{
    setSelectionMode(QListView::NoSelection);
    setItemMargin(3);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    header()->setClickEnabled(false);
    header()->setMovingEnabled(false);
}

void ScoresList::addHeader()
{
    addLine(0, static_cast<QListViewItem *>(0));
}

QListViewItem *ScoresList::addLine(uint index, bool highlight)
{
    QListViewItem *item = new ShowItem(this, highlight);
    addLine(index, item);
    return item;
}

void ScoresList::addLine(uint index, QListViewItem *line)
{
    uint k = 0;
    for (uint i=0; i<_items.size(); i++) {
        const ItemContainer &container = *_items[i];
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

void ScoresList::reload()
{
    uint index = 0;
    QListViewItem *item = firstChild();
    while (item) {
        uint k = 0;
        for (uint i=0; i<_items.size(); i++) {
            const ItemContainer &container = *_items[i];
            if ( !container.item()->isVisible() || !showColumn(container) )
                continue;
            item->setText(k, itemText(container, index));
            k++;
        }
        index++;
        item = item->nextSibling();
    }
}

//-----------------------------------------------------------------------------
HighscoresList::HighscoresList(const ItemArray &items, int highlight,
                               QWidget *parent)
    : ScoresList(items, parent)
{
    addHeader();

    QListViewItem *line = 0;
    for (int j=items.nbEntries()-1; j>=0; j--) {
        QListViewItem *item = addLine(j, j==highlight);
        if ( j==highlight ) line = item;
    }
    if (line) ensureItemVisible(line);
}

QString HighscoresList::itemText(const ItemContainer &item, uint row) const
{
    return item.pretty(row);
}

//-----------------------------------------------------------------------------
HighscoresWidget::HighscoresWidget(int localRank, QWidget *parent)
    : QWidget(parent, "show_highscores_widget"), _scoresList(0), _scoresUrl(0),
      _playersUrl(0)
{
    const ScoreInfos &s = internal->scoreInfos();
    const PlayerInfos &p = internal->playerInfos();

    QVBoxLayout *vbox = new QVBoxLayout(this, KDialogBase::spacingHint());

    _tw = new QTabWidget(this);
    connect(_tw, SIGNAL(currentChanged(QWidget *)), SLOT(tabChanged()));
    vbox->addWidget(_tw);

    // scores tab
    QWidget *w;
    if ( s.nbEntries()==0 ) {
        QLabel *lab = new QLabel(i18n("no score entry"), _tw);
        lab->setAlignment(AlignCenter);
        w = lab;
        _scoresList = 0;
    } else {
        _scoresList = new HighscoresList(s, localRank, _tw);
        w = _scoresList;
    }
    _tw->addTab(w, i18n("Best &Scores"));

    // players tab
    _playersList = new HighscoresList(p, p.id(), _tw);
    _tw->addTab(_playersList, i18n("&Players"));

    // statistics tab
    if ( internal->showStatistics )
        _tw->addTab(new StatisticsTab(_tw), i18n("Statistics"));

    if ( p.histogram().size()!=0 )
        _tw->addTab(new HistogramTab(_tw), i18n("Histogram"));

    // additionnal tabs
    internal->additionnalTabs(_tw);

    // url labels
    if ( internal->isWWHSAvailable() ) {
        KURL url = internal->queryURL(ManagerPrivate::Scores);
        _scoresUrl = new KURLLabel(url.url(),
                                   i18n("View world-wide highscores"), this);
        connect(_scoresUrl, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(_scoresUrl);

        url = internal->queryURL(ManagerPrivate::Players);
        _playersUrl = new KURLLabel(url.url(),
                                    i18n("View world-wide players"), this);
        connect(_playersUrl, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(_playersUrl);
    }
}

void HighscoresWidget::changeTab(int i)
{
    if ( i!=_tw->currentPageIndex() )
        _tw->setCurrentPage(i);
}

void HighscoresWidget::showURL(const QString &url) const
{
    KFileOpenWithHandler foo;
    (void)new KRun(KURL(url));
}

void HighscoresWidget::reload()
{
    if (_scoresList)
        _scoresList->reload();
    _playersList->reload();
    if (_scoresUrl)
        _scoresUrl->setURL(internal->queryURL(ManagerPrivate::Scores).url());
    if (_playersUrl)
        _playersUrl->setURL(internal->queryURL(ManagerPrivate::Players).url());
}

//-----------------------------------------------------------------------------
HighscoresDialog::HighscoresDialog(uint nbGameTypes, int rank, QWidget *parent)
    : KDialogBase(nbGameTypes>1 ? TreeList : Plain, i18n("Highscores"),
                  Close|User1|User2, Close,
                  parent, "show_highscores", true, true)
{
    bool several = ( nbGameTypes>1 );
    uint type = internal->gameType();
    for (uint i=0; i<nbGameTypes; i++) {
        internal->setGameType(i);
        QWidget *w;
        if (several) {
            QString title = internal->manager.gameTypeLabel(i, Manager::I18N);
            QString icon = internal->manager.gameTypeLabel(i, Manager::Icon);
            w = addPage(title, QString::null, BarIcon(icon, KIcon::SizeLarge));
        } else w = plainPage();
        QVBoxLayout *vbox = new QVBoxLayout(w);
        HighscoresWidget *hw = new HighscoresWidget((i==type ? rank : -1), w);
        connect(hw, SIGNAL(tabChanged(int)), SLOT(tabChanged(int)));
        vbox->addWidget(hw);
        _widgets.append(hw);
    }
    setGameType(type);
    if (several) showPage(type);

    setButtonText(User1, i18n("Export..."));
    connect(this, SIGNAL(user1Clicked()), SLOT(exportToText()));
    setButtonText(User2, i18n("Configure..."));
    connect(this, SIGNAL(user2Clicked()), SLOT(configure()));
    enableButtonSeparator(several);
}

void HighscoresDialog::tabChanged(int k)
{
    for (uint i=0; i<_widgets.size(); i++)
        _widgets[i]->changeTab(k);
}

void HighscoresDialog::configure()
{
    if ( KExtHighscore::configure(this) ) reload();
}

void HighscoresDialog::reload()
{
    for (uint i=0; i<_widgets.size(); i++)
        _widgets[i]->reload();
}

void HighscoresDialog::exportToText()
{
    KURL url = KFileDialog::getSaveURL(QString::null, QString::null, this);
    if ( url.isEmpty() ) return;
    if ( KIO::NetAccess::exists(url) ) {
        KGuiItem gi = KStdGuiItem::save();
        gi.setText(i18n("Overwrite"));
        int res = KMessageBox::warningYesNo(this,
                                 i18n("The file already exists. Overwrite?"),
                                 i18n("Export"), gi, KStdGuiItem::cancel());
        if ( res==KMessageBox::No ) return;
    }
    KTempFile tmp;
    internal->exportHighscores(*tmp.textStream());
    tmp.close();
    KIO::NetAccess::upload(tmp.name(), url);
    tmp.unlink();
}

//-----------------------------------------------------------------------------
MultipleScoresList::MultipleScoresList(const ScoreVector &scores,
                                       QWidget *parent)
    : ScoresList(internal->scoreInfos(), parent), _scores(scores)
{
    Q_ASSERT( scores.size()!=0 );

    addHeader();
    for (uint i=0; i<scores.size(); i++)
        addLine(i, false);
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
ConfigDialog::ConfigDialog(QWidget *parent)
    : KDialogBase(Swallow, i18n("Configure Highscores"),
                  Ok|Apply|Cancel, Cancel,
                  parent, "configure_highscores", true, true),
      _saved(false), _WWHEnabled(0)
{
    QWidget *page = 0;
    QTabWidget *tab = 0;
    if ( internal->isWWHSAvailable() ) {
        tab = new QTabWidget(this);
        setMainWidget(tab);
        page = new QWidget(tab);
        tab->addTab(page, i18n("Main"));
    } else {
        page = new QWidget(this);
        setMainWidget(page);
    }

    QGridLayout *pageTop =
        new QGridLayout(page, 2, 2, spacingHint(), spacingHint());

    QLabel *label = new QLabel(i18n("Nickname:"), page);
    pageTop->addWidget(label, 0, 0);
    _nickname = new QLineEdit(page);
    connect(_nickname, SIGNAL(textChanged(const QString &)),
            SLOT(modifiedSlot()));
    _nickname->setMaxLength(16);
    pageTop->addWidget(_nickname, 0, 1);

    label = new QLabel(i18n("Comment:"), page);
    pageTop->addWidget(label, 1, 0);
    _comment = new QLineEdit(page);
    connect(_comment, SIGNAL(textChanged(const QString &)),
            SLOT(modifiedSlot()));
    _comment->setMaxLength(50);
    pageTop->addWidget(_comment, 1, 1);

    if (tab) {
        _WWHEnabled
            = new QCheckBox(i18n("World-wide highscores enabled"), page);
        connect(_WWHEnabled, SIGNAL(toggled(bool)),
                SLOT(modifiedSlot()));
        pageTop->addMultiCellWidget(_WWHEnabled, 2, 2, 0, 1);

        // advanced tab
        QWidget *page = new QWidget(tab);
        tab->addTab(page, i18n("Advanced"));
        QVBoxLayout *pageTop =
            new QVBoxLayout(page, spacingHint(), spacingHint());

        QVGroupBox *group = new QVGroupBox(i18n("Registration Data"), page);
        pageTop->addWidget(group);
        QGrid *grid = new QGrid(2, group);
        grid->setSpacing(spacingHint());

        label = new QLabel(i18n("Nickname:"), grid);
        _registeredName = new KLineEdit(grid);
        _registeredName->setReadOnly(true);

        label = new QLabel(i18n("Key:"), grid);
        _key = new KLineEdit(grid);
        _key->setReadOnly(true);

        KGuiItem gi = KStdGuiItem::clear();
        gi.setText(i18n("Remove"));
        _removeButton = new KPushButton(gi, grid);
        connect(_removeButton, SIGNAL(clicked()), SLOT(removeSlot()));
    }

    load();
    enableButtonApply(false);
}

void ConfigDialog::modifiedSlot()
{
    enableButtonApply(true);
}

void ConfigDialog::accept()
{
    if ( save() ) {
        KDialogBase::accept();
        kapp->config()->sync(); // #### safer
    }
}

void ConfigDialog::removeSlot()
{
    KGuiItem gi = KStdGuiItem::clear();
    gi.setText(i18n("Remove"));
    int res = KMessageBox::warningYesNo(this,
                               i18n("This will permanently remove your "
                               "registration key. You will not be able to use "
                               "the currently registered nickname anymore."),
                               QString::null, gi, KStdGuiItem::cancel());
    if ( res==KMessageBox::Yes ) {
        internal->playerInfos().removeKey();
        _registeredName->clear();
        _key->clear();
        _removeButton->setEnabled(false);
        _WWHEnabled->setChecked(false);
        modifiedSlot();
    }
}

void ConfigDialog::load()
{
    const PlayerInfos &infos = internal->playerInfos();
    _nickname->setText(infos.isAnonymous() ? QString::null : infos.name());
    _comment->setText(infos.comment());
    if (_WWHEnabled) {
        _WWHEnabled->setChecked(infos.isWWEnabled());
        if ( !infos.key().isEmpty() ) {
            _registeredName->setText(infos.registeredName());
            _registeredName->home(false);
            _key->setText(infos.key());
            _key->home(false);
        }
        _removeButton->setEnabled(!infos.key().isEmpty());
    }
}

bool ConfigDialog::save()
{
    bool enabled = (_WWHEnabled ? _WWHEnabled->isChecked() : false);

    // do not bother the user with "nickname empty" if he has not
    // messed with nickname settings ...
    if ( _nickname->text().isEmpty()
         && !internal->playerInfos().isAnonymous() && !enabled )
        return true;

    bool res = internal->modifySettings(_nickname->text().lower(),
                                        _comment->text(), enabled, this);
    if (res) {
        load(); // needed to update view when "apply" is clicked
        enableButtonApply(false);
    }
    _saved = true;
    return res;
}

}; // namespace
