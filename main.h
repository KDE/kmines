#ifndef MAIN_H
#define MAIN_H

#include <kmainwindow.h>
#include <kaction.h>
#include <gsettings.h>

#include "defines.h"


class Status;

class MainWidget : public KMainWindow, public KMines
{
 Q_OBJECT
 public:
	MainWidget();

 private slots:
	void toggleMenubar();
    void configureKeys();
    void configureSettings();
	void gameStateChanged(GameState);
    void showHighscores();
    void settingsChanged();

 protected:
	bool eventFilter(QObject *, QEvent *);
    void focusOutEvent(QFocusEvent *);
    bool queryExit();

 private:
	Status             *status;
    bool                pauseFocus;
    KSettingCollection  collection;
    KToggleAction      *menu, *pause;
    KSelectAction      *levels;

	void readSettings();
};

#endif
