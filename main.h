/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef MAIN_H
#define MAIN_H

#include <kmainwindow.h>
#include <kaction.h>
#include "gsettings.h"

#include "defines.h"


class Status;

class MainWidget : public KMainWindow
{
 Q_OBJECT
 public:
	MainWidget();

 private slots:
	void toggleMenubar();
    void configureKeys();
    void configureSettings();
	void gameStateChanged(KMines::GameState);
    void showHighscores();
    void settingsChanged();
    void pause();

 protected:
	bool eventFilter(QObject *, QEvent *);
    void focusOutEvent(QFocusEvent *);
    bool queryExit();

 private:
	Status             *_status;
    KSettingCollection  _settings;
    KToggleAction      *_menu, *_pause;
    KSelectAction      *_levels;
    KAction            *_advise, *_solve;

    struct KeyData {
        const char *label, *name;
        Qt::Key keycode;
        const char *slot;
    };
    enum Key { NB_KEYS = 11 };
    static const KeyData KEY_DATA[NB_KEYS];

	void readSettings();
};

#endif
