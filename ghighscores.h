#ifndef G_HIGHSCORES_H
#define G_HIGHSCORES_H

#include <qdatastream.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qdom.h>

#include <kdialogbase.h>
#include <kconfig.h>
#include <kurl.h>
#include <klocale.h>

//-----------------------------------------------------------------------------
class ItemBase
{
 public:
    ItemBase() {}
    ItemBase(const QString &def, const QString &label, int alignment,
             bool canHaveSubGroup = false)
        : _default(def), _label(label), _alignment(alignment),
          _canHaveSubGroup(canHaveSubGroup) {}
    virtual ~ItemBase() {}

    void set(const QString &name, const QString &group,
             const QString &subGroup = QString::null);

    bool stored() const   { return !_group.isNull(); }
    bool shown() const    { return !_label.isEmpty(); }
    QString label() const { return _label; }
    int alignment() const { return _alignment; }
    QString name() const  { return _name; }

    virtual QString read(uint i) const;
    uint readUInt(uint i) const;
    double readDouble(uint i) const;
    virtual QString pretty(uint i) const { return read(i); }

    void write(uint i, const QString &value) const;
    void write(uint i, uint value) const;
    void write(uint i, double value) const;

    void moveDown(uint newIndex) const;

 private:
    QString _default, _name, _label, _group, _subGroup;
    int     _alignment;
    bool    _canHaveSubGroup;

    QString entryName() const;
};

//-----------------------------------------------------------------------------
class ItemContainer
{
 public:
    ItemContainer(const QString &group, const QString &subGroup);
    virtual ~ItemContainer() {}

    virtual uint nb() const = 0;
    const QList<ItemBase> &items() const { return _items; }

 protected:
    void addItem(const QString &key, ItemBase *, bool stored);
    QString group() const { return _group; }

    const ItemBase *item(const QString &n) const;
    uint name(const QString &name) const { return *_names[name]; }

 private:
    QString         _group, _subGroup;
    QList<ItemBase> _items;
    QDict<uint>     _names;

    ItemContainer(const ItemContainer &c);
    void operator =(const ItemContainer &);
};

class DataContainer : public ItemContainer
{
 public:
    DataContainer(const QString &group, const QString &subGroup)
        : ItemContainer(group, subGroup) {}

    QString prettyData(const QString &) const;

 protected:
    void addData(const QString &key, ItemBase *, bool stored, QVariant value);

    const QVariant &data(const QString &n) const
        { return _data[name(n)]; }
    QVariant &data(const QString &n)
        { return _data[name(n)]; }

 private:
    QValueList<QVariant> _data;

    friend QDataStream &operator >>(QDataStream &, DataContainer &);
    friend QDataStream &operator <<(QDataStream &, const DataContainer &);
};

QDataStream &operator >>(QDataStream &, DataContainer &);
QDataStream &operator <<(QDataStream &, const DataContainer &);

//-----------------------------------------------------------------------------
#define ANONYMOUS "_" // used to recognized anonymous players

class ScoreItemRank : public ItemBase
{
 public:
    ScoreItemRank()
        : ItemBase("0", i18n("Rank"), Qt::AlignRight) {}

    QString read(uint rank) const { return QString::number(rank+1); }
};

class ScoreItemScore : public ItemBase
{
 public:
    ScoreItemScore()
        : ItemBase("0", i18n("Score"), Qt::AlignRight) {}
};

class PlayerItemName : public ItemBase
{
 public:
    PlayerItemName()
        : ItemBase(QString::null, i18n("Name"), Qt::AlignLeft) {}

    virtual QString pretty(uint i) const {
        QString name = read(i);
        if ( name==ANONYMOUS ) return i18n("anonymous");
        return name;
    }
};

class PlayerItemMeanScore : public ItemBase
{
 public:
    PlayerItemMeanScore()
        : ItemBase("0", i18n("Mean score"), Qt::AlignRight, true) {}

    virtual QString pretty(uint i) const {
        double mean = readDouble(i);
        if ( mean==0 ) return "--";
        return QString::number(mean, 'f', 2);
    }
};

class PlayerItemBestScore : public ItemBase
{
 public:
    PlayerItemBestScore()
        : ItemBase("0", i18n("Best score"), Qt::AlignRight, true) {}

    virtual QString pretty(uint i) const {
        uint best = readUInt(i);
        if ( best==0 ) return "--";
        return QString::number(best);
    }
};

//-----------------------------------------------------------------------------
class Score : public DataContainer
{
 public:
    // a higher value is a better score
    Score(uint score, const QString &group = "scores",
          ItemBase *scoreItem = 0);

    uint score() const             { return data("score").toUInt(); }
    void setName(const QString &n) { data("name") = n; }
    QString name() const           { return data("name").toString(); }
    uint nb() const;
    uint firstScore() const        { return score(0); }
    uint lastScore() const;

    int submit(QWidget *parent, bool warn) const;

 private:
    uint score(uint rank) const { return item("score")->readUInt(rank); }
    int rank() const; // return -1 is not a best local
};

//-----------------------------------------------------------------------------
class PlayerInfos : public ItemContainer
{
 public:
    PlayerInfos(const QString &subGroup = QString::null,
                ItemBase *bestScoreItem = 0,
                ItemBase *meanScoreItem = 0);

    uint nb() const;
    QString name() const       { return item("name")->read(_id); }
    bool isAnonymous() const;
    QString prettyName() const { return item("name")->pretty(_id); }
    QString registeredName() const;
    QString comment() const    { return item("comment")->read(_id); }
    bool WWEnabled() const;
    QString key() const;
    uint id() const            { return _id; }

    int submitScore(Score &, QWidget *parent) const;
    bool modifySettings(const QString &newName,
                        const QString &comment, bool WWEnabled,
                        QWidget *parent) const;

    virtual QString playersURL() const;
    virtual QString highscoresURL() const;
    virtual QString showHighscoresCaption() const;

 protected:
    enum QueryType { Submit, Register, Change, Players, Highscores };
    bool _newPlayer;

    virtual void additionnalQueries(KURL &, QueryType) const {}
    static KURL URL(QueryType, const QString &nickname);
    static void addToURL(KURL &, const QString &entry, const QString &content);

 private:
    uint _id;

    KConfig *config() const;
    void addPlayer();

    static bool _doQuery(const KURL &url, QDomNamedNodeMap &attributes,
                         QString &error);
    static bool doQuery(const KURL &url, QDomNamedNodeMap &map,
                        QWidget *parent);
    static bool getFromQuery(const QDomNamedNodeMap &map, const QString &name,
                             QString &value, QWidget *parent);
};

//-----------------------------------------------------------------------------
class ShowHighscoresItem : public QListViewItem
{
 public:
    ShowHighscoresItem(QListView *, uint index, bool highlight);

 protected:
    virtual void paintCell (QPainter *, const QColorGroup &, int column,
                            int width, int align);

 private:
    uint _index;
    bool _highlight;
};

class ShowScores
{
 public:
    ShowScores() {}

 protected:
    QListView *createList(QWidget *parent) const;
    void addLine(QListView *, const ItemContainer &, int index,
                 bool highlight) const; // index==-1 : header
    virtual bool showColumn(const ItemBase *) const { return true; }
    virtual QString itemText(const ItemBase *, uint row) const = 0;
};

class ShowHighscores : public KDialogBase, public ShowScores
{
 Q_OBJECT
 public:
    ShowHighscores(int localRank, QWidget *parent, const Score &scoreDummy,
                   const PlayerInfos &playerDummy);

 private slots:
    void showURL(const QString &) const;

  private:
    QListView *_bestList, *_playersList;

    void fillList(QListView *, const ItemContainer &, int highlight) const;
    QString itemText(const ItemBase *, uint row) const;
};

//-----------------------------------------------------------------------------
class HighscoresOption
{
 public:
    HighscoresOption(KDialogBase *);

    bool accept();

 private:
    KDialogBase *_dialog;
    int          _pageIndex;
    QCheckBox   *_WWHEnabled;
    QLineEdit   *_nickname, *_comment;
};

#endif
