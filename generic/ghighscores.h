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
    Score(uint score = 0);

    uint nbEntries() const;
    uint score() const { return data("score").toUInt(); }
    QDateTime date() const { return data("date").toDateTime(); }
    uint id() const { return data("id").toUInt(); }
    void setName(const QString &name) { data("name") = name; }
};

//-----------------------------------------------------------------------------
class PlayerInfos : public ItemContainer
{
 public:
    PlayerInfos();

    static bool isNewPlayer();
    uint nbEntries() const;
    QString name() const { return item("name").read(_id).toString(); }
    bool isAnonymous() const;
    QString prettyName() const        { return prettyName(_id); }
    QString prettyName(uint id) const { return item("name").pretty(id); }
    QString registeredName() const;
    QString comment() const { return item("comment").pretty(_id); }
    bool WWEnabled() const;
    QString key() const;
    uint id() const { return _id; }

    void submitScore(bool won, const Score &) const;
    void submitBlackMark() const;
    void modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, const QString &newKey) const;

 private:
    uint _id;

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
    void setGameType(uint);

    HighscoresSettingsWidget *createSettingsWidget(BaseSettingsDialog *) const;
    void showHighscores(QWidget *parent) { _showHighscores(parent, -1); }
    void submitScore(bool won, Score &, QWidget *parent);
    void submitBlackMark(QWidget *parent);
    bool modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, QWidget *parent);

    uint nbScores() const;
    Score *firstScore() const { return readScore(0); }
    Score *lastScore() const;
    virtual bool isStrictlyBetter(const Score &, const Score &) const;

    QString scoreGroup() const;
    uint playerId() const;
    QString prettyPlayerName(uint id) const;
    virtual ItemBase *scoreItem() const { return new ScoreItem; }

    QString playerSubGroup() const;
    virtual ItemBase *bestScoreItem() const { return new BestScoreItem; }
    virtual ItemBase *meanScoreItem() const { return new MeanScoreItem; }
    virtual QString playersURL();
    virtual QString highscoresURL();

    virtual bool isLostGameEnabled() const  { return false; }
    virtual bool isBlackMarkEnabled() const { return false; }
    virtual bool isWWHSAvailable() const    { return !_baseURL.isEmpty(); }

 protected:
    enum LabelType { Standard, I18N, WW, Icon };
    virtual QString gameTypeLabel(uint gameType, LabelType) const = 0;
    virtual Score *score() const { return new Score; }
    virtual PlayerInfos *infos() const { return new PlayerInfos; }
    virtual void convertLegacy(uint /*gameType*/) {}
    int submitLocal(Score &, const QString &name) const;

    enum QueryType { Submit, Register, Change, Players, Scores };
    virtual void additionnalQueryArgs(QueryType, const Score *) {}
    void addToQueryURL(const QString &entry, const QString &content);

 private:
    enum ScoreType { Won = 0, Lost = -1, BlackMark = -2 };
    const QString _version;
    KURL          _baseURL;
    const uint    _nbGameTypes, _nbEntries;
    uint          _gameType;
    KURL          _url;

    void _showHighscores(QWidget *parent, int rank);
    Score *readScore(uint rank) const;
    int rank(const Score &) const; // return -1 if not a local best

    void submitWorldWide(ScoreType, const Score *, const PlayerInfos &,
                         QWidget *parent);
    void setQueryURL(QueryType, const QString &nickname, const Score * = 0);
    bool _doQuery(QDomNamedNodeMap &attributes, QString &error) const;
    bool doQuery(QDomNamedNodeMap &map, QWidget *parent) const;
    static bool getFromQuery(const QDomNamedNodeMap &map, const QString &name,
                             QString &value, QWidget *parent);
};

Highscores &highscores();

#endif
