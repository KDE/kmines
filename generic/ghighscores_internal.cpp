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

#include "ghighscores_internal.h"

#include <khighscore.h>
#include <kglobal.h>


namespace KExtHighscores
{

//-----------------------------------------------------------------------------
const char *ItemContainer::ANONYMOUS = "_";

ItemContainer::ItemContainer()
    : _item(0)
{}

ItemContainer::~ItemContainer()
{
    delete _item;
}

void ItemContainer::setItem(Item *item)
{
    delete _item;
    _item = item;
}

QString ItemContainer::entryName() const
{
    if ( _subGroup.isEmpty() ) return _name;
    return _name + "_" + _subGroup;
}

QVariant ItemContainer::read(uint i) const
{
    Q_ASSERT(_item);

    QVariant v = _item->defaultValue();
    if ( isStored() ) {
        KHighscore hs;
        hs.setHighscoreGroup(_group);
        v = hs.readPropertyEntry(i+1, entryName(), v);
    }
    return _item->read(i, v);
}

QString ItemContainer::pretty(uint i) const
{
    Q_ASSERT(_item);
    return _item->pretty(i, read(i));
}

void ItemContainer::write(uint i, const QVariant &value) const
{
    Q_ASSERT( isStored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    hs.writeEntry(i+1, entryName(), value);
}

//-----------------------------------------------------------------------------
ItemArray::ItemArray()
    : _group(""), _subGroup("") // no null groups
{}

ItemArray::~ItemArray()
{
    for (uint i=0; i<size(); i++) delete at(i);
}

int ItemArray::findIndex(const QString &name) const
{
    for (uint i=0; i<size(); i++)
        if ( at(i)->name()==name ) return i;
    return -1;
}

const ItemContainer *ItemArray::item(const QString &name) const
{
    int i = findIndex(name);
    Q_ASSERT( i!=-1 );
    return at(i);
}

void ItemArray::setItem(const QString &name, Item *item)
{
    int i = findIndex(name);
    Q_ASSERT( i!=-1 );
    bool stored = at(i)->isStored();
    bool canHaveSubGroup = at(i)->canHaveSubGroup();
    _setItem(i, name, item, stored, canHaveSubGroup);
}

void ItemArray::addItem(const QString &name, Item *item,
                        bool stored, bool canHaveSubGroup)
{
    Q_ASSERT( findIndex(name)==-1 );
    uint i = size();
    resize(i+1);
    at(i) = new ItemContainer;
    _setItem(i, name, item, stored, canHaveSubGroup);
}

void ItemArray::_setItem(uint i, const QString &name, Item *item,
                         bool stored, bool canHaveSubGroup)
{
    at(i)->setItem(item);
    at(i)->setName(name);
    at(i)->setGroup(stored ? _group : QString::null);
    at(i)->setSubGroup(canHaveSubGroup ? _subGroup : QString::null);
}

void ItemArray::setGroup(const QString &group)
{
    Q_ASSERT( !group.isNull() );
    _group = group;
    for (uint i=0; i<size(); i++)
        if ( at(i)->isStored() ) at(i)->setGroup(group);
}

void ItemArray::setSubGroup(const QString &subGroup)
{
    Q_ASSERT( !_subGroup.isNull() );
    _subGroup = subGroup;
    for (uint i=0; i<size(); i++)
        if ( at(i)->canHaveSubGroup() ) at(i)->setSubGroup(subGroup);
}

//-----------------------------------------------------------------------------
class ScoreNameItem : public NameItem
{
 public:
    ScoreNameItem(const ScoreInfos &score, const PlayerInfos &infos)
        : _score(score), _infos(infos) {}

    QString pretty(uint i, const QVariant &v) const {
        uint id = _score.item("id")->read(i).toUInt();
        if ( id==0 ) return NameItem::pretty(i, v);
        return _infos.prettyName(id-1);
    }

 private:
    const ScoreInfos  &_score;
    const PlayerInfos &_infos;
};

//-----------------------------------------------------------------------------
ScoreInfos::ScoreInfos(uint maxNbEntries, const PlayerInfos &infos)
    : _maxNbEntries(maxNbEntries)
{
    addItem("id", new Item((uint)0));
    addItem("rank", new RankItem, false);
    addItem("name", new ScoreNameItem(*this, infos));
    addItem("score", new ScoreItem);
    addItem("date", new DateItem);
}

uint ScoreInfos::nbEntries() const
{
    uint i = 0;
    for (; i<_maxNbEntries; i++)
        if ( item("score")->read(i).toUInt()==0 ) break;
	return i;
}

//-----------------------------------------------------------------------------
const char *HS_ID              = "player id";
const char *HS_REGISTERED_NAME = "registered name";
const char *HS_KEY             = "player key";
const char *HS_WW_ENABLED      = "ww hs enabled";

PlayerInfos::PlayerInfos(bool trackLostGames, bool trackBlackMarks)
    : _trackLostGames(trackLostGames), _trackBlackMarks(trackBlackMarks)
{
    setGroup("players");

    addItem("name", new NameItem);
    addItem("nb games", new Item((uint)0, i18n("Games count"),
                                 Qt::AlignRight), true, true);
    if (trackLostGames)
        addItem("success", new SuccessPercentageItem, true, true);
    addItem("mean score", new MeanScoreItem, true, true);
    addItem("best score", new BestScoreItem, true, true);
    if (trackBlackMarks)
        addItem("black mark", new Item((uint)0, i18n("Black mark"),
                                       Qt::AlignRight), true, true);
    addItem("date", new DateItem, true, true);
    addItem("comment", new Item(QString::null, i18n("Comment"),
                                Qt::AlignLeft));

    ConfigGroup cg;
    _newPlayer = !cg.config()->hasKey(HS_ID);
    if ( !_newPlayer ) _id = cg.config()->readUnsignedNumEntry(HS_ID);
    else {
        _id = nbEntries();
        cg.config()->writeEntry(HS_ID, _id);
        item("name")->write(_id, QString(ItemContainer::ANONYMOUS));
    }
}

bool PlayerInfos::isAnonymous() const
{
    return ( name()==ItemContainer::ANONYMOUS );
}

uint PlayerInfos::nbEntries() const
{
    KHighscore hs;
    hs.setHighscoreGroup("players");
    QStringList list = hs.readList("name", -1);
    return list.count();
}

QString PlayerInfos::key() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_KEY, QString::null);
}

bool PlayerInfos::isWWEnabled() const
{
    ConfigGroup cg;
    return cg.config()->readBoolEntry(HS_WW_ENABLED, false);
}

void PlayerInfos::submitScore(const Score &score) const
{
    Q_ASSERT( score.type()!=Lost || _trackLostGames );

    if ( score.type()==BlackMark ) {
        Q_ASSERT(_trackBlackMarks);
        uint nb_bm = item("black mark")->read(_id).toUInt();
        item("black mark")->write(_id, nb_bm+1);
        return;
    }

    uint nb = item("nb games")->read(_id).toUInt();
    uint nb_success = nb;
    if (_trackLostGames) {
        double success = item("success")->read(_id).toDouble();
        if ( success!=-1 ) nb_success = (uint)(success * nb / 100);
    }
    double total_score = item("mean score")->read(_id).toDouble() * nb_success;

    nb++;
    if ( score.type()==Won ) {
        nb_success++;
        total_score += score.score();
    }
    double mean = (nb_success==0 ? 0 : total_score / nb_success);

    item("nb games")->write(_id, nb);
    item("mean score")->write(_id, mean);
    if (_trackLostGames) {
        double success = 100.0 * nb_success / nb;
        item("success")->write(_id, success);
    }
    if ( score.score()>item("best score")->read(_id).toUInt() ) {
        item("best score")->write(_id, score.score());
        item("date")->write(_id, score.data("date").toDateTime());
    }
}

void PlayerInfos::modifySettings(const QString &newName,
                                 const QString &comment, bool WWEnabled,
                                 const QString &newKey) const
{
    ConfigGroup cg;
    item("name")->write(_id, newName);
    item("comment")->write(_id, comment);
    cg.config()->writeEntry(HS_WW_ENABLED, WWEnabled);
    if ( !newKey.isEmpty() ) cg.config()->writeEntry(HS_KEY, newKey);
    if (WWEnabled) cg.config()->writeEntry(HS_REGISTERED_NAME, newName);
}

QString PlayerInfos::registeredName() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_REGISTERED_NAME, QString::null);
}



}; // namespace
