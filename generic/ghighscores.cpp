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


namespace KExtHighscores
{

Highscores::Highscores(const QString &version, const KURL &baseURL,
                       uint nbGameTypes, uint maxNbEntries,
                       bool trackLostGames, bool trackBlackMarks)
{
    KURL burl = baseURL;
    if ( !baseURL.isEmpty() ) {
        Q_ASSERT( baseURL.isValid() );
        const char *HS_WW_URL = "ww hs url";
        ConfigGroup cg;
        if ( cg.config()->hasKey(HS_WW_URL) )
            burl = cg.config()->readEntry(HS_WW_URL);
        else cg.config()->writeEntry(HS_WW_URL, burl.url());
    }

    d = new HighscoresPrivate(version, burl, nbGameTypes, maxNbEntries,
                              trackLostGames, trackBlackMarks, this);
}

Highscores::~Highscores()
{
    delete d;
}

uint gameType()
{
    HighscoresPrivate::checkFirst();
    return HighscoresPrivate::gameType();
}

void setGameType(uint type)
{
    HighscoresPrivate::setGameType(type);
}

KUIConfigWidget *createConfigurationWidget(QWidget *parent)
{
    HighscoresPrivate::checkFirst();
    return new HighscoresConfigWidget(parent);
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
    HighscoresPrivate::submitScore(score, parent);
}

void showHighscores(QWidget *parent)
{
    HighscoresPrivate::showHighscores(parent, -1);
}

void Highscores::submitLegacyScore(const Score &score) const
{
    HighscoresPrivate::submitLocal(score);
}

bool Highscores::isStrictlyLess(const Score &s1, const Score &s2) const
{
    return s1.score()<s2.score();
}

void Highscores::setItem(const QString &name, Item *item)
{
    if ( name=="score" )
        HighscoresPrivate::scoreInfos().setItem("score", item);
    else if ( name=="mean score" )
        HighscoresPrivate::playerInfos().setItem("mean score", item);
    else if ( name=="best score" )
        HighscoresPrivate::playerInfos().setItem("best score", item);
    else HighscoresPrivate::scoreInfos().addItem(name, item, true);
}

Score lastScore()
{
    return HighscoresPrivate::lastScore();
}

Score firstScore()
{
    return HighscoresPrivate::firstScore();
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
