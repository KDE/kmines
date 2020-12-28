/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// KF
#include <KXmlGuiWindow>
// Qt
#include <QPointer>
#include <QLabel>

class KMinesScene;
class KMinesView;
class KGameClock;
class KToggleAction;

class KMinesMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    KMinesMainWindow();
private Q_SLOTS:
    void onMinesCountChanged(int count);
    void newGame();
    void onGameOver(bool);
    void advanceTime(const QString&);
    void onFirstClick();
    void showHighscores();
    void configureSettings();
    void pauseGame(bool paused);
    void loadSettings();
private:
    void setupActions();
    KMinesScene* m_scene = nullptr;
    KMinesView* m_view = nullptr;
    KGameClock* m_gameClock = nullptr;
    KToggleAction* m_actionPause = nullptr;
    
    QPointer<QLabel> mineLabel = new QLabel;
    QPointer<QLabel> timeLabel = new QLabel;
};
#endif
