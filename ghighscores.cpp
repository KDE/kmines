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

#include "version.h"
#include "defines.h"


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

QString ItemBase::read(uint i) const
{
    ASSERT( stored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    return hs.readEntry(i+1, entryName(), _default);
}

uint ItemBase::readUInt(uint i) const
{
    bool ok;
    uint res = read(i).toUInt(&ok);
    return (ok ? res : _default.toUInt());
}

double ItemBase::readDouble(uint i) const
{
    bool ok;
    double res = read(i).toDouble(&ok);
    return (ok ? res : _default.toDouble());
}

void ItemBase::write(uint i, const QString &value) const
{
    ASSERT( stored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    hs.writeEntry(i+1, entryName(), value);
}

void ItemBase::write(uint i, uint value) const
{
    write(i, QString::number(value));
}

void ItemBase::write(uint i, double value) const
{
    write(i, QString::number(value, 'f', 10));
}

void ItemBase::moveDown(uint newIndex) const
{
    ASSERT( newIndex!=0 );
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

const ItemBase *ItemContainer::item(const QString &n) const
{
    QListIterator<ItemBase> it(_items);
    it += name(n);
    return it.current();
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
    const QVariant &va = data(name);
    switch ( va.type() ) {
        case QVariant::String: return va.toString();
        case QVariant::UInt:   return QString::number(va.toUInt());
        case QVariant::Double: return QString::number(va.toDouble(), 'f', 10);
        default: ASSERT(false);
    }
    return QString::null;
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
Score::Score(uint score, const QString &group, ItemBase *scoreItem)
    : DataContainer(group, QString::null)
{
    addData("rank", new ScoreItemRank, false, (uint)0);
    addData("name", new ItemBase(i18n("anonymous"), i18n("Player"),
                                 Qt::AlignLeft), true, QString::null);
    if ( scoreItem==0 ) scoreItem = new ScoreItemScore;
    addData("score", scoreItem, true, score);
}

uint Score::nb() const
{
	for (uint i=0; i<NB_HS_ENTRIES; i++)
        if ( score(i)==0 ) return i;
	return NB_HS_ENTRIES;
}

int Score::rank() const
{
	uint n = nb();
	for (uint i=0; i<n; i++)
		if ( score(i)<score() ) return i;
	return (n<NB_HS_ENTRIES ? (int)n : -1);
}

uint Score::lastScore() const
{
    if ( nb()<NB_HS_ENTRIES ) return 0;
    else return score(nb()-1);
}

int Score::submit(QWidget *parent, bool warn) const
{
    if ( score()==0 ) return -1;

    if (warn) {
        PlayerInfos pi;
        if ( pi.isAnonymous() )
            KMessageBox::sorry(parent, i18n("Please enter your nickname\n"
                                            "in the settings dialog."));
    }

    int r = rank();
    if ( r!=-1 ) {
        uint n = nb();
        if ( n<NB_HS_ENTRIES ) n++;
        QListIterator<ItemBase> it(items());
        while( it.current() ) {
            const ItemBase *item = it.current();
            ++it;
            if ( !item->stored() ) continue;
            for (int j=n-1; j>r; j--) item->moveDown(j);

            item->write(r, prettyData(item->name()));
        }
    }
    return r;
}

//-----------------------------------------------------------------------------
const char *HS_ID              = "player id";
const char *HS_REGISTERED_NAME = "registered name";
const char *HS_KEY             = "player key";
const char *HS_WW_ENABLED      = "ww hs enabled";

PlayerInfos::PlayerInfos(const QString &subGroup, ItemBase *bestScoreItem,
                         ItemBase *meanScoreItem)
    : ItemContainer("players", subGroup)
{
    addItem("name", new PlayerItemName, true);
    addItem("nb games", new ItemBase("0", i18n("Nb of games"),
                                     Qt::AlignRight, true), true);
    if ( meanScoreItem==0 ) meanScoreItem = new PlayerItemMeanScore;
    addItem("mean score", meanScoreItem, true);
    if ( bestScoreItem==0 ) bestScoreItem = new PlayerItemBestScore;
    addItem("best score", bestScoreItem, true);
    addItem("comment", new ItemBase(QString::null, i18n("Comment"),
                                    Qt::AlignLeft), true);

    _newPlayer = !config()->hasKey(HS_ID);
    if (_newPlayer) addPlayer();
    else _id = config()->readUnsignedNumEntry(HS_ID);
}

bool PlayerInfos::isAnonymous() const
{
    return ( name()==ANONYMOUS );
}

KConfig *PlayerInfos::config() const
{
    KConfig *config = kapp->config();
    config->setGroup(QString::null);
    return config;
}

uint PlayerInfos::nb() const
{
    KHighscore hs;
    hs.setHighscoreGroup(group());
    QStringList list = hs.readList("name", -1);
    return list.count();
}

void PlayerInfos::addPlayer()
{
    _id = nb();
    config()->writeEntry(HS_ID, _id);
    item("name")->write(_id, QString(ANONYMOUS));
}

QString PlayerInfos::key() const
{
    return config()->readEntry(HS_KEY, QString::null);
}

bool PlayerInfos::WWEnabled() const
{
    return config()->readBoolEntry(HS_WW_ENABLED, false);
}

int PlayerInfos::submitScore(Score &score, QWidget *parent) const
{
    uint nb = item("nb games")->readUInt(_id);
    double mean;
    if ( nb==0 ) mean = score.score();
    else {
        mean = item("mean score")->readDouble(_id);
        mean = (mean*nb + score.score()) / (nb+1);
    }
    nb++;
    item("nb games")->write(_id, nb);
    item("mean score")->write(_id, mean);
    if ( score.score()>item("best score")->readUInt(_id) )
        item("best score")->write(_id, score.score());

    score.setName(prettyName());
    int rank = score.submit(parent, true);

    if ( WWEnabled() ) {
        KURL url = URL(Submit, registeredName());
        addToURL(url, "key", key());
        addToURL(url, "version", VERSION);
        QString str =  QString::number(score.score());
        addToURL(url, "score", str);
        KMD5 context(registeredName() + str);
        addToURL(url, "check", context.hexDigest());
        additionnalQueries(url, Submit);
        QDomNamedNodeMap map;
        doQuery(url, map, parent);
    }

    return rank;
}

bool PlayerInfos::modifySettings(const QString &newName,
                                 const QString &comment,
                                 bool WWEnabled, QWidget *parent) const
{
    if ( newName.isEmpty() ) {
        KMessageBox::sorry(parent,i18n("Please choose a non empty nickname."));
	    return false;
	}

    QString newKey;
    if (WWEnabled) {
        KURL url;
        bool newPlayer = key().isEmpty() || registeredName().isEmpty();
	    if (newPlayer) url = URL(Register, newName);
		else {
		    url = URL(Change, registeredName());
			addToURL(url, "key", key());
			if ( registeredName()!=newName )
                addToURL(url, "new_nickname", newName);
		}
		addToURL(url, "comment", comment);
		addToURL(url, "version", VERSION);
        QDomNamedNodeMap map;
		if ( !doQuery(url, map, parent) ) return false;
		if (newPlayer) {
            if ( !getFromQuery(map, "key", newKey, parent) ) return false;
            config()->writeEntry(HS_KEY, newKey);
        }
        config()->writeEntry(HS_REGISTERED_NAME, newName);
    }

    item("name")->write(_id, newName);
    item("comment")->write(_id, comment);
    config()->writeEntry(HS_WW_ENABLED, WWEnabled);

    return true;
}

QString PlayerInfos::registeredName() const
{
    return config()->readEntry(HS_REGISTERED_NAME, QString::null);
}

KURL PlayerInfos::URL(QueryType type, const QString &nickname)
{
    KURL url(WORLD_WIDE_HS_URL);
	switch (type) {
        case Submit:     url.addPath("submit.php");     break;
        case Register:   url.addPath("register.php");   break;
        case Change:     url.addPath("change.php");     break;
        case Players:    url.addPath("players.php");    break;
        case Highscores: url.addPath("highscores.php"); break;
	}
    if ( !nickname.isEmpty() ) {
        QString query = "nickname=" + KURL::encode_string(nickname);
        url.setQuery(query);
    }
	return url;
}

QString PlayerInfos::playersURL() const
{
    KURL url = URL(Players, QString::null);
    addToURL(url, "highlight", registeredName());
    return url.url();
}

QString PlayerInfos::highscoresURL() const
{
    return URL(Highscores, registeredName()).url();
}

QString PlayerInfos::showHighscoresCaption() const
{
    return i18n("Highscores");
}

void PlayerInfos::addToURL(KURL &url, const QString &entry,
                           const QString &content)
{
    if ( entry.isEmpty() ) return;
    QString query = url.query();
    if ( !query.isEmpty() ) query += '&';
	query += entry + '=' + KURL::encode_string(content);
	url.setQuery(query);
}

bool PlayerInfos::_doQuery(const KURL &url, QDomNamedNodeMap &attributes,
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
	if ( content.isEmpty() ) {
        error = i18n("No data in answer.");
        return false;
    }
    if ( content.mid(0, 5)!="<xml>" ) {
        error = i18n("Error from server : %1").arg(content);
        return false;
    }
	QDomDocument doc;
    doc.setContent(content);
    QDomElement root = doc.documentElement();
    QDomElement element = root.firstChild().toElement();
    if ( element.isNull() || element.tagName()!="success" ) {
 	    error = i18n("Invalid answer.");
	    return false;
	}
	attributes = element.attributes();
	return true;
}

bool PlayerInfos::doQuery(const KURL &url, QDomNamedNodeMap &map,
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

bool PlayerInfos::getFromQuery(const QDomNamedNodeMap &map,
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

//-----------------------------------------------------------------------------
ShowHighscoresItem::ShowHighscoresItem(QListView *list, uint index,
                                       bool highlight)
    : QListViewItem(list), _index(index), _highlight(highlight)
{}

void ShowHighscoresItem::paintCell(QPainter *p, const QColorGroup &cg,
                                   int column, int width, int align)
{
    QColorGroup cgrp(cg);
    if (_highlight) cgrp.setColor(QColorGroup::Text, red);
    if ( _index%2 ) cgrp.setColor(QColorGroup::Base, lightGray.light(125));
    QListViewItem::paintCell(p, cgrp, column, width, align);
}

//-----------------------------------------------------------------------------
QListView *ShowScores::createList(QWidget *parent) const
{
    QListView *list = new QListView(parent);
    list->setSelectionMode(QListView::NoSelection);
    list->setItemMargin(3);
    list->setAllColumnsShowFocus(true);
    list->header()->setClickEnabled(false);
    list->setSorting(-1); // sorting breaks the alternate gray/white lines ...
    return list;
}

void ShowScores::addLine(QListView *list, const ItemContainer &container,
                         int index, bool highlight) const
{
    QListViewItem *line
        = (index==-1 ? 0 : new ShowHighscoresItem(list, index, highlight));
    int i = -1;
    QListIterator<ItemBase> it(container.items());
    while ( it.current() ) {
        const ItemBase *item = it.current();
        ++it;
        if ( !item->shown() || !showColumn(item) ) continue;
        i++;
        if (line) line->setText(i, itemText(item, index));
        else {
            list->addColumn( item->label() );
            list->setColumnAlignment(i, item->alignment());
        }
    }
}

//-----------------------------------------------------------------------------
ShowHighscores::ShowHighscores(int localRank, QWidget *parent,
                               const Score &scoreDummy,
                               const PlayerInfos &playerDummy)
    : KDialogBase(Tabbed, playerDummy.showHighscoresCaption(), Close, Close,
                  parent, "show_highscores", true, true)
{
    // best scores
    QVBox *page = addVBoxPage(i18n("Best scores"));
    if ( scoreDummy.nb()==0 ) {
        QLabel *lab = new QLabel(i18n("no score entry"), page);
        lab->setAlignment(AlignCenter);
    } else {
        _bestList = createList(page);
        fillList(_bestList, scoreDummy, localRank);
    }

    KURLLabel *urlLabel = new KURLLabel(playerDummy.highscoresURL(),
                                    i18n("View world-wide highscores"), page);
    urlLabel->setEnabled(playerDummy.WWEnabled());
    connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
            SLOT(showURL(const QString &)));

    // player infos
    page = addVBoxPage(i18n("Players"));
    _playersList = createList(page);
    fillList(_playersList, playerDummy, playerDummy.id());

    urlLabel = new KURLLabel(playerDummy.playersURL(),
                                    i18n("View world-wide players"), page);
    urlLabel->setEnabled(playerDummy.WWEnabled());
    connect(urlLabel, SIGNAL(leftClickedURL(const QString &)),
            SLOT(showURL(const QString &)));

    enableButtonSeparator(false);
}

void ShowHighscores::showURL(const QString &url) const
{
    KFileOpenWithHandler foo;
    (void)new KRun(KURL(url));
}

QString ShowHighscores::itemText(const ItemBase *item, uint row) const
{
    return item->pretty(row);
}

void ShowHighscores::fillList(QListView *list, const ItemContainer &container,
                              int highlight) const
{
    uint nb = container.nb();
    for (int j=nb; j>=0; j--)
        addLine(list, container, (j==(int)nb ? -1 : j), j==highlight);
}

//-----------------------------------------------------------------------------
HighscoresOption::HighscoresOption(KDialogBase *dialog)
    : _dialog(dialog)
{
    QFrame *page = dialog->addPage(i18n("Highscores"), QString::null,
                                   BarIcon("highscores", KIcon::SizeLarge));
    QVBoxLayout *top = new QVBoxLayout(page, dialog->spacingHint());
    _pageIndex = dialog->pageIndex(page);

    PlayerInfos pi;
    QGrid *grid = new QGrid(2, page);
    grid->setSpacing(dialog->spacingHint());
    top->addWidget(grid);
    QLabel *label = new QLabel(i18n("Nickname"), grid);
    _nickname = new QLineEdit((pi.isAnonymous() ? QString::null : pi.name()),
                              grid);
    _nickname->setMaxLength(16);
    QString name = pi.registeredName();
    if ( !pi.key().isEmpty() && !name.isEmpty() ) {
        (void)new QLabel(i18n("Registered nickname :"), grid);
        (void)new QLabel(name, grid);
    }

    label = new QLabel(i18n("Comment"), grid);
    _comment = new QLineEdit(pi.comment(), grid);
    _comment->setMaxLength(50);

    _WWHEnabled = new QCheckBox(i18n("world-wide highscores enabled"), page);
    _WWHEnabled->setChecked(pi.WWEnabled());
    top->addWidget(_WWHEnabled);

    top->addStretch(1);
}

bool HighscoresOption::accept()
{
    _dialog->showPage(_pageIndex);
    PlayerInfos pi;
    return pi.modifySettings(_nickname->text().lower(), _comment->text(),
                             _WWHEnabled->isChecked(), _dialog);
}
