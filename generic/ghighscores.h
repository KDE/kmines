#ifndef G_HIGHSCORES_H
#define G_HIGHSCORES_H

#include <qdom.h>

#include <kurl.h>

#include "ghighscores_gui.h"


//-----------------------------------------------------------------------------
class Score : public DataContainer
{
 public:
    // a higher value is a better score
    // 0 is a valid score but it cannot enter highscores list.
    Score(uint score = 0);

    uint nbEntries() const;
    uint score() const { return data("score").toUInt(); }
    QDateTime date() const { return data("date").toDateTime(); }
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
    virtual ItemBase *scoreItem() const { return new ScoreItem; }

    QString playerSubGroup() const;
    virtual ItemBase *bestScoreItem() const { return new BestScoreItem; }
    virtual ItemBase *meanScoreItem() const { return new MeanScoreItem; }
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
