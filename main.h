#ifndef MAIN_H
#define MAIN_H

#include <qptrvector.h>

#include <kmainwindow.h>
#include <kaction.h>

#include "defines.h"

class Status;

class MainWidget : public KMainWindow, public KMines
{
 Q_OBJECT
 public:
	MainWidget();

 private slots:
    void changeLevel();
	void toggleMenubar();
    void configureKeys();
    void configureSettings();
	void gameStateChanged(GameState);
    void showHighscores();

 protected:
	bool eventFilter(QObject *, QEvent *);
    void focusOutEvent(QFocusEvent *);

 private:
	QPtrVector<KRadioAction>  levelAction;
	Status                *status;
    bool                   pauseFocus;

	void readSettings();
    void settingsChanged();
};

#endif
