/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCENE_H
#define SCENE_H

// KDEGames
#include <KGameRenderer>
// Qt
#include <QGraphicsView>
#include <QGraphicsScene>

class MineFieldItem;
class KGamePopupItem;

/**
 * Graphics scene for KMines game
 */
class KMinesScene : public QGraphicsScene
{
    Q_OBJECT
public:
    /**
     * Constructs scene
     */
    explicit KMinesScene( QObject* parent );
    /**
     * Resizes scene to given dimensions
     */
    void resizeScene(int width, int height);
    /**
     * @return total number of mines in field
     */
    int totalMines() const;
    /**
     * Starts new game
     */
    void startNewGame(int rows, int cols, int numMines);
    /**
     * Toggles paused state for all cells in the field item
     */
    void setGamePaused(bool paused);
    /**
     * Resets the scene
     */
    void reset();

    KGameRenderer& renderer() {return m_renderer;}
    /**
     * Represents if the scores should be considered for the highscores
     */
    bool canScore() const;
    void setCanScore(bool value);

Q_SIGNALS:
    void minesCountChanged(int);
    void gameOver(bool);
    void firstClickDone();
private Q_SLOTS:
    void onGameOver(bool);
private:
    bool m_canScore;
    KGameRenderer m_renderer;
    /**
     * Game field graphics item
     */
    MineFieldItem* m_fieldItem = nullptr;
    KGamePopupItem* m_messageItem = nullptr;
    KGamePopupItem* m_gamePausedMessageItem = nullptr;
};

class QResizeEvent;

class KMinesView : public QGraphicsView
{
    Q_OBJECT
public:
    KMinesView( KMinesScene* scene, QWidget *parent );
private:
    void resizeEvent( QResizeEvent *ev ) override;

    KMinesScene* m_scene = nullptr;
};
#endif
