#ifndef MAIN_H
#define MAIN_H

#include <kmainwindow.h>
#include <kaction.h>

#include "defines.h"

class Status;

class MainWidget : public KMainWindow
{
  Q_OBJECT

 public:
	MainWidget();

 private slots:
    void easyLevel()            { changeLevel(Easy); }
	void normalLevel()          { changeLevel(Normal); }
	void expertLevel()          { changeLevel(Expert); }
	void customLevel()          { changeLevel(Custom); }
	void toggleMenubar();
    void configureKeys();
    void configureSettings();
	void gameStateChanged(GameState);
    void showHighscores(int);

 protected:
	bool eventFilter(QObject *, QEvent *);
    void focusOutEvent(QFocusEvent *);

 private:
	QMemArray<KRadioAction *>   levelAction;
	Status                  *status;
    bool                     pauseFocus;
    KSelectAction           *hs;

	void readSettings();
	void changeLevel(Level);
    void settingsChanged();
};

#endif // MAIN_H
