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

#ifndef G_HIGHSCORES_H
#define G_HIGHSCORES_H

#include <qvaluelist.h>

#include <kurl.h>

#include "ghighscores_item.h"


#define kHighscores KExtHighscores::Highscores::highscores()

class QDomNamedNodeMap;
class QWidget;
class KSettingWidget;

namespace KExtHighscores
{

class PlayerInfos;
typedef QValueList<Score> ScoreList;

/**
 * This class manages highscores and players entries (several players can
 * share the same highscore list if the libkdegames library is built to
 * support a common highscore file).
 *
 * You need to inherit this class to implement the abstract method
 * @ref gameTypeLabel. You need one instance of this class during the
 * application lifetime.
 *
 * This class has three functions :
 * <ul>
 * <li> Update the highscores list when new entries are submitted </li>
 * <li> Display the highscores list and the players list </li>
 * <li> Send query to an optionnal web server to support world-wide
 *      highscores </li>
 * </ul>
 *
 * The highscores and the players lists contain several items (represented by
 * an @ref Item).
 *
 * The highsores list contains by default :
 * <ul>
 * <li> "id" : the index of the player (internal and not shown) </li>
 * <li> "name" : the player name (automatically set) </li>
 * <li> "score" : the score value </li>
 * <li> "date" : the time and date of the highscore (automatically set) </li>
 * </ul>
 * You can add an item by calling @ref addScoreItem right after construction.
 * You can replace the default item for the score value with @ref setScoreItem
 * (for e.g. displaying it differently).
 *
 * The players list contains :
 * <ul>
 * <li> "name" : the player name (as defined by the user in the configuration
 *      dialog) </li>
 * <li> "nb game" : the number of games </li>
 * <li> "success" : the number of successes (only if @p trackLostGame is set
 *      in the construtor </li>
 * <li> "mean score" : the mean score </li>
 * <li> "best score" : the best score </li>
 * <li> "black mark" : the number of black marks (only if @p trackBlackMark is
 *      set in the constructor) ; black marks should be called if the user
 *      abort a game and you consider it is somehow cheating the
 *      statistics </li>
 * <li> "date" : the best score time and date </li>
 * <li> "comment" : the player comment (as defined by the user in the
 *      configuration dialog) </li>
 * </ul>
 * You can replace the best score and the mean score item with
 * @ref setBestScoreItem and @ref setMeanScoreItem.
 *
 * To submit a new score at game end, just construct a @ref Score, set the
 * score data and then call @ref submitScore.
 * <pre>
 *     KExtHighscores::Score score(KExtHighscores::Won);
 *     score.setData("score", myScore);
 *     kHighscores->submitScore(score, widget); // delete score
 * </pre>
 * You can only set the score value ("name" and "date" are set automatically)
 * and the value of the item you have (optionnally) added with
 * @ref addScoreItem.
 */
class Highscores
{
 public:
    /**
     * Constructor
     *
     * @param version the game version which is sent to the web server (it is
     *        useful for backward compatibility on the server side).
     * @param baseURL the web server url (an empty url means that world-wide
     *        highscores are not available)
     * @param nbGameTypes the number of different game types (usually one) ;
     *        for e.g. KMines has easy, normal and expert levels.
     * @param maxNbEntries the maximum numbers of highscores entries (by game
     *        types)
     * @param trackLostGame if true, tracks the number of lost games
     * @param trackBlackMark if true, tracks the number of black marks
     */
    Highscores(const QString &version, const KURL &baseURL = KURL(),
                   uint nbGameTypes = 1, uint maxNbEntries = 10,
                   bool trackLostGames = false, bool trackBlackMarks = false);
    virtual ~Highscores();

    /**
     * Set the current game type.
     */
    void setGameType(uint);

    /**
     * @return a pointer to the instance of the class.
     */
    static Highscores *highscores() { return _highscores; }

    /**
     * @return a @ref KSettings::Widget.
     */
    KSettingWidget *createSettingsWidget(QWidget *parent);

    /**
     * Show the highscores lists.
     */
    void showHighscores(QWidget *parent) { _showHighscores(parent, -1); }

    /**
     * Show scores for a multiplayers game.
     *
     * Usage example :
     * <pre>
     * KExtHighscores::ScoreList scores(2);
     * scores[0].setType(KExtHighscores::Won);
     * scores[0].setData("score", score1);
     * scores[0].setData("name", player1);
     * scores[1].setType(KExtHighscores::Lost);
     * scores[1].setData("score", score2);
     * scores[1].setData("name", player2);
     * kHighscores->showMultipleScores(scores, widget);
     * </pre>
     */
    void showMultipleScores(const ScoreList &scores, QWidget *parent) const;

    /**
     * Submit the score generated by @ref newScore. Note that the @ref Score
     * object is deleted by this method.
     *
     * @param widget a widget used as parent for error message box.
     */
    void submitScore(const Score &score, QWidget *widget);

    /**
     * @return the last score in the local list of highscores.
     */
    Score lastScore();

    /**
     * @return the first score in the local list of highscores.
     */
    Score firstScore();

    /**
     * @return true is the first score is strictly worse than the second one.
     * By default return score1.score()<score2.score(). You can reimplement
     * this method if additionnal items added to @ref Score can further
     * differentiate the scores (for e.g. the time spent).
     *
     * Note that you can simply write <pre> score1<score2 </pre> since
     * @ref Score::operator< calls this method.
     */
    virtual bool isStrictlyWorse(const Score &score1,
                                 const Score &score2) const;

    /**
     * @internal
     */
    const ScoreInfos *scoreInfos() const { return _scoreInfos; }

    /**
     * @internal
     */
    bool modifySettings(const QString &newName, const QString &comment,
                        bool WWEnabled, QWidget *parent);

 protected:
    /**
     * Possible tpye of label (@see gameTypeLabel).
     * @p Standard label used in config file.
     * @p I18N label used to display the game type.
     * @p WW label used when contacting the world-wide highscores server.
     * @p Icon label used to load the icon corresponding to the game type (if
     *    there is one).
     */
    enum LabelType { Standard, I18N, WW, Icon };

    /**
     * @return the label corresponding to the game type. The default
     * implementation is ok for one game type. But you need to reimplement
     * this method if you declare more than one at construction time.
     */
    virtual QString gameTypeLabel(uint gameType, LabelType) const;

    /**
     * Add an item to the score. It will add a column to the highscores list.
     * Should be called at construction time.
     *
     * if name is "score", "best score" or "mean score", the default item
     * will be replaced by the given one.
     */
    void setItem(const QString &name, Item *item);

    /**
     * This method is called once for each player (ie for each user). You
     * can reimplement it to convert old style highscores to the new mechanism.
     *
     * By default this method does nothing.
     * @see submitLegacyScore.
     *
     * @param gameType the game type
     */
    virtual void convertLegacy(uint) {}

    /**
     * This method should be called from @ref convertLegacy. It is used
     * to submit (only locally) an old highscore.
     * For each score do something like :
     * <pre>
     * Score score = newScore(Won);
     * score.setData("score", oldScore);
     * score.setData("name", name);
     * submiteLegacyScore(score);
     * </pre>
     * Note that here you can set the player "name" and the highscore "date"
     * if they are known.
     */
    void submitLegacyScore(const Score &score) const { submitLocal(score); }

    /**
     * This method is called before submitting a score to the world-wide
     * highscores server. You can reimplement this method to add an entry
     * with @ref addQueryURL.
     *
     * By default this method do nothing.
     *
     * @param score the score to be submitted.
     */
    virtual void additionnalQueryItems(const Score &) {}

    /**
     * Add an entry to the query to be sent.
     * @see additionnalQueryEntries
     *
     * @param item the item name
     * @param content the item content
     */
    void addToQueryURL(const QString &item, const QString &content);

 private:
    enum QueryType { Submit, Register, Change, Players, Scores };
    static Highscores *_highscores;
    const QString _version;
    KURL          _baseURL;
    const uint    _nbGameTypes;
    uint          _gameType;
    KURL          _url;
    bool          _first;
    PlayerInfos  *_playerInfos;
    ScoreInfos   *_scoreInfos;

    class HighscoresPrivate;
    HighscoresPrivate *d;

    void _showHighscores(QWidget *parent, int rank);
    int rank(const Score &score) const; // return -1 if not a local best score

    int submitLocal(const Score &score) const;
    void submitWorldWide(const Score &score, QWidget *parent);
    void setQueryURL(QueryType, const QString &nickname,
                     const Score *score = 0);
    bool doQuery(QDomNamedNodeMap &map, QWidget *parent) const;
    static bool getFromQuery(const QDomNamedNodeMap &map, const QString &name,
                             QString &value, QWidget *parent);

    Highscores(const Highscores &);
    Highscores &operator =(const Highscores &);
};

}; // namespace

#endif
