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

#include "ghighscores.h"

#include <qlayout.h>

#include <kmdcodec.h>
#include <kiconloader.h>
#include <kstaticdeleter.h>

#include "ghighscores_internal.h"
#include "ghighscores_gui.h"


namespace KExtHighscores
{

Highscores *Highscores::_highscores = 0L;

static KStaticDeleter<Highscores> sd;
const char *HS_WW_URL = "ww hs url";

Highscores::Highscores(const QString &version, const KURL &baseURL,
                       uint nbGameTypes, uint maxNbEntries,
                       bool trackLostGames, bool trackBlackMarks)
    : _nbGameTypes(nbGameTypes), _gameType(0), _first(true)
{
    Q_ASSERT(nbGameTypes);
    Q_ASSERT(maxNbEntries);
    if (_highscores) qFatal("A highscore object already exists");
    sd.setObject(_highscores, this);

    KURL burl = baseURL;
    if ( !baseURL.isEmpty() ) {
        Q_ASSERT( baseURL.isValid() );
        ConfigGroup cg;
        if ( cg.config()->hasKey(HS_WW_URL) )
            burl = cg.config()->readEntry(HS_WW_URL);
        else cg.config()->writeEntry(HS_WW_URL, burl.url());
    }

    d = new HighscoresPrivate(version, burl, maxNbEntries,
                              trackLostGames, trackBlackMarks);
}

Highscores::~Highscores()
{
    delete d;
    // in case the destructor is called explicitely
    sd.setObject(_highscores, 0, false);
}

void Highscores::setGameType(uint type)
{
    if (_first) {
        _first = false;
        if ( HighscoresPrivate::playerInfos().isNewPlayer() ) {
            for (uint i=0; i<_nbGameTypes; i++) {
                setGameType(i);
                convertLegacy(i);
            }
        }
    }

    Q_ASSERT( type<_nbGameTypes );
    _gameType = type;
    QString label = gameTypeLabel(_gameType, Standard);
    QString str = "scores";
    if ( !label.isEmpty() ) str += "_" + label;
    HighscoresPrivate::scoreInfos().setGroup(str);
    if ( !label.isEmpty() )
        HighscoresPrivate::playerInfos().setSubGroup(label);
}

KSettingWidget *Highscores::createSettingsWidget(QWidget *parent)
{
    if (_first) setGameType(0);
    return new HighscoresSettingsWidget(parent);
}

void Highscores::_showHighscores(QWidget *parent, int rank)
{
    uint tmp = _gameType;
    int face = (_nbGameTypes==1 ? KDialogBase::Plain : KDialogBase::TreeList);
    KDialogBase hs(face, i18n("Highscores"),
                   KDialogBase::Close, KDialogBase::Close,
                   parent, "show_highscores", true, true);
    for (uint i=0; i<_nbGameTypes; i++) {
        setGameType(i);
        QWidget *w;
        if ( _nbGameTypes==1 ) w = hs.plainPage();
        else w = hs.addPage(gameTypeLabel(i, I18N), QString::null,
                            BarIcon(gameTypeLabel(i, Icon), KIcon::SizeLarge));
        QVBoxLayout *vbox = new QVBoxLayout(w);

        QString typeLabel = (_nbGameTypes>1 ? gameTypeLabel(_gameType, WW)
                             : QString::null);
        w = new HighscoresWidget((i==tmp ? rank : -1), w, hs.spacingHint(),
                                 typeLabel);
        vbox->addWidget(w);
    }
    setGameType(tmp);

    hs.resize( hs.calculateSize(500, 370) ); // hacky
    hs.showPage(_gameType);
    if ( _nbGameTypes==1 ) hs.enableButtonSeparator(false);
	hs.exec();
}

void Highscores::showMultipleScores(const QValueList<Score> &scores,
                                    QWidget *parent) const
{
    KDialogBase dialog(KDialogBase::Plain, i18n("Multiplayers scores"),
                       KDialogBase::Close, KDialogBase::Close,
                       parent, "show_multiplayers_score", true, true);
    QVBoxLayout *vbox = new QVBoxLayout(dialog.plainPage());
    QWidget *list = new MultipleScoresList(scores, dialog.plainPage());
    vbox->addWidget(list);
    dialog.enableButtonSeparator(false);
    dialog.exec();
}

void Highscores::submitScore(const Score &ascore, QWidget *parent)
{
    if (_first) setGameType(0);

    Score score = ascore;
    score.setData("id", HighscoresPrivate::playerInfos().id() + 1);
    score.setData("date", QDateTime::currentDateTime());

    HighscoresPrivate::playerInfos().submitScore(score);
    if ( HighscoresPrivate::playerInfos().isWWEnabled() )
        submitWorldWide(score, parent);

    if ( score.type()==Won ) {
        int rank = submitLocal(score);
        if ( rank!=-1 ) _showHighscores(parent, rank);
    }
}

int Highscores::submitLocal(const Score &score) const
{
    int r = HighscoresPrivate::rank(score);
    if ( r!=-1 ) {
        uint nb = HighscoresPrivate::scoreInfos().nbEntries();
        if ( nb<HighscoresPrivate::scoreInfos().maxNbEntries() ) nb++;
        score.write(r, nb);
    }
    return r;
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

Score Highscores::lastScore()
{
    if (_first) setGameType(0);
    Score score(Won);
    score.read(HighscoresPrivate::scoreInfos().maxNbEntries() - 1);
    return score;
}

Score Highscores::firstScore()
{
    if (_first) setGameType(0);
    Score score(Won);
    score.read(0);
    return score;
}

bool Highscores::submitWorldWide(const Score &score, QWidget *parent) const
{
    const PlayerInfos &p = HighscoresPrivate::playerInfos();
    KURL url = HighscoresPrivate::queryURL(HighscoresPrivate::Submit,
                                       p.registeredName());
    additionnalQueryItems(url, score);
    HighscoresPrivate::addToQueryURL(url, "key", p.key());
    HighscoresPrivate::addToQueryURL(url, "version",
                                     HighscoresPrivate::version());
    int s = (score.type()==Won ? score.score() : (int)score.type());
    QString str =  QString::number(s);
    HighscoresPrivate::addToQueryURL(url, "score", str);
    KMD5 context(QString(p.registeredName() + str).latin1());
    HighscoresPrivate::addToQueryURL(url, "check", context.hexDigest());
    HighscoresPrivate::addToQueryURL(url, "level",
                                     gameTypeLabel(_gameType, WW));

    bool ok;
    HighscoresPrivate::doQuery(url, parent, ok);
    return ok;
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
                               const QString &content) const
{
    HighscoresPrivate::addToQueryURL(url, item, content);
}

}; // namescape
