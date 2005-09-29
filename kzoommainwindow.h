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

#ifndef KZOOMMAINWINDOW_H
#define KZOOMMAINWINDOW_H

#include <kmainwindow.h>

class KToggleAction;

/**
 * KZoomMainWindow is a main window of fixed size. Its size can be
 * modified with the "zoom in"/"zoom out" actions.
 *
 * It manages one or several widgets: their adjustSize() method is
 * called whenever the zoom level is changed.
 * The usual implementation for those widget is to redefine adjustSize()
 * with code like:
 * /code
 * setFixedSize(newsize);
 * /endcode
 *
 * This class also has a "show/hide menubar" action and allows the use
 * of a context popup menu (useful to restore the menubar when hidden).
 */
class KZoomMainWindow : public KMainWindow
{
  Q_OBJECT
public:
  /** Constructor. */
  KZoomMainWindow(uint minZoom, uint maxZoom, uint zoomStep,
                  const char *name = 0);

  /** Add a widget to be managed i.e. the adjustSize() method of the
   * widget is called whenever the zoom is changed.
   * This function assumes that the topLevelWidget() is the KZoomMainWindow.
   */
  static void addWidget(QWidget *widget);
                  
  uint zoom() const { return _zoom; }
  
public slots:
  void zoomIn();
  void zoomOut();
  void toggleMenubar();

protected:
  /** You need to call this after the createGUI or setupGUI method
   * is called.
   * @param popupName is the name of the context popup menu as defined in
   * the ui.rc file.
   */
  void init(const char *popupName = 0);
    
  virtual void setZoom(uint zoom);
  virtual bool eventFilter(QObject *o, QEvent *e);
  virtual bool queryExit();
  
  /** You need to implement this method since different application
   * use different setting class names and keys.
   * Use something like:
   * /code
   * Settings::setZoom(zoom);
   * Settings::writeConfig();
   * /endcode
   */
  virtual void writeZoomSetting(uint zoom) = 0;
  
  /** Youneed to implement this method since different application
   * use different setting class names and keys.
   * Use something like:
   * /code
   * return Settings::zoom();
   * /endcode
   */
  virtual uint readZoomSetting() const = 0;
  
  /** You need to implement this method since different application
   * use different setting class names and keys.
   * Use something like:
   * /code
   * Settings::setMenubarVisible(visible);
   * Settings::writeConfig();
   * /endcode
   */
  virtual void writeMenubarVisibleSetting(bool visible) = 0;
  
  /** You need to implement this method since different application
   * use different setting class names and keys.
   * Use something like: 
   * /code
   * Settings::menubarVisible();
   * /endcode
   */
  virtual bool menubarVisibleSetting() const = 0;

private slots:
  void widgetDestroyed();
  
private:
  uint _zoom, _zoomStep, _minZoom, _maxZoom;
  QPtrList<QWidget> _widgets;
  KAction *_zoomInAction, *_zoomOutAction;
  KToggleAction *_menu;
  
  class KZoomMainWindowPrivate;
  KZoomMainWindowPrivate *d;
};

#endif
