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
const char *HS_ID         = "player id";
const char *HS_KEY        = "player key";
const char *HS_WW_ENABLED = "ww hs enabled";

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
        KURL url = URL(Submit, name());
        addToURL(url, "key", key());
        addToURL(url, "version", VERSION);
        QString str =  QString::number(score.score());
        addToURL(url, "score", str);
        KMD5 context(name() + str);
        addToURL(url, "check", context.hexDigest());
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
        bool newPlayer = key().isEmpty();
	    if (newPlayer) url = URL(Register, newName);
		else {
		    url = URL(Change, name());
			addToURL(url, "key", key());
			if ( name()!=newName ) addToURL(url, "new_nickname", newName);
		}
		addToURL(url, "comment", comment);
		addToURL(url, "version", VERSION);
        QDomNamedNodeMap map;
		if ( !doQuery(url, map, parent) ) return false;
		if (newPlayer) {
            if ( !getFromQuery(map, "key", newKey, parent) ) return false;
            config()->writeEntry(HS_KEY, newKey);
        }
    }

    item("name")->write(_id, newName);
    item("comment")->write(_id, comment);
    config()->writeEntry(HS_WW_ENABLED, WWEnabled);

    return true;
}

KURL PlayerInfos::URL(QueryType type, const QString &nickname)
{
    KURL url;
    url.setProtocol("http");
    url.setHost(WORLD_WIDE_HS_HOST);
	switch (type) {
	case Submit:     url.setPath("submit.php");     break;
	case Register:   url.setPath("register.php");   break;
	case Change:     url.setPath("change.php");     break;
	case Highscores: url.setPath("highscores.php"); break;
	}
	QString query = "nickname=" + KURL::encode_string(nickname);
	url.setQuery(query);
	return url;
}

void PlayerInfos::addToURL(KURL &url, const QString &entry,
                           const QString &content)
{
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
  	    error = KIO::NetAccess::lastErrorString();
	    return false;
	}
	QFile file(tmpFile);
	if ( !file.open(IO_ReadOnly) ) {
	    error = i18n("Unable to open temporary file");
	    return false;
	}
	QTextStream t(&file);
	QString content = t.read();
	file.close();
	if ( content.isEmpty() ) {
        error = i18n("no data in answer");
        return false;
    }
    if ( content.mid(0, 5)!="<xml>" ) {
        error = content;
        return false;
    }
	QDomDocument doc;
    doc.setContent(content);
    QDomElement root = doc.documentElement();
    QDomElement element = root.firstChild().toElement();
    if ( element.isNull() || element.tagName()!="success" ) {
 	    error = i18n("Invalid answer");
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
  if ( !ok ) KMessageBox::sorry(parent, error);
  return ok;
}

bool PlayerInfos::getFromQuery(const QDomNamedNodeMap &map,
                               const QString &name, QString &value,
                               QWidget *parent)
{
    QDomAttr attr = map.namedItem(name).toAttr();
    if ( attr.isNull() ) {
	    KMessageBox::sorry(parent, i18n("Invalid answer"));
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
    : KDialogBase(Tabbed, i18n("Highscores"), Close, Close,
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

    // player infos
    page = addVBoxPage(i18n("Players"));
    _playersList = createList(page);
    fillList(_playersList, playerDummy, playerDummy.id());

    enableButtonSeparator(false);
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
    label = new QLabel(i18n("Comment"), grid);
    _comment = new QLineEdit(pi.comment(), grid);
    _comment->setMaxLength(50);

    _WWHEnabled = new QCheckBox(i18n("world-wide highscores enabled"), page);
    _WWHEnabled->setChecked(pi.WWEnabled());
    _WWHEnabled->setEnabled(false); // #### FIXME
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
