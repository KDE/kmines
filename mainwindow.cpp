/*
    Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "mainwindow.h"
#include "scene.h"

#include <KStandardGameAction>
#include <KActionCollection>

KMinesMainWindow::KMinesMainWindow()
{
    KMinesScene* m_scene = new KMinesScene(this);

    KMinesView* view = new KMinesView( m_scene, this );
    view->setCacheMode( QGraphicsView::CacheBackground );
    view->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    view->setFrameStyle(QFrame::NoFrame);

    view->setOptimizationFlags( QGraphicsView::DontClipPainter |
                                  QGraphicsView::DontSavePainterState |
                                  QGraphicsView::DontAdjustForAntialiasing );

    setCentralWidget(view);
    setupActions();
}

void KMinesMainWindow::setupActions()
{
    KStandardGameAction::quit(this, SLOT(close()), actionCollection());
    setupGUI();
}
