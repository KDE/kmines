#ifndef MAIN_H
#define MAIN_H

#include <ktmainwindow.h>
#include <kaction.h>

#include "defines.h"

class Status;

class MainWidget : public KTMainWindow
{
  Q_OBJECT
	
 public:
	MainWidget();

 private slots:
    void easyLevel()      { changeLevel(0); }
	void normalLevel()    { changeLevel(1); }
	void expertLevel()    { changeLevel(2); }
	void customLevel()    { changeLevel(3); }
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
	void changeLevel(uint i);
};

#endif // MAIN_H
