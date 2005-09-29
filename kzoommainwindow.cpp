/*
    This file is part of the KDE games library
    Copyright (C) 2004 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kzoommainwindow.h"
#include "kzoommainwindow.moc"

#include <kaction.h>
#include <kstdaction.h>
#include <kmenubar.h>
#include <kcmenumngr.h>

KZoomMainWindow::KZoomMainWindow(uint min, uint max, uint step, const char *name)
  : KMainWindow(0, name), _zoomStep(step), _minZoom(min), _maxZoom(max)
{
  installEventFilter(this);
  
  _zoomInAction = KStdAction::zoomIn(this, SLOT(zoomIn()), actionCollection());
  _zoomOutAction =
    KStdAction::zoomOut(this, SLOT(zoomOut()), actionCollection());
  _menu =
    KStdAction::showMenubar(this, SLOT(toggleMenubar()), actionCollection());
}

void KZoomMainWindow::init(const char *popupName)
{
  // zoom
  setZoom(readZoomSetting());

  // menubar
  _menu->setChecked( menubarVisibleSetting() );  
  toggleMenubar();
  
  // context popup
  if (popupName) {
    QPopupMenu *popup =
      static_cast<QPopupMenu *>(factory()->container(popupName, this));
    Q_ASSERT(popup);
    if (popup) KContextMenuManager::insert(this, popup);
  }
}

void KZoomMainWindow::addWidget(QWidget *widget)
{
  widget->adjustSize();
  QWidget *tlw = widget->topLevelWidget();
  KZoomMainWindow *zm = 
    static_cast<KZoomMainWindow *>(tlw->qt_cast("KZoomMainWindow"));
  Q_ASSERT(zm);
  zm->_widgets.append(widget);
  connect(widget, SIGNAL(destroyed()), zm, SLOT(widgetDestroyed()));
}

void KZoomMainWindow::widgetDestroyed()
{
  _widgets.remove(static_cast<const QWidget *>(sender()));
}

bool KZoomMainWindow::eventFilter(QObject *o, QEvent *e)
{
  if ( e->type()==QEvent::LayoutHint )
    setFixedSize(minimumSize()); // because K/QMainWindow
                                 // does not manage fixed central widget
                                 // with hidden menubar...
  return KMainWindow::eventFilter(o, e);
}

void KZoomMainWindow::setZoom(uint zoom)
{
  _zoom = zoom;
  writeZoomSetting(_zoom);
  QPtrListIterator<QWidget> it(_widgets);
  for (; it.current(); ++it)
    (*it)->adjustSize();; 
  _zoomOutAction->setEnabled( _zoom>_minZoom );
  _zoomInAction->setEnabled( _zoom<_maxZoom );
}

void KZoomMainWindow::zoomIn()
{
  setZoom(_zoom + _zoomStep);
}

void KZoomMainWindow::zoomOut()
{
  Q_ASSERT( _zoom>=_zoomStep );
  setZoom(_zoom - _zoomStep);
}

void KZoomMainWindow::toggleMenubar()
{
  if ( _menu->isChecked() ) menuBar()->show();
  else menuBar()->hide();
}

bool KZoomMainWindow::queryExit()
{
  writeMenubarVisibleSetting(_menu->isChecked());
  return KMainWindow::queryExit();
}
