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
#include "ghighscores.moc"

#include <qlayout.h>

#include "ghighscores_internal.h"
#include "ghighscores_gui.h"


namespace KExtHighscores
{

Highscores::Highscores(const QString &version, const KURL &baseURL,
                       uint nbGameTypes, uint maxNbEntries)
{
    Q_ASSERT(nbGameTypes);
    Q_ASSERT(maxNbEntries);

    KURL burl = baseURL;
    if ( !baseURL.isEmpty() ) {
        Q_ASSERT( baseURL.isValid() );
        const char *HS_WW_URL = "ww hs url";
        ConfigGroup cg;
        if ( cg.config()->hasKey(HS_WW_URL) )
            burl = cg.config()->readEntry(HS_WW_URL);
        else cg.config()->writeEntry(HS_WW_URL, baseURL.url());
    }

    (void)new HighscoresPrivate(version, burl, nbGameTypes, maxNbEntries,
                                *this);
}

Highscores::~Highscores()
{
    delete internal;
}

uint gameType()
{
    internal->checkFirst();
    return internal->gameType();
}

void setGameType(uint type)
{
    internal->setGameType(type);
}

ConfigWidget *createConfigWidget(QWidget *parent)
{
    internal->checkFirst();
    return new ImplConfigWidget(parent);
}

void showMultipleScores(const QValueList<Score> &scores, QWidget *parent)
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

void showHighscores(QWidget *parent)
{
    internal->showHighscores(parent, -1);
}

void Highscores::setTrackLostGames(bool track)
{
    internal->trackLostGames = track;
}

void Highscores::setTrackBlackMarks(bool track)
{
    internal->trackBlackMarks = track;
}

void Highscores::showStatistics(bool show)
{
    internal->showStatistics = show;
}

void Highscores::setScoreHistogram(const QMemArray<uint> &scores, bool bound)
{
    Q_ASSERT( internal->scoreHistogram.size()==0 );
    Q_ASSERT( scores.size()>=2 );
    for (uint i=0; i<scores.size()-1; i++)
        Q_ASSERT( scores[i]<scores[i+1] );
    internal->scoreHistogram = scores;
    internal->scoreBound = bound;
    internal->playerInfos().createHistoItems();
}

void Highscores::submitLegacyScore(const Score &score) const
{
    internal->submitLocal(score);
}

bool Highscores::isStrictlyLess(const Score &s1, const Score &s2) const
{
    return s1.score()<s2.score();
}

void Highscores::setItem(const QString &name, Item *item)
{
    if ( name=="score" ) internal->scoreInfos().setItem("score", item);
    else if ( name=="mean score" )
        internal->playerInfos().setItem("mean score", item);
    else if ( name=="best score" )
        internal->playerInfos().setItem("best score", item);
    else internal->scoreInfos().addItem(name, item, true);
}

Score lastScore()
{
    return internal->lastScore();
}

Score firstScore()
{
    return internal->firstScore();
}

QString Highscores::gameTypeLabel(uint gameType, LabelType type) const
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

void Highscores::addToQueryURL(KURL &url, const QString &item,
                               const QString &content)
{
    Q_ASSERT( !item.isEmpty() && url.queryItem(item).isNull() );

    QString query = url.query();
    if ( !query.isEmpty() ) query += '&';
	query += item + '=' + KURL::encode_string(content);
	url.setQuery(query);
}

}; // namescape
