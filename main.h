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
	void setKeyboardEnabled(bool);
	void gameStateChanged(GameState);

 protected:
	bool eventFilter(QObject *, QEvent *);

 private:
	QArray<KRadioAction *>   levelAction;
	Status                  *status;

	void readSettings();
	void changeLevel(Level);
};

#endif // MAIN_H
