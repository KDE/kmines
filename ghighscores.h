#ifndef G_HIGHSCORES_H
#define G_HIGHSCORES_H

#include <qdatastream.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qdom.h>
#include <qtabwidget.h>
#include <qdict.h>

#include <klistview.h>
#include <kdialogbase.h>
#include <kconfig.h>
#include <kurl.h>
#include <klocale.h>

#include "gsettings.h"


//-----------------------------------------------------------------------------
class ItemBase
{
 public:
    ItemBase() {}
    ItemBase(const QVariant &def, const QString &label, int alignment,
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

	virtual QVariant read(uint i) const;
	virtual QString pretty(uint i) const;
	void write(uint i, const QVariant &) const;

    void moveDown(uint newIndex) const;

 private:
	QVariant _default;
    QString  _name, _label, _group, _subGroup;
    int      _alignment;
    bool     _canHaveSubGroup;

    QString entryName() const;
};

//-----------------------------------------------------------------------------
class ItemContainer
{
 public:
    ItemContainer(const QString &group, const QString &subGroup);
    virtual ~ItemContainer() {}

    virtual uint nbEntries() const = 0;
    const QPtrList<ItemBase> &items() const { return _items; }

 protected:
    void addItem(const QString &key, ItemBase *, bool stored);
    QString group() const { return _group; }

    const ItemBase &item(const QString &name) const;
    uint name(const QString &name) const { return *_names[name]; }

 private:
    QString            _group, _subGroup;
    QPtrList<ItemBase> _items;
    QDict<uint>        _names;

    ItemContainer(const ItemContainer &c);
    void operator =(const ItemContainer &);
};

//-----------------------------------------------------------------------------
class DataContainer : public ItemContainer
{
 public:
    DataContainer(const QString &group, const QString &subGroup)
        : ItemContainer(group, subGroup) {}

    void read(uint i);
    void write(uint i, uint maxNbLines) const;
    QString prettyData(const QString &name) const;

 protected:
    const QVariant &data(const QString &n) const { return _data[name(n)]; }
    QVariant &data(const QString &n)             { return _data[name(n)]; }
    void addData(const QString &key, ItemBase *, bool stored, QVariant value);

 private:
    QValueList<QVariant> _data;

    friend QDataStream &operator >>(QDataStream &, DataContainer &);
    friend QDataStream &operator <<(QDataStream &, const DataContainer &);
};

QDataStream &operator >>(QDataStream &, DataContainer &);
QDataStream &operator <<(QDataStream &, const DataContainer &);

//-----------------------------------------------------------------------------
extern const char *ANONYMOUS; // used to recognized anonymous players

class ScoreItemRank : public ItemBase
{
 public:
    ScoreItemRank()
        : ItemBase((uint)0, i18n("Rank"), Qt::AlignRight) {}

    QVariant read(uint rank) const  { return rank; }
    QString pretty(uint rank) const { return QString::number(rank+1); }
};

class ScoreItemScore : public ItemBase
{
 public:
    ScoreItemScore()
        : ItemBase((uint)0, i18n("Score"), Qt::AlignRight) {}
};

class PlayerItemName : public ItemBase
{
 public:
    PlayerItemName()
        : ItemBase(QString::null, i18n("Name"), Qt::AlignLeft) {}

    virtual QString pretty(uint i) const {
        QString name = ItemBase::pretty(i);
        if ( name==ANONYMOUS ) return i18n("anonymous");
        return name;
    }
};

class PlayerItemMeanScore : public ItemBase
{
 public:
    PlayerItemMeanScore()
        : ItemBase((double)0, i18n("Mean score"), Qt::AlignRight, true) {}

    virtual QString pretty(uint i) const {
        double mean = read(i).toDouble();
        if ( mean==0 ) return "--";
        return QString::number(mean, 'f', 1);
    }
};

class PlayerItemBestScore : public ItemBase
{
 public:
    PlayerItemBestScore()
        : ItemBase((uint)0, i18n("Best score"), Qt::AlignRight, true) {}

    virtual QString pretty(uint i) const {
		QString best = ItemBase::pretty(i);
        if ( best=="0" ) return "--";
        return best;
    }
};

class PlayerItemWin : public ItemBase
{
 public:
    PlayerItemWin()
        : ItemBase((double)-1, i18n("Success"), Qt::AlignRight, true) {}

    QString pretty(uint i) const {
        double win = read(i).toDouble();
        if ( win==-1 ) return "--";
        return QString::number(win, 'f', 1) + " %";
    }
};

//-----------------------------------------------------------------------------
class Score : public DataContainer
{
 public:
    // a higher value is a better score
    // 0 is a valid score but it cannot enter highscores list.
    Score(uint score = 0);

    uint nbEntries() const;
    uint score() const { return data("score").toUInt(); }
    void setName(const QString &name) { data("name") = name; }
};

//-----------------------------------------------------------------------------
class PlayerInfos : public ItemContainer
{
 public:
    PlayerInfos();

    static bool isNewPlayer();
    uint nbEntries() const;
    QString name() const       { return item("name").read(_id).toString(); }
    bool isAnonymous() const;
    QString prettyName() const { return item("name").pretty(_id); }
    QString registeredName() const;
    QString comment() const    { return item("comment").pretty(_id); }
    bool WWEnabled() const;
    QString key() const;
    uint id() const            { return _id; }

    void submitScore(bool won, const Score &) const;
    void submitBlackMark() const;
    void modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, const QString &newKey) const;

 private:
    uint _id;

    static KConfig *config();
    void addPlayer();
};

//-----------------------------------------------------------------------------
class ShowHighscoresItem : public KListViewItem
{
 public:
    ShowHighscoresItem(QListView *, bool highlight);

 protected:
    virtual void paintCell(QPainter *, const QColorGroup &, int column,
						   int width, int align);

 private:
    bool _highlight;
};

class ShowScoresList : public KListView
{
 Q_OBJECT
 public:
    ShowScoresList(QWidget *parent);

 protected:
    // index==-1 : header
    void addLine(const ItemContainer &, int index, bool highlight);
    virtual bool showColumn(const ItemBase *) const { return true; }
    virtual QString itemText(const ItemBase *, uint row) const = 0;
};

//-----------------------------------------------------------------------------
class ShowHighscoresList : public ShowScoresList
{
 Q_OBJECT
 public:
    ShowHighscoresList(const ItemContainer &, int highlight, QWidget *parent);

 protected:
    QString itemText(const ItemBase *, uint row) const;
};

class ShowHighscoresWidget : public QWidget
{
 Q_OBJECT
 public:
    ShowHighscoresWidget(int localRank, QWidget *parent, const Score &,
                         const PlayerInfos &, int spacingHint);

 private slots:
    void showURL(const QString &) const;
};

//-----------------------------------------------------------------------------
class ShowMultiScoresList : public ShowScoresList
{
 Q_OBJECT
 public:
    ShowMultiScoresList(const QPtrVector<Score> &, QWidget *parent);

 private:
    const QPtrVector<Score> _scores;

    bool showColumn(const ItemBase *) const;
    QString itemText(const ItemBase *, uint row) const;
};

class ShowMultiScoresDialog : public KDialogBase
{
 Q_OBJECT
 public:
    ShowMultiScoresDialog(const QPtrVector<Score> &, QWidget *parent);
};

//-----------------------------------------------------------------------------
class HighscoresSettingsWidget : public BaseSettingsWidget
{
 Q_OBJECT
 public:
    HighscoresSettingsWidget(BaseSettingsDialog *parent, PlayerInfos *);

    bool writeConfig();

 private:
    QCheckBox *_WWHEnabled;
    QLineEdit *_nickname, *_comment;
};

//-----------------------------------------------------------------------------
class Highscores
{
 public:
    // empty baseURL means WWW HS not available
    Highscores(const QString version, const KURL &baseURL = KURL(),
               uint nbGameTypes = 1, uint nbEntries = 10);
    virtual ~Highscores() {}
    void init();
    void setGameType(uint);

    HighscoresSettingsWidget *createSettingsWidget(BaseSettingsDialog *) const;
    void showHighscores(QWidget *parent) { _showHighscores(parent, -1); }
    void submitScore(bool won, Score &, QWidget *parent);
    void submitBlackMark(QWidget *parent) const;
    bool modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, QWidget *parent) const;

    uint nbScores() const;
    Score *firstScore() const { return readScore(0); }
    Score *lastScore() const;
    virtual bool isStrictlyBetter(const Score &, const Score &) const;

    QString scoreGroup() const;
    virtual ItemBase *scoreItemScore() const { return new ScoreItemScore; }

    QString playerSubGroup() const;
    virtual ItemBase *playerItemBestScore() const
        { return new PlayerItemBestScore; }
    virtual ItemBase *playerItemMeanScore() const
        { return new PlayerItemMeanScore; }
    virtual QString playersURL() const;
    virtual QString highscoresURL() const;

    virtual bool isLostGameEnabled() const  { return false; }
    virtual bool isBlackMarkEnabled() const { return false; }
    virtual bool isWWHSAvailable() const    { return !_baseURL.isEmpty(); }

 protected:
    enum LabelType { Standard, I18N, WW, Icon };
    virtual QString gameTypeLabel(uint /*gameType*/, LabelType) const
        { return QString::null; }
    virtual Score *score() const { return new Score; }
    virtual PlayerInfos *infos() const { return new PlayerInfos; }
    virtual void convertLegacy(uint /*gameType*/) {}
    int submitLocal(Score &, const QString &name) const;

    enum QueryType { Submit, Register, Change, Players, Scores };
    virtual void additionnalQueries(KURL &, QueryType) const {}
    KURL URL(QueryType, const QString &nickname) const;
    static void addToURL(KURL &, const QString &entry, const QString &content);

 private:
    const QString _version;
    const KURL    _baseURL;
    const uint    _nbGameTypes, _nbEntries;
    uint          _gameType;

    void _showHighscores(QWidget *parent, int rank);
    Score *readScore(uint rank) const;
    int rank(const Score &) const; // return -1 if not a local best

    static const int LOST_GAME_ID;
    static const int BLACK_MARK_ID;
    void submitWorldWide(int score, const PlayerInfos &,
                         QWidget *parent) const;

    static bool _doQuery(const KURL &url, QDomNamedNodeMap &attributes,
                         QString &error);
    static bool doQuery(const KURL &url, QDomNamedNodeMap &map,
                        QWidget *parent);
    static bool getFromQuery(const QDomNamedNodeMap &map, const QString &name,
                             QString &value, QWidget *parent);
};

Highscores &highscores();

#endif
