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
	MainWidget(const KAboutData *);

 private slots:
    void easyLevel()      { changeLevel(0); }
	void normalLevel()    { changeLevel(1); }
	void expertLevel()    { changeLevel(2); }
	void customLevel()    { changeLevel(3); }
    void configKeys()     { KKeyDialog::configureKeys(kacc); }
	void toggleMenu();
	bool toggleUMark();
	bool toggleKeyboard();

 protected:
	bool eventFilter(QObject *, QEvent *);

 private:
	bool                     init;
	KAccel                  *kacc;
	KActionMenu             *popup;
	QArray<KAction *>        keyAction;
	QArray<KRadioAction *>   levelAction;
	KAction                 *menuAction;
	Status                  *status;
	
	bool toggle(const char *name);
	void changeLevel(uint i);
};

#endif // MAIN_H
