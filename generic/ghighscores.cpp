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

#include "ghighscores.h"

#include <qlayout.h>

#include "ghighscores_internal.h"
#include "ghighscores_gui.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
ManagerPrivate *internal = 0;

uint gameType()
{
    internal->checkFirst();
    return internal->gameType();
}

void setGameType(uint type)
{
    internal->setGameType(type);
}

bool configure(QWidget *parent)
{
    internal->checkFirst();
    ConfigDialog *cd = new ConfigDialog(parent);
    cd->exec();
    bool saved = cd->hasBeenSaved();
    delete cd;
    return saved;
}

void showMultipleScores(const ScoreVector &scores, QWidget *parent)
{
    KDialogBase dialog(KDialogBase::Plain, i18n("Multiplayers Scores"),
                       KDialogBase::Close, KDialogBase::Close,
                       parent, "show_multiplayers_score", true, true);
    QVBoxLayout *vbox = new QVBoxLayout(dialog.plainPage());
    QWidget *list = new MultipleScoresList(scores, dialog.plainPage());
    vbox->addWidget(list);
    dialog.enableButtonSeparator(false);
    dialog.exec();
}

void show(QWidget *parent, int rank)
{
    HighscoresDialog *hd = new HighscoresDialog(rank, parent);
	hd->exec();
    delete hd;
}

void submitScore(const Score &score, QWidget *parent)
{
    int rank = internal->submitScore(score, parent);

    switch (internal->showMode) {
    case Manager::AlwaysShow:
        show(parent, -1);
        break;
    case Manager::ShowForHigherScore:
        if ( rank!=-1) show(parent, rank);
        break;
    case Manager::ShowForHighestScore:
        if ( rank==0 )
            show(parent, rank);
        break;
    case Manager::NeverShow:
        break;
    }
}

void show(QWidget *parent)
{
    internal->checkFirst();
    show(parent, -1);
}

Score lastScore()
{
    internal->checkFirst();
    Score score(Won);
    uint nb = internal->scoreInfos().maxNbEntries();
    internal->scoreInfos().read(nb - 1, score);
    return score;
}

Score firstScore()
{
    internal->checkFirst();
    Score score(Won);
    internal->scoreInfos().read(0, score);
    return score;
}


//-----------------------------------------------------------------------------
Manager::Manager(uint nbGameTypes, uint maxNbEntries)
{
    Q_ASSERT(nbGameTypes);
    Q_ASSERT(maxNbEntries);
    if (internal) qFatal("A highscore object already exists");
    internal = new ManagerPrivate(nbGameTypes, maxNbEntries, *this);
}

Manager::~Manager()
{
    delete internal;
    internal = 0;
}

void Manager::setTrackLostGames(bool track)
{
    internal->trackLostGames = track;
}

void Manager::showStatistics(bool show)
{
    internal->showStatistics = show;
}

void Manager::setWWHighscores(const KURL &url, const QString &version)
{
    Q_ASSERT( url.isValid() );
    internal->serverURL = url;
    const char *HS_WW_URL = "ww hs url";
    ConfigGroup cg;
    if ( cg.config()->hasKey(HS_WW_URL) )
        internal->serverURL = cg.config()->readEntry(HS_WW_URL);
    else cg.config()->writeEntry(HS_WW_URL, url.url());
    internal->version = version;
}

void Manager::setScoreHistogram(const QMemArray<uint> &scores,
                                ScoreTypeBound type)
{
    Q_ASSERT( scores.size()>=2 );
    for (uint i=0; i<scores.size()-1; i++)
        Q_ASSERT( scores[i]<scores[i+1] );
    internal->playerInfos().createHistoItems(scores, type==ScoreBound);
}

void Manager::setShowMode(ShowMode mode)
{
    internal->showMode = mode;
}

void Manager::setScoreType(ScoreType type)
{
    switch (type) {
    case Normal:
        return;
    case MinuteTime: {
        ScoreItem *scoreItem = new ScoreItem;
        scoreItem->setPrettyFormat(Item::MinuteTime);
        setScoreItem("score", scoreItem);

        MeanScoreItem *meanScoreItem = new MeanScoreItem;
        meanScoreItem->setPrettyFormat(Item::MinuteTime);
        setPlayerItem("mean score", meanScoreItem);

        BestScoreItem *bestScoreItem = new BestScoreItem;
        bestScoreItem->setPrettyFormat(Item::MinuteTime);
        setPlayerItem("best score", bestScoreItem);
        return;
    }
    }
}

void Manager::submitLegacyScore(const Score &score) const
{
    internal->submitLocal(score);
}

bool Manager::isStrictlyLess(const Score &s1, const Score &s2) const
{
    return s1.score()<s2.score();
}

void Manager::setScoreItem(const QString &name, Item *item)
{
    if ( name=="score" ) internal->scoreInfos().setItem("score", item);
    else internal->scoreInfos().addItem(name, item, true);
}

void Manager::setPlayerItem(const QString &name, Item *item)
{
    internal->playerInfos().setItem(name, item);
}

QString Manager::gameTypeLabel(uint gameType, LabelType type) const
{
    if ( gameType!=0 )
        qFatal("You need to reimplement KExtHighscore::Manager for "
               "multiple game types");
    switch (type) {
    case Icon:
    case Standard:
    case I18N:     break;
    case WW:       return "normal";
    }
    return QString::null;
};

void Manager::addToQueryURL(KURL &url, const QString &item,
                               const QString &content)
{
    Q_ASSERT( !item.isEmpty() && url.queryItem(item).isNull() );

    QString query = url.query();
    if ( !query.isEmpty() ) query += '&';
	query += item + '=' + KURL::encode_string(content);
	url.setQuery(query);
}

}; // namescape
