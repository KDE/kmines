#include "ghighscores.h"
#include "ghighscores.moc"

#include <qlabel.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qheader.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kapp.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kmdcodec.h>
#include <kio/netaccess.h>
#include <khighscore.h>
#include <kurllabel.h>
#include <kopenwith.h>
#include <krun.h>


//-----------------------------------------------------------------------------
void ItemBase::set(const QString &n, const QString &grp, const QString &sgrp)
{
    _name = n;
    _group = grp;
    _subGroup = sgrp;
}

QString ItemBase::entryName() const
{
    if ( !_canHaveSubGroup || _subGroup.isEmpty() ) return _name;
    return _name + "_" + _subGroup;
}

QVariant ItemBase::read(uint i) const
{
    Q_ASSERT( stored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    QVariant v = hs.readEntry(i+1, entryName(), _default.toString());
    if ( v.cast(_default.type()) ) return v;
    return _default;
}

QString ItemBase::pretty(uint i) const
{
	return read(i).toString();
}

void ItemBase::write(uint i, const QVariant &value) const
{
    Q_ASSERT( stored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    hs.writeEntry(i+1, entryName(), value.toString());
}

void ItemBase::moveDown(uint newIndex) const
{
    Q_ASSERT( newIndex!=0 );
    write(newIndex, read(newIndex-1));
}

//-----------------------------------------------------------------------------
ItemContainer::ItemContainer(const QString &group, const QString &subGroup)
    : _group(group), _subGroup(subGroup)
{
    _items.setAutoDelete(true);
    _names.setAutoDelete(true);
}

void ItemContainer::addItem(const QString &key, ItemBase *item, bool stored)
{
    uint i = _items.count();
    _names.insert(key, new uint(i));
    _items.append(item);
    _items.at(i)->set(key, (stored ? _group : QString::null), _subGroup);
}

const ItemBase &ItemContainer::item(const QString &n) const
{
    QPtrListIterator<ItemBase> it(_items);
    it += name(n);
    return *it.current();
}

//-----------------------------------------------------------------------------
void DataContainer::addData(const QString &key, ItemBase *item, bool stored,
                            QVariant value)
{
    ItemContainer::addItem(key, item, stored);
    _data.append(value);
}

QString DataContainer::prettyData(const QString &name) const
{
	return data(name).toString();
}

void DataContainer::read(uint i)
{
    QPtrListIterator<ItemBase> it(items());
    while( it.current() ) {
        const ItemBase *item = it.current();
        data(item->name()) = item->read(i);
        ++it;
    }
}

void DataContainer::write(uint i, uint nb) const
{
    QPtrListIterator<ItemBase> it(items());
    while( it.current() ) {
        const ItemBase *item = it.current();
        ++it;
        if ( !item->stored() ) continue;
        for (uint j=nb-1; j>i; j--) item->moveDown(j);
        item->write(i, data(item->name()));
    }
}

QDataStream &operator <<(QDataStream &stream, const DataContainer &c)
{
    stream << c._data;
    return stream;
}

QDataStream &operator >>(QDataStream &stream, DataContainer &c)
{
    stream >> c._data;
    return stream;
}

//-----------------------------------------------------------------------------
Score::Score(uint score)
    : DataContainer(highscores().scoreGroup(), QString::null)
{
    addData("rank", new ScoreItemRank, false, (uint)0);
    addData("name", new ItemBase(i18n("anonymous"), i18n("Player"),
                                 Qt::AlignLeft), true, QString::null);
    addData("score", highscores().scoreItemScore(), true, score);
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
const char *ANONYMOUS          = "_";

PlayerInfos::PlayerInfos()
    : ItemContainer("players", highscores().playerSubGroup())
{
    addItem("name", new PlayerItemName, true);
    addItem("nb games", new ItemBase((uint)0, i18n("Nb of games"),
                                     Qt::AlignRight, true), true);
    if ( highscores().isLostGameEnabled() )
        addItem("success", new PlayerItemWin, true);
    addItem("mean score", highscores().playerItemMeanScore(), true);
    addItem("best score", highscores().playerItemBestScore(), true);
    if ( highscores().isBlackMarkEnabled() )
        addItem("black mark", new ItemBase((uint)0, i18n("Black mark"),
                                           Qt::AlignRight, true), true);
    addItem("comment", new ItemBase(QString::null, i18n("Comment"),
                                    Qt::AlignLeft), true);

    if ( isNewPlayer() ) addPlayer();
    else _id = config()->readUnsignedNumEntry(HS_ID);
}

bool PlayerInfos::isNewPlayer()
{
    return !config()->hasKey(HS_ID);
}

bool PlayerInfos::isAnonymous() const
{
    return ( name()==ANONYMOUS );
}

KConfig *PlayerInfos::config()
{
    KConfig *config = kapp->config();
    config->setGroup(QString::null);
    return config;
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
    _id = nbEntries();
    config()->writeEntry(HS_ID, _id);
    item("name").write(_id, QString(ANONYMOUS));
}

QString PlayerInfos::key() const
{
    return config()->readEntry(HS_KEY, QString::null);
}

bool PlayerInfos::WWEnabled() const
{
    return config()->readBoolEntry(HS_WW_ENABLED, false);
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
    if ( score.score()>item("best score").read(_id).toUInt() )
        item("best score").write(_id, score.score());
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
    item("name").write(_id, newName);
    item("comment").write(_id, comment);
    config()->writeEntry(HS_WW_ENABLED, WWEnabled);
    if ( !newKey.isEmpty() ) config()->writeEntry(HS_KEY, newKey);
    if (WWEnabled) config()->writeEntry(HS_REGISTERED_NAME, newName);
}

QString PlayerInfos::registeredName() const
{
    return config()->readEntry(HS_REGISTERED_NAME, QString::null);
}

//-----------------------------------------------------------------------------
ShowHighscoresItem::ShowHighscoresItem(QListView *list, bool highlight)
    : KListViewItem(list), _highlight(highlight)
{}

void ShowHighscoresItem::paintCell(QPainter *p, const QColorGroup &cg,
                                   int column, int width, int align)
{
    QColorGroup cgrp(cg);
    if (_highlight) cgrp.setColor(QColorGroup::Text, red);
    KListViewItem::paintCell(p, cgrp, column, width, align);
}

//-----------------------------------------------------------------------------
ShowScoresList::ShowScoresList(QWidget *parent)
    : KListView(parent)
{
    setSelectionMode(QListView::NoSelection);
    setItemMargin(3);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    header()->setClickEnabled(false);
    header()->setMovingEnabled(false);
}

void ShowScoresList::addLine(const ItemContainer &container, int index,
                             bool highlight)
{
    QListViewItem *line
        = (index==-1 ? 0 : new ShowHighscoresItem(this, highlight));
    int i = -1;
    QPtrListIterator<ItemBase> it(container.items());
    while ( it.current() ) {
        const ItemBase *item = it.current();
        ++it;
        if ( !item->shown() || !showColumn(item) ) continue;
        i++;
        if (line) line->setText(i, itemText(item, index));
        else {
            addColumn( item->label() );
            setColumnAlignment(i, item->alignment());
        }
    }
}

//-----------------------------------------------------------------------------
ShowHighscoresList::ShowHighscoresList(const ItemContainer &container,
                                       int highlight, QWidget *parent)
    : ShowScoresList(parent)
{
    uint nb = container.nbEntries();
    for (int j=nb; j>=0; j--)
        addLine(container, (j==(int)nb ? -1 : j), j==highlight);
}

QString ShowHighscoresList::itemText(const ItemBase *item, uint row) const
{
    return item->pretty(row);
}

//-----------------------------------------------------------------------------
ShowHighscoresWidget::ShowHighscoresWidget(int localRank,
         QWidget *parent, const Score &score, const PlayerInfos &player,
         int spacingHint)
    : QWidget(parent, "show_highscores_widget")
{
    QVBoxLayout *vbox = new QVBoxLayout(this, spacingHint);

    QTabWidget *tw = new QTabWidget(this);
    vbox->addWidget(tw);

    QWidget *w;
    if ( score.nbEntries()==0 ) {
        QLabel *lab = new QLabel(i18n("no score entry"), this);
        lab->setAlignment(AlignCenter);
        w = lab;
    } else w = new ShowHighscoresList(score, localRank, this);
    tw->addTab(w, i18n("Best &scores"));

    w = new ShowHighscoresList(player, player.id(), this);
    tw->addTab(w, i18n("&Players"));

    if ( highscores().isWWHSAvailable() ) {
        KURLLabel *urlLabel = new KURLLabel(highscores().highscoresURL(),
                                 i18n("View world-wide highscores"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);

        urlLabel = new KURLLabel(highscores().playersURL(),
                                 i18n("View world-wide players"), this);
        connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
                SLOT(showURL(const QString &)));
        vbox->addWidget(urlLabel);
    }
}

void ShowHighscoresWidget::showURL(const QString &url) const
{
    KFileOpenWithHandler foo;
    (void)new KRun(KURL(url));
}

//-----------------------------------------------------------------------------
ShowMultiScoresList::ShowMultiScoresList(const QPtrVector<Score> &scores,
                                         QWidget *parent)
    : ShowScoresList(parent), _scores(scores)
{
    addLine(*scores[0], -1, false);
    for (uint i=0; i<scores.size(); i++)
        addLine(*scores[i], i, false);
}

QString ShowMultiScoresList::itemText(const ItemBase *item, uint row) const
{
    return _scores[row]->prettyData(item->name());
}

bool ShowMultiScoresList::showColumn(const ItemBase *item) const
{
    return item->name()!="rank";
}

//-----------------------------------------------------------------------------
ShowMultiScoresDialog::ShowMultiScoresDialog(const QPtrVector<Score> &scores,
                                             QWidget *parent)
: KDialogBase(Plain, i18n("Multiplayers scores"), Close, Close,
			  parent, "show_multiplayers_score", true, true)
{
    QVBoxLayout *vbox = new QVBoxLayout(plainPage());
    QWidget *list = new ShowMultiScoresList(scores, plainPage());
    vbox->addWidget(list);

    enableButtonSeparator(false);
}

//-----------------------------------------------------------------------------
HighscoresSettingsWidget::HighscoresSettingsWidget(BaseSettingsDialog *parent,
                                                   PlayerInfos *infos)
    : BaseSettingsWidget(new BaseSettings(i18n("Highscores"), "highscores"),
						 parent, "highscores_settings"),
      _WWHEnabled(0)
{
    QVBoxLayout *top = new QVBoxLayout(this, parent->spacingHint());

    QGrid *grid = new QGrid(2, this);
    grid->setSpacing(parent->spacingHint());
    top->addWidget(grid);
    (void)new QLabel(i18n("Nickname"), grid);
    _nickname = new QLineEdit((infos->isAnonymous() ? QString::null
                               : infos->name()), grid);
    _nickname->setMaxLength(16);
    QString name = infos->registeredName();
    if ( !infos->key().isEmpty() && !name.isEmpty() ) {
        (void)new QLabel(i18n("Registered nickname :"), grid);
        (void)new QLabel(name, grid);
    }

    (void)new QLabel(i18n("Comment"), grid);
    _comment = new QLineEdit(infos->comment(), grid);
    _comment->setMaxLength(50);

    if ( highscores().isWWHSAvailable() ) {
        _WWHEnabled
            = new QCheckBox(i18n("world-wide highscores enabled"), this);
        _WWHEnabled->setChecked(infos->WWEnabled());
        top->addWidget(_WWHEnabled);
    }
}

bool HighscoresSettingsWidget::writeConfig()
{
    bool enabled = (_WWHEnabled ? _WWHEnabled->isChecked() : false);
    bool res
        =  highscores().modifySettings(_nickname->text().lower(),
                                       _comment->text(), enabled,
                                       (QWidget *)parent());
    if ( !res ) emit showPage();
    return res;
}

//-----------------------------------------------------------------------------
const int Highscores::LOST_GAME_ID  = -1;
const int Highscores::BLACK_MARK_ID = -2;

Highscores::Highscores(const QString version, const KURL &baseURL,
                       uint nbGameTypes, uint nbEntries)
    : _version(version), _baseURL(baseURL), _nbGameTypes(nbGameTypes),
      _nbEntries(nbEntries), _gameType(0)
{
    Q_ASSERT( nbGameTypes!=0 );
    Q_ASSERT( nbEntries!=0 );
    Q_ASSERT( baseURL.isEmpty()
            || (baseURL.isValid() && baseURL.fileName(false).isEmpty()) );
}

void Highscores::init()
{
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
    if ( i->isAnonymous() )
        KMessageBox::sorry(parent, i18n("Please enter your nickname\n"
                                        "in the settings dialog."));
    i->submitScore(won, score);
    submitWorldWide((won ? (int)score.score() : LOST_GAME_ID), *i, parent);

    if (won) {
        int rank = submitLocal(score, i->prettyName());
        if ( rank!=-1 ) _showHighscores(parent, rank);
    }

    delete i;
}

void Highscores::submitBlackMark(QWidget *parent) const
{
    Q_ASSERT( isBlackMarkEnabled() );
    PlayerInfos *i = infos();
    i->submitBlackMark();
    submitWorldWide(BLACK_MARK_ID, *i, parent);
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

KURL Highscores::URL(QueryType type, const QString &nickname) const
{
    KURL url(_baseURL);
	switch (type) {
        case Submit:   url.addPath("submit.php");     break;
        case Register: url.addPath("register.php");   break;
        case Change:   url.addPath("change.php");     break;
        case Players:  url.addPath("players.php");    break;
        case Scores:   url.addPath("highscores.php"); break;
	}
    if ( !nickname.isEmpty() ) {
        QString query = "nickname=" + KURL::encode_string(nickname);
        url.setQuery(query);
    }
	return url;
}

QString Highscores::playersURL() const
{
    PlayerInfos *i = infos();
    KURL url = URL(Players, QString::null);
    if ( !i->registeredName().isEmpty() )
        addToURL(url, "highlight", i->registeredName());
    delete i;
    return url.url();
}

QString Highscores::highscoresURL() const
{
    PlayerInfos *i = infos();
    KURL url = URL(Scores, i->registeredName());
    delete i;
    if ( _nbGameTypes>1 ) addToURL(url, "level", gameTypeLabel(_gameType, WW));
    return url.url();
}

void Highscores::addToURL(KURL &url, const QString &entry,
                          const QString &content)
{
    if ( entry.isEmpty() ) return;
    QString query = url.query();
    if ( !query.isEmpty() ) query += '&';
	query += entry + '=' + KURL::encode_string(content);
	url.setQuery(query);
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

bool Highscores::_doQuery(const KURL &url, QDomNamedNodeMap &attributes,
                          QString &error)
{
    QString tmpFile;
    if ( !KIO::NetAccess::download(url, tmpFile) ) {
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
    error = i18n("Invalid answer.");
    return false;
}

bool Highscores::doQuery(const KURL &url, QDomNamedNodeMap &map,
                         QWidget *parent)
{
  QString error;
  bool ok = _doQuery(url, map, error);
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

void Highscores::submitWorldWide(int score, const PlayerInfos &i,
                                 QWidget *parent) const
{
    if ( !i.WWEnabled() ) return;
    KURL url = URL(Submit, i.registeredName());
    addToURL(url, "key", i.key());
    addToURL(url, "version", _version);
    QString str =  QString::number(score);
    addToURL(url, "score", str);
    KMD5 context(i.registeredName() + str);
    addToURL(url, "check", context.hexDigest());
    if ( _nbGameTypes>1 ) addToURL(url, "level", gameTypeLabel(_gameType, WW));
    additionnalQueries(url, Submit);
    QDomNamedNodeMap map;
    doQuery(url, map, parent);
}

bool Highscores::modifySettings(const QString &newName,
                                const QString &comment, bool WWEnabled,
                                QWidget *parent) const
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
        if (newPlayer) url = URL(Register, newName);
        else {
            url = URL(Change, i->registeredName());
            addToURL(url, "key", i->key());
            if ( i->registeredName()!=newName )
                addToURL(url, "new_nickname", newName);
        }
        addToURL(url, "comment", comment);
        addToURL(url, "version", _version);
        QDomNamedNodeMap map;
        if ( !doQuery(url, map, parent)
             || (newPlayer && !getFromQuery(map, "key", newKey, parent)) ) {
            delete i;
            return false;
        }
    }

    i->modifySettings(newName, comment, WWEnabled, newKey);
    delete i;
    return true;
}
