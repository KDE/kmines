/*
    This file is part of the KDE games library
    Copyright (C) 2001-02 Nicolas Hadacek (hadacek@kde.org)

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

#ifndef KEXTHIGHSCORE_H
#define KEXTHIGHSCORE_H

#include "ghighscores_item.h"

class KURL;
class QTabWidget;


namespace KExtHighscore
{

class Score;
class Item;

class ManagerPrivate;
extern ManagerPrivate *internal;

/**
 * Get the current game type.
 */
uint gameType();

/**
 * Set the current game type.
 */
void setGameType(uint gameType);

/**
 * Configure the highscores.
 * @return true if the configuration has been modified and saved
 */
bool configure(QWidget *parent);

/**
 * Show the highscores lists.
 */
void show(QWidget *parent);

/**
 * Submit a score. See @ref Manager for usage example.
 *
 * @param widget a widget used as parent for error message box.
 */
void submitScore(const Score &score, QWidget *widget);

/**
 * @return the last score in the local list of highscores. The worst possible
 * score if there are less items than the maximum number.
 */
Score lastScore();

/**
 * @return the first score in the local list of highscores (the worst possible
 * score if there is no entry).
 */
Score firstScore();

/**
 * This class manages highscores and players entries (several players can
 * share the same highscores list if the libkdegame library is built to
 * support a common highscores file; NOTE that to correctly implement such
 * feature we probably need a locking mechanism in @ref KHighscore).
 *
 * You need one instance of this class during the application lifetime ; in
 * main() just insert <pre> KExtHighscore::Manager highscoresManager; </pre>
 * with the needed arguments. Use the derived class if you need to
 * reimplement some of the default methods.
 *
 * This class has three functions :
 * <ul>
 * <li> Update the highscores list when new entries are submitted </li>
 * <li> Display the highscores list and the players list </li>
 * <li> Send query to an optionnal web server to support world-wide
 *      highscores </li>
 * </ul>
 *
 * The highscores and the players lists contain several items described by
 * the @ref Item class.
 *
 * The highscores list contains by default :
 * <ul>
 * <li> the player name (automatically set from the config value)</li>
 * <li> the score value </li>
 * <li> the time and date of the highscore (automatically set) </li>
 * </ul>
 * You can replace the score item (for e.g. displaying it differently) with
 * @ref setScoreItem or add an item with @ref addScoreItem.
 *
 * The players list contains :
 * <ul>
 * <li> the player name (as defined by the user in the configuration
 *      dialog) </li>
 * <li> the number of games played </li>
 * <li> the mean score </li>
 * <li> the best score </li>
 * <li> the best score time and date </li>
 * <li> the player comment (as defined by the user in the
 *      configuration dialog) </li>
 * </ul>
 * You can replace the best score and the mean score item
 * by calling @ref setPlayerItem.
 *
 * To submit a new score at game end, just construct a @ref Score, set the
 * score data and then call @ref submitScore.
 * <pre>
 *     KExtHighscore::Score score(KExtHighscore::Won);
 *     score.setScore(myScore);
 *     KExtHighscore::submitScore(score, widget);
 * </pre>
 * You only need to set the score value with @ref Score::setScore
 * and the value of the items that you have optionnally added
 * with @ref Score::setData ; player name and date are set automatically.
 */
class Manager
{
 public:
    /**
     * Constructor
     *
     * @param nbGameTypes the number of different game types (usually one).
     *        For example KMines has easy, normal and expert levels.
     * @param maxNbEntries the maximum numbers of highscores entries (by game
     *        types)
     */
    Manager(uint nbGameTypes = 1, uint maxNbEntries = 10);
    virtual ~Manager();

    /**
     * Set the world-wide highscores.
     *
     * Note: should be called at construction time.
     *
     * @param url the web server url
     * @param version the game version which is sent to the wrb server (it can
     * be useful for backward compatibility on the server side).
     */
    void setWWHighscores(const KURL &url, const QString &version);

    /**
     * Set if the number of lost games should be track for the world-wide
     * highscores statistics. By default, there is no tracking.
     *
     * Note: should be called at construction time.
     */
    void setTrackLostGames(bool track);

    /**
     * Set if the statistics tab should be shown in the highscores dialog.
     * You only want to show this tab if it makes sense to lose or to win the
     * game (for e.g. it makes no sense for a tetris game but it does for a
     * minesweeper game).
     *
     * Note: should be called at construction time.
     */
    void showStatistics(bool show);

    enum ScoreTypeBound { ScoreNotBound, ScoreBound };
    /**
     * Set the ranges for the score histogram.
     *
     * Note: should be called at construction time.
     */
    void setScoreHistogram(const QMemArray<uint> &scores, ScoreTypeBound type);

    enum ShowMode { AlwaysShow, NeverShow, ShowForHigherScore,
                    ShowForHighestScore };
    /**
     * Set how the highscores dialog is shown at game end.
     * By default, the mode is @ref ShowAtHigherScore.
     *
     * Note: should be called at construction time.
     */
    void setShowMode(ShowMode mode);

    /**
     * Score type (@see setScoreType).
     * @p Normal default score (unsigned integer without upper bound)
     * @p MinuteTime score by time bound at 3599 seconds (for e.g. kmines)
     */
    enum ScoreType { Normal, MinuteTime };
    /**
     * Set score type. Helper method to quickly set the type of score.
     *
     * Note: should be called at construction time.
     */
    void setScoreType(ScoreType type);

    /**
     * Some predefined item types.
     * @p Score default item for the score in the highscores list.
     * @p MeanScore default item for the mean score (only show one decimal and
     * 0 is shown as "--".
     * @p BestScore default item for the best score (0 is shown as "--").
     * @p Optionnal item for elapsed time (maximum value is 3599 seconds).
     */
    enum ItemType { ScoreDefault, MeanScoreDefault, BestScoreDefault,
                    ElapsedTime };
    /**
     * Create a predefined item.
     */
    static Item *createItem(ItemType type);

    /**
     * Replace the default score item in the highscores list by the given one.
     * @p worstScore is the worst possible score. By default it is 0.
     *
     * Note : This method should be called at construction time.
     */
    void setScoreItem(uint worstScore, Item *item);

    /**
     * Add an item in the highscores list (it will add a column to this list).
     *
     * Note : This method should be called at construction time.
     */
    void addScoreItem(const QString &name, Item *item);

    enum PlayerItemType { MeanScore, BestScore };
    /**
     * Replace an item in the players list.
     *
     * Note : This method should be called at construction time.
     */
    void setPlayerItem(PlayerItemType type, Item *item);

    /**
     * @return true if the first score is strictly worse than the second one.
     * By default return <pre>s1.score()<s2.score()</pre>. You can reimplement
     * this method if additionnal items added to @ref Score can further
     * differentiate the scores (for e.g. the time spent).
     *
     * Note that you do not need to use directly this method, simply write
     * <pre>s1<s2</pre> since the operator calls this method.
     */
    virtual bool isStrictlyLess(const Score &s1, const Score &s2) const;

    /**
     * Possible type of label (@see gameTypeLabel).
     * @p Standard label used in config file.
     * @p I18N label used to display the game type.
     * @p WW label used when contacting the world-wide highscores server.
     * @p Icon label used to load the icon corresponding to the game type.
     */
    enum LabelType { Standard, I18N, WW, Icon };

    /**
     * @return the label corresponding to the game type. The default
     * implementation works only for one game type : you need to reimplement
     * this method if the number of game types is more than one.
     */
    virtual QString gameTypeLabel(uint gameType, LabelType type) const;

 protected:
    /**
     * This method is called once for each player (ie for each user). You
     * can reimplement it to convert old style highscores to the new mechanism
     * (@see submitLegacyScore). By default this method does nothing.
     *
     * @param gameType the game type
     */
    virtual void convertLegacy(uint gameType) { Q_UNUSED(gameType); }

    /**
     * This method should be called from @ref convertLegacy. It is used
     * to submit an old highscore (it will not be send over the network).
     * For each score do something like:
     * <pre>
     * Score score(Won);
     * score.setScore(oldScore);
     * score.setData("name", name);
     * submitLegacyScore(score);
     * </pre>
     * Note that here you can set the player "name" and the highscore "date"
     * if they are known.
     */
    void submitLegacyScore(const Score &score) const;

    /**
     * This method is called before submitting a score to the world-wide
     * highscores server. You can reimplement this method to add an entry
     * with @ref addToQueryURL. By default this method does nothing.
     *
     * @param score the score to be submitted.
     */
    virtual void additionnalQueryItems(KURL &url, const Score &score) const
        { Q_UNUSED(url); Q_UNUSED(score); }

    /**
     * Add an entry to the url to be submitted (@see additionnalQueryItems).
     *
     * @param item the item name
     * @param content the item content
     */
    static void addToQueryURL(KURL &url, const QString &item,
                              const QString &content);

    friend class ManagerPrivate;

 private:
    Manager(const Manager &);
    Manager &operator =(const Manager &);
};

}; // namespace

#endif
