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

#include "ghighscores_internal.h"

#include <qfile.h>
#include <qlayout.h>

#include <kglobal.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kmdcodec.h>

#include "khighscore.h"
#include "ghighscores.h"
#include "ghighscores_gui.h"


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

void ItemArray::read(uint k, DataArray &data) const
{
    for (uint i=0; i<size(); i++) {
        if ( !at(i)->isStored() ) continue;
        data.setData(at(i)->name(), at(i)->read(k));
    }
}

void ItemArray::write(uint k, const DataArray &data, uint nb) const
{
    for (uint i=0; i<size(); i++) {
        if ( !at(i)->isStored() ) continue;
        for (uint j=nb-1; j>k; j--)  at(i)->write(j, at(i)->read(j-1));
        at(i)->write(k, data.data(at(i)->name()));
    }
}

void ItemArray::exportToText(QTextStream &s) const
{
    for (uint k=0; k<nbEntries()+1; k++) {
        for (uint i=0; i<size(); i++) {
            const Item *item = at(i)->item();
            if ( item->isVisible() ) {
                if ( i!=0 ) s << '\t';
                if ( k==0 ) s << item->label();
                else s << at(i)->pretty(k-1);
            }
        }
        s << endl;
    }
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
        if ( item("score")->read(i)==item("score")->item()->defaultValue() )
            break;
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
    item("name")->write(_id, newName);
    item("comment")->write(_id, comment);
    ConfigGroup cg;
    cg.config()->writeEntry(HS_WW_ENABLED, WWEnabled);
    if ( !newKey.isEmpty() ) cg.config()->writeEntry(HS_KEY, newKey);
    if (WWEnabled) cg.config()->writeEntry(HS_REGISTERED_NAME, newName);
}

QString PlayerInfos::registeredName() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_REGISTERED_NAME, QString::null);
}

void PlayerInfos::removeKey()
{
    ConfigGroup cg;

    // save old key/nickname
    uint i = 0;
    QString str = "%1 old #%2";
    QString sk;
    do {
        i++;
        sk = str.arg(HS_KEY).arg(i);
    } while ( !cg.config()->readEntry(sk, QString::null).isEmpty() );
    cg.config()->writeEntry(sk, key());
    cg.config()->writeEntry(str.arg(HS_REGISTERED_NAME).arg(i),
                            registeredName());

    // clear current key/nickname
    cg.config()->deleteEntry(HS_KEY);
    cg.config()->deleteEntry(HS_REGISTERED_NAME);
    cg.config()->writeEntry(HS_WW_ENABLED, false);
}

//-----------------------------------------------------------------------------
KURL *HighscoresPrivate::_baseURL = 0;
QString *HighscoresPrivate::_version = 0;
PlayerInfos *HighscoresPrivate::_playerInfos = 0;
ScoreInfos *HighscoresPrivate::_scoreInfos = 0;
bool HighscoresPrivate::_first = true;
uint HighscoresPrivate::_nbGameTypes;
uint HighscoresPrivate::_gameType = 0;
Highscores *HighscoresPrivate::_highscores = 0;

static KStaticDeleter<Highscores> sd;

HighscoresPrivate::HighscoresPrivate(const QString &version, const KURL &burl,
                      uint nbGameTypes, uint maxNbEntries, bool trackLostGames,
                      bool trackBlackMarks, Highscores *highscores)
{
    Q_ASSERT(nbGameTypes);
    _nbGameTypes = nbGameTypes;
    Q_ASSERT(maxNbEntries);
    if (_highscores) qFatal("A highscore object already exists");
    sd.setObject(_highscores, highscores);

    _baseURL = new KURL(burl);
    _version = new QString(version);
    _playerInfos = new PlayerInfos(trackLostGames, trackBlackMarks);
    _scoreInfos = new ScoreInfos(maxNbEntries, *_playerInfos);
}

HighscoresPrivate::~HighscoresPrivate()
{
    delete _scoreInfos;
    delete _playerInfos;
    delete _baseURL;
    delete _version;
    sd.setObject(_highscores, 0, false);
}

KURL HighscoresPrivate::queryURL(QueryType type, const QString &newName)
{
    KURL url = *_baseURL;
    QString nameItem = "nickname";
    QString name = _playerInfos->registeredName();
    bool version = true;
    bool key = false;
    bool level = false;

	switch (type) {
        case Submit:
            url.addPath("submit.php");
            level = true;
            key = true;
            break;
        case Register:
            url.addPath("register.php");
            name = newName;
            break;
        case Change:
            url.addPath("change.php");
            key = true;
            if ( newName!=name )
                Highscores::addToQueryURL(url, "new_nickname", newName);
            break;
        case Players:
            url.addPath("players.php");
            nameItem = "highlight";
            version = false;
            break;
        case Scores:
            url.addPath("highscores.php");
            version = false;
            if ( _nbGameTypes>1 ) level = true;
            break;
	}

    if (version) Highscores::addToQueryURL(url, "version", *_version);
    if ( !name.isEmpty() ) Highscores::addToQueryURL(url, nameItem, name);
    if (key) Highscores::addToQueryURL(url, "key", _playerInfos->key());
    if (level) {
        QString label = _highscores->gameTypeLabel(_gameType, Highscores::WW);
        if ( !label.isEmpty() ) Highscores::addToQueryURL(url, "level", label);
    }

    return url;
}

// strings that needs to be translated (coming from the highscores server)
const char *DUMMY_STRINGS[] = {
    I18N_NOOP("Undefined error."),
    I18N_NOOP("Missing argument(s)."),
    I18N_NOOP("Invalid argument(s)."),

    I18N_NOOP("Unable to connect to MySQL server."),
    I18N_NOOP("Unable to select database."),
    I18N_NOOP("Error on database query."),
    I18N_NOOP("Error on database insert."),

    I18N_NOOP("Nickname already registered."),
    I18N_NOOP("Nickname not registered."),
    I18N_NOOP("Invalid key."),
    I18N_NOOP("Invalid submit key."),

    I18N_NOOP("Invalid level."),
    I18N_NOOP("Invalid score.")
};

const char *UNABLE_TO_CONTACT =
    I18N_NOOP("Unable to contact world-wide highscore server");

bool HighscoresPrivate::doQuery(const KURL &url, QWidget *parent,
                                QDomNamedNodeMap *map)
{
    KIO::http_update_cache(url, true, 0); // remove cache !

    QString tmpFile;
    if ( !KIO::NetAccess::download(url, tmpFile) ) {
        QString details = i18n("Server URL: %1").arg(url.host());
        KMessageBox::detailedSorry(parent, i18n(UNABLE_TO_CONTACT), details);
        return false;
    }

	QFile file(tmpFile);
	if ( !file.open(IO_ReadOnly) ) {
        KIO::NetAccess::removeTempFile(tmpFile);
        QString details = i18n("Unable to open temporary file.");
        KMessageBox::detailedSorry(parent, i18n(UNABLE_TO_CONTACT), details);
        return false;
    }

	QTextStream t(&file);
	QString content = t.read().stripWhiteSpace();
	file.close();
    KIO::NetAccess::removeTempFile(tmpFile);

	QDomDocument doc;
    if ( doc.setContent(content) ) {
        QDomElement root = doc.documentElement();
        QDomElement element = root.firstChild().toElement();
        if ( element.tagName()=="success" ) {
            if (map) *map = element.attributes();
            return true;
        }
        if ( element.tagName()=="error" ) {
            QDomAttr attr = element.attributes().namedItem("label").toAttr();
            if ( !attr.isNull() ) {
                QString msg = i18n(attr.value().latin1());
                QString caption = i18n("Message from world-wide highscores "
                                       "server");
                KMessageBox::sorry(parent, msg, caption);
                return false;
            }
        }
    }
    QString msg = i18n("Invalid answer from world-wide highscores server.");
    QString details = i18n("Raw message: %1").arg(content);
    KMessageBox::detailedSorry(parent, msg, details);
    return false;
}

bool HighscoresPrivate::getFromQuery(const QDomNamedNodeMap &map,
                                     const QString &name, QString &value,
                                     QWidget *parent)
{
    QDomAttr attr = map.namedItem(name).toAttr();
    if ( attr.isNull() ) {
	    KMessageBox::sorry(parent,
               i18n("Invalid answer from world-wide "
                    "highscores server (missing item: %1).").arg(name));
		return false;
	}
	value = attr.value();
	return true;
}

int HighscoresPrivate::rank(const Score &score)
{
    Score tmp(Won);
    uint nb = _scoreInfos->nbEntries();
    uint i = 0;
	for (; i<nb; i++) {
        _scoreInfos->read(i, tmp);
		if ( tmp<score ) break;
    }
	return (i<_scoreInfos->maxNbEntries() ? (int)i : -1);
}

bool HighscoresPrivate::modifySettings(const QString &newName,
                                       const QString &comment, bool WWEnabled,
                                       QWidget *parent)
{
    if ( newName.isEmpty() ) {
        KMessageBox::sorry(parent,i18n("Please choose a non empty nickname."));
	    return false;
	}

    QString newKey;
    bool newPlayer = false;

    if (WWEnabled) {
        newPlayer = _playerInfos->key().isEmpty()
                    || _playerInfos->registeredName().isEmpty();
        KURL url = queryURL((newPlayer ? Register : Change), newName);
        Highscores::addToQueryURL(url, "comment", comment);

        QDomNamedNodeMap map;
        bool ok = doQuery(url, parent, &map);
        if ( !ok || (newPlayer && !getFromQuery(map, "key", newKey, parent)) )
            return false;
    }

    _playerInfos->modifySettings(newName, comment, WWEnabled, newKey);
    return true;
}

void HighscoresPrivate::setGameType(uint type)
{
    if (_first) {
        _first = false;
        if ( _playerInfos->isNewPlayer() ) {
            for (uint i=0; i<_nbGameTypes; i++) {
                setGameType(i);
                _highscores->convertLegacy(i);
            }
        }
    }

    Q_ASSERT( type<_nbGameTypes );
    _gameType = kMin(type, _nbGameTypes-1);
    QString str = "scores";
    QString lab = _highscores->gameTypeLabel(_gameType, Highscores::Standard);
    if ( !lab.isEmpty() ) {
        _playerInfos->setSubGroup(lab);
        str += "_" + lab;
    }
    _scoreInfos->setGroup(str);
}

void HighscoresPrivate::checkFirst()
{
    if (_first) setGameType(0);
}

void HighscoresPrivate::showHighscores(QWidget *parent, int rank)
{
    uint tmp = _gameType;

    bool treeList = ( _nbGameTypes!=1 );
    HighscoresDialog hs(treeList, parent);
    for (uint i=0; i<_nbGameTypes; i++) {
        setGameType(i);
        QWidget *w;
        if (treeList) {
            QString title = _highscores->gameTypeLabel(i, Highscores::I18N);
            QString icon = _highscores->gameTypeLabel(i, Highscores::Icon);
            w = hs.addPage(title, QString::null,
                            BarIcon(icon, KIcon::SizeLarge));
        } else w = hs.plainPage();
        QVBoxLayout *vbox = new QVBoxLayout(w);

        w = new HighscoresWidget((i==tmp ? rank : -1), w,
                              queryURL(Players).url(), queryURL(Scores).url());
        vbox->addWidget(w);
    }

    setGameType(tmp);
    hs.showPage(_gameType);
	hs.exec();
}

void HighscoresPrivate::submitScore(const Score &ascore, QWidget *parent)
{
    checkFirst();

    Score score = ascore;
    score.setData("id", _playerInfos->id() + 1);
    score.setData("date", QDateTime::currentDateTime());

    _playerInfos->submitScore(score);
    if ( _playerInfos->isWWEnabled() )
        submitWorldWide(score, parent);

    if ( score.type()==Won ) {
        int rank = submitLocal(score);
        if ( rank!=-1 ) showHighscores(parent, rank);
    }

    _highscores->scoreSubmitted(score);
}

int HighscoresPrivate::submitLocal(const Score &score)
{
    int r = rank(score);
    if ( r!=-1 ) {
        uint nb = _scoreInfos->nbEntries();
        if ( nb<_scoreInfos->maxNbEntries() ) nb++;
        _scoreInfos->write(r, score, nb);
    }
    return r;
}

bool HighscoresPrivate::submitWorldWide(const Score &score, QWidget *parent)
{
    KURL url = queryURL(Submit);
    _highscores->additionnalQueryItems(url, score);
    int s = (score.type()==Won ? score.score() : (int)score.type());
    QString str =  QString::number(s);
    Highscores::addToQueryURL(url, "score", str);
    KMD5 context(QString(_playerInfos->registeredName() + str).latin1());
    Highscores::addToQueryURL(url, "check", context.hexDigest());

    qDebug("%s", url.url().latin1());
    return doQuery(url, parent);
}

Score HighscoresPrivate::lastScore()
{
    checkFirst();
    Score score(Won);
    _scoreInfos->read(_scoreInfos->maxNbEntries() - 1, score);
    return score;
}

Score HighscoresPrivate::firstScore()
{
    checkFirst();
    Score score(Won);
    _scoreInfos->read(0, score);
    return score;
}

void HighscoresPrivate::exportHighscores(QTextStream &s)
{
    uint tmp = _gameType;

    for (uint i=0; i<_nbGameTypes; i++) {
        setGameType(i);
        if ( _nbGameTypes>1 ) {
            if ( i!=0 ) s << endl;
            s << "--------------------------------" << endl;
            s << "Game type: "
              << _highscores->gameTypeLabel(_gameType, Highscores::I18N)
              << endl;
            s << endl;
        }
        s << "Players list:" << endl;
        _playerInfos->exportToText(s);
        s << endl;
        s << "Highscores list:" << endl;
        _scoreInfos->exportToText(s);
    }

    setGameType(tmp);
}

}; // namespace
