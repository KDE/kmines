/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MAIN_H
#define MAIN_H

#include "kzoommainwindow.h"

#include "defines.h"

class KAction;
class KToggleAction;
class KSelectAction;
class Status;

class MainWidget : public KZoomMainWindow, public KMines
{
 Q_OBJECT
 public:
     MainWidget();

 private slots:
    void configureKeys();
    void configureSettings();
    void configureNotifications();
    void configureHighscores();
	void gameStateChanged(KMines::GameState);
    void showHighscores();
    void settingsChanged();
    void pause();

 protected:
    virtual void focusOutEvent(QFocusEvent *);
    virtual bool queryExit();

 private:
    Status            *_status;
    KToggleAction     *_pause;
    KSelectAction     *_levels;
    KAction           *_advise, *_solve;
    KActionCollection *_keybCollection;

    struct KeyData {
        const char *label, *name;
        Qt::Key keycode;
        const char *slot;
    };
    enum Key { NB_KEYS = 11 };
    static const KeyData KEY_DATA[NB_KEYS];

    void readSettings();
    virtual void writeZoomSetting(uint zoom);
    virtual uint readZoomSetting() const;
    virtual void writeMenubarVisibleSetting(bool visible);
    virtual bool menubarVisibleSetting() const;
};

#endif
