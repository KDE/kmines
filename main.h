#ifndef MAIN_H
#define MAIN_H

#include <kkeydialog.h>
#include <ktmainwindow.h>
#include <kaction.h>

class Status;

class MainWidget : public KTMainWindow
{
  Q_OBJECT
	
 public:
	MainWidget();

 private slots:
	void easyLevel()   { changeLevel(0); }
	void normalLevel() { changeLevel(1); }
	void expertLevel() { changeLevel(2); }
	void customLevel() { changeLevel(3); }
    void configKeys()  { KKeyDialog::configureKeys(kacc); }
	void toggleMenu()  { _toggleMenu(FALSE); }
	void toggleUMark() { _toggleUMark(FALSE); }

 protected:
	bool eventFilter(QObject *, QEvent *);

 private:
	KAccel        *kacc;
	KActionMenu   *popup;
	KToggleAction *toggleMenuAction, *toggleMarkAction, *levelAction[4];
	Status        *status;
	
	void _toggleMenu(bool first);
	void _toggleUMark(bool first);
	void changeLevel(int i);
};

#endif // MAIN_H
