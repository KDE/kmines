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

#include <qfile.h>
#include <qlayout.h>
#include <qdom.h>

#include <kmdcodec.h>
#include <kio/netaccess.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "ghighscores_internal.h"
#include "ghighscores_gui.h"


namespace KExtHighscores
{

Highscores *Highscores::_highscores = 0L;

const char *HS_WW_URL = "ww hs url";

Highscores::Highscores(const QString &version, const KURL &baseURL,
                       uint nbGameTypes, uint maxNbEntries,
                       bool trackLostGames, bool trackBlackMarks)
    : _version(version), _baseURL(baseURL), _nbGameTypes(nbGameTypes),
      _gameType(0), _first(true)
{
    _playerInfos = new PlayerInfos(trackLostGames, trackBlackMarks);
    _scoreInfos = new ScoreInfos(maxNbEntries, *_playerInfos);

    if (_highscores) qFatal("A highscore object already exists");
    _highscores = this;

    Q_ASSERT(nbGameTypes && maxNbEntries);

    if ( baseURL.isEmpty() ) return;
    Q_ASSERT( baseURL.isValid() );
    ConfigGroup cg;
    if ( cg.config()->hasKey(HS_WW_URL) )
        _baseURL = cg.config()->readEntry(HS_WW_URL);
    else cg.config()->writeEntry(HS_WW_URL, _baseURL.url());
}

Highscores::~Highscores()
{
    delete _scoreInfos;
    delete _playerInfos;
}

void Highscores::setGameType(uint type)
{
    if (_first) {
        _first = false;
        if ( _playerInfos->isNewPlayer() ) {
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
    _scoreInfos->setGroup(str);
    if ( !label.isEmpty() ) _playerInfos->setSubGroup(label);
}

KSettingWidget *Highscores::createSettingsWidget(QWidget *parent)
{
    if (_first) setGameType(0);
    return new HighscoresSettingsWidget(*_playerInfos, !_baseURL.isEmpty(),
                                        parent);
}

void Highscores::_showHighscores(QWidget *parent, int rank)
{
    uint tmp = _gameType;
    int face = (_nbGameTypes==1 ? KDialogBase::Plain : KDialogBase::TreeList);
    KDialogBase hs(face, i18n("Highscores"),
                   KDialogBase::Close, KDialogBase::Close,
                   parent, "show_highscores", true, true);
    bool WWHSAvailable = !_baseURL.isEmpty();
    for (uint i=0; i<_nbGameTypes; i++) {
        setGameType(i);
        QWidget *w;
        if ( _nbGameTypes==1 ) w = hs.plainPage();
        else w = hs.addPage(gameTypeLabel(i, I18N), QString::null,
                            BarIcon(gameTypeLabel(i, Icon), KIcon::SizeLarge));
        QVBoxLayout *vbox = new QVBoxLayout(w);
        setQueryURL(Scores, _playerInfos->registeredName());
        if ( _nbGameTypes>1 )
            addToQueryURL("level", gameTypeLabel(_gameType, WW));
        QString highscoresURL = _url.url();
        setQueryURL(Players, QString::null);
        if ( !_playerInfos->registeredName().isEmpty() )
            addToQueryURL("highlight", _playerInfos->registeredName());
        QString playersURL = _url.url();
        w = new HighscoresWidget(
            (i==tmp ? rank : -1), w, *_scoreInfos, *_playerInfos,
            hs.spacingHint(), WWHSAvailable, highscoresURL, playersURL);
        vbox->addWidget(w);
    }
    setGameType(tmp);

    hs.resize( hs.calculateSize(500, 370) ); // hacky
    hs.showPage(_gameType);
    if ( _nbGameTypes==1 ) hs.enableButtonSeparator(false);
	hs.exec();
}

void Highscores::showMultipleScores(const ScoreList &scores,
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
    score.setData("id", _playerInfos->id() + 1);
    score.setData("date", QDateTime::currentDateTime());

    _playerInfos->submitScore(score);
    if ( _playerInfos->isWWEnabled() ) submitWorldWide(score, parent);

    if ( score.type()==Won ) {
        int rank = submitLocal(score);
        if ( rank!=-1 ) _showHighscores(parent, rank);
    }
}

int Highscores::submitLocal(const Score &score) const
{
    int r = rank(score);
    if ( r!=-1 ) {
        uint nb = _scoreInfos->nbEntries();
        if ( nb<_scoreInfos->maxNbEntries() ) nb++;
        score.write(r, nb);
    }
    return r;
}

bool Highscores::isStrictlyWorse(const Score &s1, const Score &s2) const
{
    return s1.score()<s2.score();
}

void Highscores::addItemToScore(const QString &name, Item *item)
{
    _scoreInfos->addItem(name, item, true);
}

void Highscores::setItem(ReplaceableItem type, Item *item)
{
    switch (type) {
    case RScore:
        _scoreInfos->setItem("score", item);
        break;
    case RMeanScore:
        _playerInfos->setItem("mean score", item);
        break;
    case RBestScore:
        _playerInfos->setItem("best score", item);
        break;
    }
}

Score Highscores::lastScore()
{
    if (_first) setGameType(0);
    Score score(Won);
    score.read(_scoreInfos->maxNbEntries() - 1);
    return score;
}

Score Highscores::firstScore()
{
    if (_first) setGameType(0);
    Score score(Won);
    score.read(0);
    return score;
}

int Highscores::rank(const Score &score) const
{
    Score tmp(Won);
    uint nb = _scoreInfos->nbEntries();
    uint i = 0;
	for (; i<nb; i++) {
        tmp.read(i);
		if ( tmp<score ) break;
    }
	return (i<_scoreInfos->maxNbEntries() ? (int)i : -1);
}

void Highscores::setQueryURL(QueryType type, const QString &nickname,
                             const Score *score)
{
    _url = _baseURL;
	switch (type) {
        case Submit:   _url.addPath("submit.php");     break;
        case Register: _url.addPath("register.php");   break;
        case Change:   _url.addPath("change.php");     break;
        case Players:  _url.addPath("players.php");    break;
        case Scores:   _url.addPath("highscores.php"); break;
	}
    if ( !nickname.isEmpty() ) addToQueryURL("nickname", nickname);
    if ( type==Submit ) {
        Q_ASSERT(score);
        additionnalQueryItems(*score);
    }
}

void Highscores::addToQueryURL(const QString &item, const QString &content)
{
    Q_ASSERT( !item.isEmpty() && _url.queryItem(item).isNull() );

    QString query = _url.query();
    if ( !query.isEmpty() ) query += '&';
	query += item + '=' + KURL::encode_string(content);
	_url.setQuery(query);
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

bool Highscores::doQuery(QDomNamedNodeMap &attributes, QWidget *parent) const
{
    QString tmpFile;
    if ( !KIO::NetAccess::download(_url, tmpFile) ) {
        QString msg = i18n("Unable to contact world-wide highscore server");
        QString details = i18n("Host url : %1").arg(_url.host());
        KMessageBox::detailedSorry(parent, msg, details);
        return false;
    }

	QFile file(tmpFile);
	if ( !file.open(IO_ReadOnly) ) {
        QString msg = i18n("Unable to contact world-wide highscore server");
        QString details = i18n("Unable to open temporary file.");
        KMessageBox::detailedSorry(parent, msg, details);
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
        attributes = element.attributes();
        if ( element.tagName()=="success" ) return true;
        if ( element.tagName()=="error" ) {
            QDomAttr attr = attributes.namedItem("label").toAttr();
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
    QString details = i18n("Raw message is : %1").arg(content);
    KMessageBox::detailedSorry(parent, msg, details);
    return false;
}

bool Highscores::getFromQuery(const QDomNamedNodeMap &map,
                              const QString &name, QString &value,
                              QWidget *parent)
{
    QDomAttr attr = map.namedItem(name).toAttr();
    if ( attr.isNull() ) {
	    KMessageBox::sorry(parent,
               i18n("Invalid answer from world-wide "
                    "highscores server (missing item : %1).").arg(name));
		return false;
	}
	value = attr.value();
	return true;
}

void Highscores::submitWorldWide(const Score &score, QWidget *parent)
{
    setQueryURL(Submit, _playerInfos->registeredName(), &score);
    addToQueryURL("key", _playerInfos->key());
    addToQueryURL("version", _version);
    int s = (score.type()==Won ? score.score() : (int)score.type());
    QString str =  QString::number(s);
    addToQueryURL("score", str);
    KMD5 context(QString(_playerInfos->registeredName() + str).latin1());
    addToQueryURL("check", context.hexDigest());
    addToQueryURL("level", gameTypeLabel(_gameType, WW));
    QDomNamedNodeMap map;
    doQuery(map, parent);
}

bool Highscores::modifySettings(const QString &newName,
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
        KURL url;
        newPlayer = _playerInfos->key().isEmpty()
                    || _playerInfos->registeredName().isEmpty();
        if (newPlayer) setQueryURL(Register, newName);
        else {
            setQueryURL(Change, _playerInfos->registeredName());
            addToQueryURL("key", _playerInfos->key());
            if ( _playerInfos->registeredName()!=newName )
                addToQueryURL("new_nickname", newName);
        }
        addToQueryURL("comment", comment);
        addToQueryURL("version", _version);
        QDomNamedNodeMap map;
        if ( !doQuery(map, parent)
             || (newPlayer && !getFromQuery(map, "key", newKey, parent)) )
            return false;
    }

    _playerInfos->modifySettings(newName, comment, WWEnabled, newKey);
    return true;
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

}; // namescape
