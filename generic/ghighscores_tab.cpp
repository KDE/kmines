/*
    This file is part of the KDE games library
    Copyright (C) 2002 Nicolas Hadacek (hadacek@kde.org)

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

#include "ghighscores_tab.h"
#include "ghighscores_tab.moc"

#include <qlayout.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qgrid.h>
#include <qheader.h>

#include <kdialogbase.h>
#include <klistview.h>
#include <kdebug.h>
#include <kglobal.h>

#include "ghighscores_internal.h"


namespace KExtHighscores
{

//-----------------------------------------------------------------------------
PlayersCombo::PlayersCombo(bool addNoneElement,
                           QWidget *parent, const char *name)
    : QComboBox(parent, name)
{
    const PlayerInfos &p = internal->playerInfos();
    for (uint i = 0; i<p.nbEntries(); i++)
        insertItem(p.prettyName(i));
    insertItem(QString("<") + i18n("all") + '>');
    if (addNoneElement) insertItem(QString("<") + i18n("none") + '>');

    connect(this, SIGNAL(activated(int)), SLOT(activatedSlot(int)));
}

void PlayersCombo::activatedSlot(int i)
{
    const PlayerInfos &p = internal->playerInfos();
    if ( i==(int)p.nbEntries() ) emit allSelected();
    else if ( i==(int)p.nbEntries()+1 ) emit noneSelected();
    else emit playerSelected(i);
}

//-----------------------------------------------------------------------------
AdditionalTab::AdditionalTab(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QVBoxLayout *top = new QVBoxLayout(this, KDialogBase::marginHint(),
                                       KDialogBase::spacingHint());

    QHBoxLayout *hbox = new QHBoxLayout(top);
    QLabel *label = new QLabel(i18n("Select player:"), this);
    hbox->addWidget(label);
    _combo = new PlayersCombo(false, this);
    connect(_combo, SIGNAL(playerSelected(uint)),
            SLOT(playerSelected(uint)));
    connect(_combo, SIGNAL(allSelected()), SLOT(allSelected()));
    hbox->addWidget(_combo);
    hbox->addStretch(1);
}

void AdditionalTab::init()
{
    uint id = internal->playerInfos().id();
    _combo->setCurrentItem(id);
    playerSelected(id);
}

void AdditionalTab::allSelected()
{
    display(internal->playerInfos().nbEntries());
}

QString AdditionalTab::percent(uint n, uint total, bool withBraces)
{
    if ( n==0 || total==0 ) return QString::null;
    QString s =  QString("%1%").arg(100.0 * n / total, 0, 'f', 1);
    return (withBraces ? QString("(") + s + ")" : s);
}


//-----------------------------------------------------------------------------
const char *StatisticsTab::COUNT_LABELS[Nb_Counts] = {
    I18N_NOOP("Total:"), I18N_NOOP("Won:"), I18N_NOOP("Lost:"),
    I18N_NOOP("Black marks:")
};
const char *StatisticsTab::TREND_LABELS[Nb_Trends] = {
    I18N_NOOP("Current:"), I18N_NOOP("Max won:"), I18N_NOOP("Max lost:")
};

StatisticsTab::StatisticsTab(QWidget *parent)
    : AdditionalTab(parent, "statistics_tab")
{
    // construct GUI
    QVBoxLayout *top = static_cast<QVBoxLayout *>(layout());

    QHBoxLayout *hbox = new QHBoxLayout(top);
    QVBoxLayout *vbox = new QVBoxLayout(hbox);
    QVGroupBox *group = new QVGroupBox(i18n("Games Counts"), this);
    vbox->addWidget(group);
    QGrid *grid = new QGrid(3, group);
    grid->setSpacing(KDialogBase::spacingHint());
    for (uint k=0; k<Nb_Counts; k++) {
        (void)new QLabel(i18n(COUNT_LABELS[k]), grid);
        _nbs[k] = new QLabel(grid);
        _percents[k] = new QLabel(grid);
    }

    group = new QVGroupBox(i18n("Trends"), this);
    vbox->addWidget(group);
    grid = new QGrid(2, group);
    grid->setSpacing(KDialogBase::spacingHint());
    for (uint k=0; k<Nb_Trends; k++) {
        (void)new QLabel(i18n(TREND_LABELS[k]), grid);
        _trends[k] = new QLabel(grid);
    }

    hbox->addStretch(1);
    top->addStretch(1);

    // read data
    const PlayerInfos &pi = internal->playerInfos();
    uint nb = pi.nbEntries();
    _data.resize(nb+1);
    for (uint i=0; i<_data.size(); i++) {
        _data[i].count[Total] = pi.item("nb games")->read(i).toUInt();
        _data[i].count[Lost] = pi.item("nb lost games")->read(i).toUInt();
        _data[i].count[BlackMark] =
            pi.item("nb black marks")->read(i).toUInt();
        _data[i].count[Won] = _data[i].count[Total] -
                              _data[i].count[Lost] - _data[i].count[BlackMark];
        _data[i].trend[CurrentTrend] =
            pi.item("current trend")->read(i).toInt();
        _data[i].trend[WonTrend] = pi.item("max won trend")->read(i).toUInt();
        _data[i].trend[LostTrend] =
            -pi.item("max lost trend")->read(i).toUInt();
    }

    for (uint k=0; k<Nb_Counts; k++) _data[nb].count[k] = 0;
    for (uint k=0; k<Nb_Trends; k++) _data[nb].trend[k] = 0;
    for (uint i=0; i<_data.size(); i++) {
        for (uint k=0; k<Nb_Counts; k++)
            _data[nb].count[k] += _data[i].count[k];
        for (uint k=0; k<Nb_Trends; k++)
            _data[nb].trend[k] += _data[i].trend[k];
    }
    for (uint k=0; k<Nb_Trends; k++)
        _data[nb].trend[k] = qRound(double(_data[nb].trend[k]) / _data.size());

    init();
}

QString StatisticsTab::percent(const Data &d, Count count) const
{
    if ( count==Total ) return QString::null;
    return AdditionalTab::percent(d.count[count], d.count[Total], true);
}

void StatisticsTab::display(uint i)
{
    const Data &d = _data[i];
    for (uint k=0; k<Nb_Counts; k++) {
        _nbs[k]->setText(QString::number(d.count[k]));
        _percents[k]->setText(percent(d, (Count)k));
    }
    for (uint k=0; k<Nb_Trends; k++) {
        QString s;
        if ( d.trend[k]>0 ) s = '+';
        _trends[k]->setText(s + QString::number(d.trend[k]));
    }
}

//-----------------------------------------------------------------------------
HistogramTab::HistogramTab(QWidget *parent)
    : AdditionalTab(parent, "histogram_tab")
{
    // construct GUI
    QVBoxLayout *top = static_cast<QVBoxLayout *>(layout());

    _list = new KListView(this);
    _list->setSelectionMode(QListView::NoSelection);
    _list->setItemMargin(3);
    _list->setAllColumnsShowFocus(true);
    _list->setSorting(-1);
    _list->header()->setClickEnabled(false);
    _list->header()->setMovingEnabled(false);
    top->addWidget(_list);

    _list->addColumn(i18n("From"));
    _list->addColumn(i18n("To"));
    _list->addColumn(i18n("Count"));
    _list->addColumn(i18n("Percent"));
    for (uint i=0; i<4; i++) _list->setColumnAlignment(i, AlignRight);
    _list->addColumn(QString::null);

    const Item *sitem = internal->scoreInfos().item("score")->item();
    const QMemArray<uint> &sh = internal->scoreHistogram;
    const PlayerInfos &pi = internal->playerInfos();
    for (uint k=1; k<pi.histoSize(); k++) {
        QString s1 = sitem->pretty(0, sh[k-1]);
        QString s2;
        if ( k==sh.size() ) s2 = "...";
        else if ( sh[k]!=sh[k-1]+1 ) s2 = sitem->pretty(0, sh[k]);
        (void)new KListViewItem(_list, s1, s2);
    }

    // read data
    _counts.resize((pi.histoSize()-1) * (pi.nbEntries()+1));
    _data.resize(pi.nbEntries()+1);
    for (uint i=0; i<=pi.nbEntries(); i++) {
        _data[i].total = 0;
        _data[i].max = 0;
    }
    for (uint k=1; k<pi.histoSize(); k++) {
        _counts[pi.nbEntries()*(pi.histoSize()-1) + (k-1)] = 0;
        for (uint i=0; i<pi.nbEntries(); i++) {
            uint nb = pi.item(pi.histoName(k))->read(i).toUInt();
            _counts[i*(pi.histoSize()-1) + (k-1)] = nb;
            _counts[pi.nbEntries()*(pi.histoSize()-1) + (k-1)] += nb;
            _data[i].total += nb;
            _data[pi.nbEntries()].total += nb;
            if ( k!=pi.histoSize()-1 )
                _data[i].max = kMax(_data[i].max, double(nb) / delta(k));
        }
    }
    for (uint k=1; k<pi.histoSize(); k++)
        if ( k!=pi.histoSize()-1 ) {
            double v = double(_counts[pi.nbEntries() + (k-1)]) / delta(k);
            _data[pi.nbEntries()].max = kMax(_data[pi.nbEntries()].max, v);
        }

    init();
}

uint HistogramTab::delta(uint k) const
{
    const QMemArray<uint> &sh = internal->scoreHistogram;
    return sh[k] - sh[k-1] + 1;
}

void HistogramTab::display(uint i)
{
    const PlayerInfos &pi = internal->playerInfos();
    QListViewItem *item = _list->firstChild();
    for (uint k=pi.histoSize()-1; k>0; k--) {
        uint nb = _counts[i*(pi.histoSize()-1) + (k-1)];
        item->setText(2, QString::number(nb));
        item->setText(3, percent(nb, _data[i].total));
        if ( !internal->scoreBound && k==pi.histoSize()-1 )
            item->setText(4, "--");
        else {
            uint width = (_data[i].max==0 ? 0
                          : qRound(150.0 * nb / delta(k) / _data[i].max));
            QPixmap pixmap(width, 10);
            pixmap.fill(blue);
            item->setPixmap(4, pixmap);
        }
        item = item->nextSibling();
    }
}

}; // namespace
