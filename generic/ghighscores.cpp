#include "ghighscores.h"

#include <qfile.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kmdcodec.h>
#include <kio/netaccess.h>
#include <khighscore.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "ghighscores_gui.h"


//-----------------------------------------------------------------------------
class ScoreNameItem : public NameItem
{
 public:
    ScoreNameItem(const ItemBase *idItem) : _idItem(idItem) {}

    QString pretty(uint i) const {
        uint id = _idItem->read(i).toUInt();
        if ( id==0 ) return NameItem::pretty(i);
        return highscores().prettyPlayerName(id-1);
    }

 private:
    const ItemBase *_idItem;
};

//-----------------------------------------------------------------------------
Score::Score(uint score)
    : DataContainer(highscores().scoreGroup(), QString::null)
{
    ItemBase *idItem = new ItemBase((uint)0);
    addData("id", idItem, true, highscores().playerId()+1);
    addData("rank", new RankItem, false, (uint)0);
    addData("name", new ScoreNameItem(idItem), true, QString::null);
    addData("score", highscores().scoreItem(), true, score);
    addData("date", new DateItem, true, QDateTime::currentDateTime());
}

uint Score::nbEntries() const
{
    return highscores().nbScores();
}

//-----------------------------------------------------------------------------
const char *HS_ID              = "player id";
const char *HS_REGISTERED_NAME = "registered name";
const char *HS_KEY             = "player key";
const char *HS_WW_ENABLED      = "ww hs enabled";
const char *HS_WW_URL          = "ww hs url";

class ConfigGroup : public KConfigGroupSaver
{
 public:
    ConfigGroup() : KConfigGroupSaver(kapp->config(), QString::null) {}
};

//-----------------------------------------------------------------------------
PlayerInfos::PlayerInfos()
    : ItemContainer("players", highscores().playerSubGroup())
{
    addItem("name", new NameItem, true);
    addItem("nb games", new ItemBase((uint)0, i18n("Nb of games"),
                                     Qt::AlignRight, true), true);
    if ( highscores().isLostGameEnabled() )
        addItem("success", new SuccessPercentageItem, true);
    addItem("mean score", highscores().meanScoreItem(), true);
    addItem("best score", highscores().bestScoreItem(), true);
    if ( highscores().isBlackMarkEnabled() )
        addItem("black mark", new ItemBase((uint)0, i18n("Black mark"),
                                           Qt::AlignRight, true), true);
    addItem("date", new DateItem, true);
    addItem("comment", new ItemBase(QString::null, i18n("Comment"),
                                    Qt::AlignLeft), true);

    if ( isNewPlayer() ) addPlayer();
    else {
        ConfigGroup cg;
        _id = cg.config()->readUnsignedNumEntry(HS_ID);
    }
}

bool PlayerInfos::isNewPlayer()
{
    ConfigGroup cg;
    return !cg.config()->hasKey(HS_ID);
}

bool PlayerInfos::isAnonymous() const
{
    return ( name()==ItemBase::ANONYMOUS );
}

uint PlayerInfos::nbEntries() const
{
    KHighscore hs;
    hs.setHighscoreGroup(group());
    QStringList list = hs.readList("name", -1);
    return list.count();
}

void PlayerInfos::addPlayer()
{
    ConfigGroup cg;
    _id = nbEntries();
    cg.config()->writeEntry(HS_ID, _id);
    item("name").write(_id, QString(ItemBase::ANONYMOUS));
}

QString PlayerInfos::key() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_KEY, QString::null);
}

bool PlayerInfos::WWEnabled() const
{
    ConfigGroup cg;
    return cg.config()->readBoolEntry(HS_WW_ENABLED, false);
}

void PlayerInfos::submitScore(bool won, const Score &score) const
{
    uint nb = item("nb games").read(_id).toUInt();
    uint nb_success = nb;
    if ( highscores().isLostGameEnabled() ) {
        double success = item("success").read(_id).toDouble();
        if ( success!=-1 ) nb_success = (uint)(success * nb / 100);
    }
    double total_score = item("mean score").read(_id).toDouble() * nb_success;

    nb++;
    if (won) {
        nb_success++;
        total_score += score.score();
    }
    double mean = (nb_success==0 ? 0 : total_score / nb_success);

    item("nb games").write(_id, nb);
    item("mean score").write(_id, mean);
    if ( highscores().isLostGameEnabled() ) {
        double success = 100.0 * nb_success / nb;
        item("success").write(_id, success);
    }
    if ( score.score()>item("best score").read(_id).toUInt() ) {
        item("best score").write(_id, score.score());
        item("date").write(_id, score.date());
    }
}

void PlayerInfos::submitBlackMark() const
{
    Q_ASSERT( highscores().isBlackMarkEnabled() );
    uint nb_bm = item("black mark").read(_id).toUInt();
    item("black mark").write(_id, nb_bm+1);
}

void PlayerInfos::modifySettings(const QString &newName,
                                 const QString &comment, bool WWEnabled,
                                 const QString &newKey) const
{
    ConfigGroup cg;
    item("name").write(_id, newName);
    item("comment").write(_id, comment);
    cg.config()->writeEntry(HS_WW_ENABLED, WWEnabled);
    if ( !newKey.isEmpty() ) cg.config()->writeEntry(HS_KEY, newKey);
    if (WWEnabled) cg.config()->writeEntry(HS_REGISTERED_NAME, newName);
}

QString PlayerInfos::registeredName() const
{
    ConfigGroup cg;
    return cg.config()->readEntry(HS_REGISTERED_NAME, QString::null);
}

//-----------------------------------------------------------------------------
Highscores::Highscores(const QString version, const KURL &baseURL,
                       uint nbGameTypes, uint nbEntries)
    : _version(version), _baseURL(baseURL), _nbGameTypes(nbGameTypes),
      _nbEntries(nbEntries), _gameType(0)
{
    Q_ASSERT( nbGameTypes!=0 );
    Q_ASSERT( nbEntries!=0 );
    bool b = baseURL.isEmpty();
    Q_ASSERT( b || (baseURL.isValid() && baseURL.fileName(false).isEmpty()) );

    if ( !b ) {
        ConfigGroup cg;
        _baseURL = cg.config()->readEntry(HS_WW_URL, baseURL.url());
    }

    if ( !PlayerInfos::isNewPlayer() ) return;
    uint tmp = _gameType;
    for (uint i=0; i<_nbGameTypes; i++) {
        _gameType = i;
        convertLegacy(i);
    }
    _gameType = tmp;
}

void Highscores::setGameType(uint type)
{
    Q_ASSERT( type<_nbGameTypes );
    _gameType = type;
}

QString Highscores::scoreGroup() const
{
    QString label = gameTypeLabel(_gameType, Standard);
    if ( label.isEmpty() ) return "scores";
    else return QString("scores_") + label;
}

QString Highscores::playerSubGroup() const
{
    return gameTypeLabel(_gameType, Standard);
}

HighscoresSettingsWidget *
Highscores::createSettingsWidget(BaseSettingsDialog *d) const
{
    return new HighscoresSettingsWidget(d, infos());
}

void Highscores::_showHighscores(QWidget *parent, int rank)
{
    uint tmp = _gameType;
    int face = (_nbGameTypes==1 ? KDialogBase::Plain : KDialogBase::TreeList);
    KDialogBase hs(face, i18n("Highscores"),
                   KDialogBase::Close, KDialogBase::Close,
                   parent, "show_highscores", true, true);
    for (uint i=0; i<_nbGameTypes; i++) {
        _gameType = i;
        Score *s = score();
        PlayerInfos *info = infos();
        QWidget *w;
        if ( _nbGameTypes==1 ) w = hs.plainPage();
        else w = hs.addPage(gameTypeLabel(i, I18N), QString::null,
                            BarIcon(gameTypeLabel(i, Icon), KIcon::SizeLarge));
        QVBoxLayout *vbox = new QVBoxLayout(w);
        w = new ShowHighscoresWidget(
                (i==tmp ? rank : -1), w, *s, *info, hs.spacingHint());
        vbox->addWidget(w);
        delete s;
        delete info;
    }
    _gameType = tmp;
    hs.resize( hs.calculateSize(500, 370) ); // hacky
    hs.showPage(_gameType);
    if ( _nbGameTypes==1 ) hs.enableButtonSeparator(false);
	hs.exec();
}

void Highscores::submitScore(bool won, Score &score, QWidget *parent)
{
    Q_ASSERT( won || isLostGameEnabled() );

    PlayerInfos *i = infos();
    i->submitScore(won, score);
    if ( i->WWEnabled() )
        submitWorldWide((won ? Won : Lost), &score, *i, parent);
    delete i;

    if (won) {
        int rank = submitLocal(score, QString::null);
        if ( rank!=-1 ) _showHighscores(parent, rank);
    }

}

void Highscores::submitBlackMark(QWidget *parent)
{
    Q_ASSERT( isBlackMarkEnabled() );
    PlayerInfos *i = infos();
    i->submitBlackMark();
    submitWorldWide(BlackMark, 0, *i, parent);
    delete i;
}

int Highscores::submitLocal(Score &score, const QString &name) const
{
    score.setName(name);
    int r = rank(score);
    if ( r!=-1 ) {
        uint nb = nbScores();
        if ( nb<_nbEntries ) nb++;
        score.write(r, nb);
    }
    return r;
}

bool Highscores::isStrictlyBetter(const Score &s1, const Score &s2) const
{
    return s1.score()>s2.score();
}

uint Highscores::nbScores() const
{
    Score *s = score();
    uint i = 0;
    for (; i<_nbEntries; i++) {
        s->read(i);
        if ( s->score()==0 ) break;
    }
    delete s;
	return i;
}

Score *Highscores::readScore(uint rank) const
{
    Score *s = score();
    s->read(rank);
    return s;
}

Score *Highscores::lastScore() const
{
    return readScore(_nbEntries - 1);
}

int Highscores::rank(const Score &s) const
{
    Score *tmp = score();
    uint nb = nbScores();
    uint i = 0;
	for (; i<nb; i++) {
        tmp->read(i);
		if ( isStrictlyBetter(s, (*tmp)) ) break;
    }
    delete tmp;
	return (i<_nbEntries ? (int)i : -1);
}

uint Highscores::playerId() const
{
    PlayerInfos *i = infos();
    uint id = i->id();
    delete i;
    return id;
}

QString Highscores::prettyPlayerName(uint id) const
{
    PlayerInfos *i = infos();
    QString name = i->prettyName(id);
    delete i;
    return name;
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
    additionnalQueryArgs(type, score);
}

QString Highscores::playersURL()
{
    PlayerInfos *i = infos();
    setQueryURL(Players, QString::null);
    if ( !i->registeredName().isEmpty() )
        addToQueryURL("highlight", i->registeredName());
    delete i;
    return _url.url();
}

QString Highscores::highscoresURL()
{
    PlayerInfos *i = infos();
    setQueryURL(Scores, i->registeredName());
    delete i;
    if ( _nbGameTypes>1 ) addToQueryURL("level", gameTypeLabel(_gameType, WW));
    return _url.url();
}

void Highscores::addToQueryURL(const QString &entry, const QString &content)
{
    if ( entry.isEmpty() ) return;
    QString query = _url.query();
    if ( !query.isEmpty() ) query += '&';
	query += entry + '=' + KURL::encode_string(content);
	_url.setQuery(query);
}

// strings that needs to be translated (coming from the highscores server)
const char *DUMMY_STRINGS[] = {
    I18N_NOOP("Undefined error."),
    I18N_NOOP("Missing argument(s)."),
    I18N_NOOP("Invalid argument(s)."),

    I18N_NOOP("Unable to connect mysql server."),
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

bool Highscores::_doQuery(QDomNamedNodeMap &attributes, QString &error) const
{
    QString tmpFile;
    if ( !KIO::NetAccess::download(_url, tmpFile) ) {
  	    error = i18n("Unable to contact remote host.");
	    return false;
	}
	QFile file(tmpFile);
	if ( !file.open(IO_ReadOnly) ) {
	    error = i18n("Unable to open temporary file.");
	    return false;
	}
	QTextStream t(&file);
	QString content = t.read().stripWhiteSpace();
	file.close();
	QDomDocument doc;
    if ( doc.setContent(content) ) {
        QDomElement root = doc.documentElement();
        QDomElement element = root.firstChild().toElement();
        attributes = element.attributes();
        if ( element.tagName()=="success" ) return true;
        if ( element.tagName()=="error" ) {
            QDomAttr attr = attributes.namedItem("label").toAttr();
            if ( !attr.isNull() ) {
                error = i18n(attr.value().latin1());
                return false;
            }
        }
    }
    file.close();
    KIO::NetAccess::removeTempFile(tmpFile);
    error = i18n("Invalid answer.");
    return false;
}

bool Highscores::doQuery(QDomNamedNodeMap &map, QWidget *parent) const
{
  QString error;
  bool ok = _doQuery(map, error);
  if ( !ok ) {
      error = i18n("An error occured while contacting\n"
                   "the world-wide highscores server :\n%1").arg(error);
      KMessageBox::sorry(parent, error);
  }
  return ok;
}

bool Highscores::getFromQuery(const QDomNamedNodeMap &map,
                              const QString &name, QString &value,
                              QWidget *parent)
{
    QDomAttr attr = map.namedItem(name).toAttr();
    if ( attr.isNull() ) {
	    KMessageBox::sorry(parent, i18n("Missing argument."));
		return false;
	}
	value = attr.value();
	return true;
}

void Highscores::submitWorldWide(ScoreType type, const Score *score,
                                 const PlayerInfos &i, QWidget *parent)
{
    setQueryURL(Submit, i.registeredName(), score);
    addToQueryURL("key", i.key());
    addToQueryURL("version", _version);
    int s = (type==Won ? score->score() : (int)type);
    QString str =  QString::number(s);
    addToQueryURL("score", str);
    KMD5 context(QString(i.registeredName() + str).latin1());
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
    PlayerInfos *i = infos();

    if (WWEnabled) {
        KURL url;
        newPlayer = i->key().isEmpty() || i->registeredName().isEmpty();
        if (newPlayer) setQueryURL(Register, newName);
        else {
            setQueryURL(Change, i->registeredName());
            addToQueryURL("key", i->key());
            if ( i->registeredName()!=newName )
                addToQueryURL("new_nickname", newName);
        }
        addToQueryURL("comment", comment);
        addToQueryURL("version", _version);
        QDomNamedNodeMap map;
        if ( !doQuery(map, parent)
             || (newPlayer && !getFromQuery(map, "key", newKey, parent)) ) {
            delete i;
            return false;
        }
    }

    i->modifySettings(newName, comment, WWEnabled, newKey);
    delete i;
    return true;
}
