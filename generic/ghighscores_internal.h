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

#ifndef KEXTHIGHSCORE_INTERNAL_H
#define KEXTHIGHSCORE_INTERNAL_H

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kurl.h>

#include "ghighscores.h"

class QTextStream;
class QTabWidget;
class QDomNamedNodeMap;


namespace KExtHighscore
{

class PlayerInfos;
class Score;
class Manager;


//-----------------------------------------------------------------------------
class RankItem : public Item
{
 public:
    RankItem()
        : Item((uint)0, i18n("Rank"), Qt::AlignRight) {}

    QVariant read(uint rank, const QVariant &) const  { return rank; }
    QString pretty(uint rank, const QVariant &) const
        { return QString::number(rank+1); }
};

class NameItem : public Item
{
 public:
    NameItem()
        : Item(QString::null, i18n("Name"), Qt::AlignLeft) {
            setPrettySpecial(Anonymous);
    }
};

class DateItem : public Item
{
 public:
    DateItem()
        : Item(QDateTime(), i18n("Date"), Qt::AlignRight) {
            setPrettyFormat(DateTime);
    }
};

class SuccessPercentageItem : public Item
{
 public:
    SuccessPercentageItem()
        : Item((double)-1, i18n("Success"), Qt::AlignRight) {
            setPrettyFormat(Percentage);
            setPrettySpecial(NegativeNotDefined);
    }
};

//-----------------------------------------------------------------------------
class ItemContainer
{
 public:
    ItemContainer();
    ~ItemContainer();

    void setItem(Item *item);
    const Item *item() const { return _item; }

    void setName(const QString &name) { _name = name; }
    QString name() const { return _name; }

    void setGroup(const QString &group) { _group = group; }
    bool isStored() const { return !_group.isNull(); }

    void setSubGroup(const QString &subGroup) { _subGroup = subGroup; }
    bool canHaveSubGroup() const { return !_subGroup.isNull(); }

    static const char *ANONYMOUS; // name assigned to anonymous players

    QVariant read(uint i) const;
    QString pretty(uint i) const;
    void write(uint i, const QVariant &value) const;
    uint increment(uint i) const; // for UInt QVariant (return new value)

 private:
    Item    *_item;
    QString  _name, _group, _subGroup;

    QString entryName() const;

    ItemContainer(const ItemContainer &);
    ItemContainer &operator =(const ItemContainer &);
};

//-----------------------------------------------------------------------------
/**
 * Manage a bunch of @ref Item which are saved under the same group
 * in KHighscores config file.
 */
class ItemArray : public QMemArray<ItemContainer *>
{
 public:
    ItemArray();
    virtual ~ItemArray();

    virtual uint nbEntries() const = 0;

    const ItemContainer *item(const QString &name) const;

    void addItem(const QString &name, Item *, bool stored = true,
                 bool canHaveSubGroup = false);
    void setItem(const QString &name, Item *);
    int findIndex(const QString &name) const;

    void setGroup(const QString &group);
    void setSubGroup(const QString &subGroup);

    void read(uint k, Score &data) const;
    void write(uint k, const Score &data, uint maxNbLines) const;

    void exportToText(QTextStream &) const;

 private:
    QString _group, _subGroup;

    void _setItem(uint i, const QString &name, Item *, bool stored,
                  bool canHaveSubGroup);

    ItemArray(const ItemArray &);
    ItemArray &operator =(const ItemArray &);
};

//-----------------------------------------------------------------------------
class ScoreInfos : public ItemArray
{
 public:
    ScoreInfos(uint maxNbEntries, const PlayerInfos &infos);

    uint nbEntries() const;
    uint maxNbEntries() const { return _maxNbEntries; }

 private:
    uint _maxNbEntries;
};

//-----------------------------------------------------------------------------
class ConfigGroup : public KConfigGroupSaver
{
 public:
    ConfigGroup() : KConfigGroupSaver(kapp->config(), QString::null) {}
};

//-----------------------------------------------------------------------------
class PlayerInfos : public ItemArray
{
 public:
    PlayerInfos();

    bool isNewPlayer() const { return _newPlayer; }
    uint nbEntries() const;
    QString name() const { return item("name")->read(_id).toString(); }
    bool isAnonymous() const;
    QString prettyName() const        { return prettyName(_id); }
    QString prettyName(uint id) const { return item("name")->pretty(id); }
    QString registeredName() const;
    QString comment() const { return item("comment")->pretty(_id); }
    bool isWWEnabled() const;
    QString key() const;
    uint id() const { return _id; }

    void createHistoItems(const QMemArray<uint> &scores, bool bound);
    QString histoName(uint i) const;
    uint histoSize() const;
    const QMemArray<uint> &histogram() const { return _histogram; }

    void submitScore(const Score &) const;
    void modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, const QString &newKey) const;
    void removeKey();

 private:
    bool _newPlayer, _bound;
    uint _id;
    QMemArray<uint> _histogram;
};

//-----------------------------------------------------------------------------
class ManagerPrivate
{
 public:
    ManagerPrivate(uint nbGameTypes, uint maxNbentries, Manager &manager);
    ~ManagerPrivate();

    bool modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, QWidget *parent);

    void setGameType(uint type);
    void checkFirst();
    int submitLocal(const Score &score);
    int submitScore(const Score &score, QWidget *parent);
    Score readScore(uint i) const;

    uint gameType() const            { return _gameType; }
    uint nbGameTypes() const         { return _nbGameTypes; }
    bool isWWHSAvailable() const     { return !serverURL.isEmpty(); }
    ScoreInfos &scoreInfos() const   { return *_scoreInfos; }
    PlayerInfos &playerInfos() const { return *_playerInfos; }
    enum QueryType { Submit, Register, Change, Players, Scores };
    KURL queryURL(QueryType type, const QString &newName=QString::null) const;

    void exportHighscores(QTextStream &);

    Manager &manager;
    KURL     serverURL;
    QString  version;
    bool     showStatistics, trackLostGames;
    Manager::ShowMode showMode;

 private:
    PlayerInfos  *_playerInfos;
    ScoreInfos   *_scoreInfos;
    bool          _first;
    const uint    _nbGameTypes;
    uint          _gameType;

    // return -1 if not a local best score
    int rank(const Score &score) const;

    bool submitWorldWide(const Score &score, QWidget *parent) const;
    static bool doQuery(const KURL &url, QWidget *parent,
                        QDomNamedNodeMap *map = 0);
    static bool getFromQuery(const QDomNamedNodeMap &map, const QString &name,
                             QString &value, QWidget *parent);
};

}; // namespace

#endif
