/*
    Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
  
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "canvaswidget.h"
#include "settings.h"

#include <QGraphicsObject>
#include <KGameRenderer>
#include <KStandardDirs>

CanvasWidget::CanvasWidget(KGameRenderer *renderer, QWidget *parent) :
    KgDeclarativeView(renderer, parent)
{
    QString path = KStandardDirs::locate("appdata", "qml/main.qml");
    setSource(QUrl::fromLocalFile(path));

    // forward signals from QML
    connect(rootObject(), SIGNAL(minesCountChanged(int,int)), this, SIGNAL(minesCountChanged(int,int)));
    connect(rootObject(), SIGNAL(firstClickDone()), this, SIGNAL(firstClickDone()));
    connect(rootObject(), SIGNAL(gameOver(bool)), this, SIGNAL(gameOver(bool)));

    updateUseQuestionMarks();
}

void CanvasWidget::setGamePaused(bool paused)
{
    QMetaObject::invokeMethod(rootObject(), "setGamePaused", Q_ARG(QVariant, paused));
}

void CanvasWidget::startNewGame(int rows, int cols, int numMines)
{
    QMetaObject::invokeMethod(rootObject(), "startNewGame",
                              Q_ARG(QVariant, rows),
                              Q_ARG(QVariant, cols),
                              Q_ARG(QVariant, numMines));
}

void CanvasWidget::updateUseQuestionMarks()
{
    rootObject()->setProperty("useQuestionMarks", Settings::useQuestionMarks());
}
