/*
    This file is part of the KDE games library
    Copyright (C) 2001-2003 Nicolas Hadacek (hadacek@kde.org)

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

#include "ghighscores_item.h"

#include <qlayout.h>
#include <kglobal.h>
#include <kdialogbase.h>
#include <kdebug.h>

#include "khighscore.h"
#include "ghighscores_internal.h"
#include "ghighscores_gui.h"


namespace KExtHighscore
{

//-----------------------------------------------------------------------------
Item::Item(const QVariant &def, const QString &label, int alignment)
    : _default(def), _label(label), _alignment(alignment),
      _format(NoFormat), _special(NoSpecial)
{}

Item::~Item()
{}

QVariant Item::read(uint, const QVariant &value) const
{
    return value;
}

void Item::setPrettyFormat(Format format)
{
    bool buint = ( _default.type()==QVariant::UInt );
    bool bdouble = ( _default.type()==QVariant::Double );
    bool bnum = ( buint || bdouble || _default.type()==QVariant::Int );

    switch (format) {
    case OneDecimal:
    case Percentage:
        Q_ASSERT(bdouble);
        break;
    case MinuteTime:
        Q_ASSERT(bnum);
        break;
    case DateTime:
    	Q_ASSERT( _default.type()==QVariant::DateTime );
	break;
    case NoFormat:
        break;
    }

    _format = format;
}

void Item::setPrettySpecial(Special special)
{
    bool buint = ( _default.type()==QVariant::UInt );
    bool bdouble = ( _default.type()==QVariant::Double );
    bool bnum = ( buint || bdouble || _default.type()==QVariant::Int );

    switch (special) {
    case ZeroNotDefined:
        Q_ASSERT(bnum);
        break;
    case NegativeNotDefined:
        Q_ASSERT(bnum && !buint);
        break;
    case Anonymous:
        Q_ASSERT( _default.type()==QVariant::String );
        break;
    case NoSpecial:
        break;
    }

     _special = special;
}

QString Item::timeFormat(uint n)
{
    Q_ASSERT( n<=3600 && n!=0 );
    n = 3600 - n;
    return QString::number(n / 60).rightJustify(2, '0') + ':'
        + QString::number(n % 60).rightJustify(2, '0');
}

QString Item::pretty(uint, const QVariant &value) const
{
    switch (_special) {
    case ZeroNotDefined:
        if ( value.toUInt()==0 ) return "--";
        break;
    case NegativeNotDefined:
        if ( value.toInt()<0 ) return "--";
        break;
    case Anonymous:
        if ( value.toString()==ItemContainer::ANONYMOUS )
            return i18n("anonymous");
        break;
    case NoFormat:
        break;
    }

    switch (_format) {
    case OneDecimal:
        return QString::number(value.toDouble(), 'f', 1);
    case Percentage:
        return QString::number(value.toDouble(), 'f', 1) + "%";
    case MinuteTime:
        return timeFormat(value.toUInt());
    case DateTime:
        if ( value.toDateTime().isNull() ) return "--";
        return KGlobal::locale()->formatDateTime(value.toDateTime());
    case NoSpecial:
        break;
    }

    return value.toString();
}

//-----------------------------------------------------------------------------
Score::Score(ScoreType type)
    : _type(type)
{
    const ItemArray &items = internal->scoreInfos();
    for (uint i=0; i<items.size(); i++)
        _data[items[i]->name()] = items[i]->item()->defaultValue();
}

Score::~Score()
{}

const QVariant &Score::data(const QString &name) const
{
    Q_ASSERT( _data.contains(name) );
    return _data[name];
}

void Score::setData(const QString &name, const QVariant &value)
{
    Q_ASSERT( _data.contains(name) );
    Q_ASSERT( _data[name].type()==value.type() );
    _data[name] = value;
}

bool Score::isTheWorst() const
{
    Score s;
    return score()==s.score();
}

bool Score::operator <(const Score &score)
{
    return internal->manager.isStrictlyLess(*this, score);
}

QDataStream &operator <<(QDataStream &s, const Score &score)
{
    s << (Q_UINT8)score.type();
    s << score._data;
    return s;
}

QDataStream &operator >>(QDataStream &s, Score &score)
{
    Q_UINT8 type;
    s >> type;
    score._type = (ScoreType)type;
    s >> score._data;
    return s;
}

//-----------------------------------------------------------------------------
MultiplayerScores::MultiplayerScores()
{}

MultiplayerScores::~MultiplayerScores()
{}

void MultiplayerScores::clear()
{
    Score score;
    for (uint i=0; i<_scores.size(); i++) {
        _nbGames[i] = 0;
        QVariant name = _scores[i].data("name");
        _scores[i] = score;
        _scores[i].setData("name", name);
        _scores[i]._data["mean score"] = double(0);
        _scores[i]._data["nb won games"] = uint(0);
    }
}

void MultiplayerScores::setPlayerCount(uint nb)
{
    _nbGames.resize(nb);
    _scores.resize(nb);
    clear();
}

void MultiplayerScores::setName(uint i, const QString &name)
{
    _scores[i].setData("name", name);
}

void MultiplayerScores::addScore(uint i, const Score &score)
{
    QVariant name = _scores[i].data("name");
    double mean = _scores[i].data("mean score").toDouble();
    uint won = _scores[i].data("nb won games").toUInt();
    _scores[i] = score;
    _scores[i].setData("name", name);
    _nbGames[i]++;
    mean += (double(score.score()) - mean) / _nbGames[i];
    _scores[i]._data["mean score"] = mean;
    if ( score.type()==Won ) won++;
    _scores[i]._data["nb won games"] = won;
}

void MultiplayerScores::show(QWidget *parent)
{
    // check consistency
    if ( _nbGames.size()<2 ) kdWarning(11002) << "less than 2 players" << endl;
    else {
        bool ok = true;
        uint nb = _nbGames[0];
        for (uint i=1; i<_nbGames.size(); i++)
            if ( _nbGames[i]!=nb ) ok = false;
        if (!ok)
           kdWarning(11002) << "players have not same number of games" << endl;
    }

    // order the players according to the number of won games
    QValueVector<Score> ordered;
    for (uint i=0; i<_scores.size(); i++) {
        uint won = _scores[i].data("nb won games").toUInt();
        double mean = _scores[i].data("mean score").toDouble();
        QValueVector<Score>::iterator it;
        for(it = ordered.begin(); it!=ordered.end(); ++it) {
            uint cwon = (*it).data("nb won games").toUInt();
            double cmean = (*it).data("mean score").toDouble();
            if ( won<cwon || (won==cwon && mean<cmean) ) {
                ordered.insert(it, _scores[i]);
                break;
            }
        }
        if ( it==ordered.end() ) ordered.push_back(_scores[i]);
    }

    // show the scores
    KDialogBase dialog(KDialogBase::Plain, i18n("Multiplayers Scores"),
                       KDialogBase::Close, KDialogBase::Close,
                       parent, "show_multiplayers_score", true, true);
    QHBoxLayout *hbox = new QHBoxLayout(dialog.plainPage(),
                                KDialog::marginHint(), KDialog::spacingHint());

    QVBox *vbox = new QVBox(dialog.plainPage());
    hbox->addWidget(vbox);
    if ( _nbGames[0]==0 ) (void)new QLabel(i18n("No game played !"), vbox);
    else {
        (void)new QLabel(i18n("Scores for last game:"), vbox);
        (void)new LastMultipleScoresList(ordered, vbox);
    }

    if ( _nbGames[0]>1 ) {
        vbox = new QVBox(dialog.plainPage());
        hbox->addWidget(vbox);
        (void)new QLabel(i18n("Scores for the last %1 games:")
                         .arg(_nbGames[0]), vbox);
        (void)new TotalMultipleScoresList(ordered, vbox);
    }

    dialog.enableButtonSeparator(false);
    dialog.exec();
}

QDataStream &operator <<(QDataStream &s, const MultiplayerScores &score)
{
    s << score._scores;
    s << score._nbGames;
    return s;
}

QDataStream &operator >>(QDataStream &s, MultiplayerScores &score)
{
    s >> score._scores;
    s >> score._nbGames;
    return s;
}

}; // namespace
