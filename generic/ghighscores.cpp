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
    KConfigWidget *cw = new KConfigWidget(i18n("Configure Highscores"));
    cw->configCollection()->insert( new ConfigItem(cw) );
    KConfigDialog *cd = new KConfigDialog(cw, parent);
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

void submitScore(const Score &score, QWidget *parent)
{
    internal->submitScore(score, parent);
}

void show(QWidget *parent)
{
    internal->showHighscores(parent, -1);
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

void Manager::setTrackBlackMarks(bool track)
{
    internal->trackBlackMarks = track;
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

void Manager::setScoreHistogram(const QMemArray<uint> &scores, bool bound)
{
    Q_ASSERT( scores.size()>=2 );
    for (uint i=0; i<scores.size()-1; i++)
        Q_ASSERT( scores[i]<scores[i+1] );
    internal->playerInfos().createHistoItems(scores, bound);
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

Score lastScore()
{
    return internal->lastScore();
}

Score firstScore()
{
    return internal->firstScore();
}

QString Manager::gameTypeLabel(uint gameType, LabelType type) const
{
    Q_ASSERT( gameType==0 );
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
