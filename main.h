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
    void easyLevel()      { changeLevel(0); }
	void normalLevel()    { changeLevel(1); }
	void expertLevel()    { changeLevel(2); }
	void customLevel()    { changeLevel(3); }
    void configKeys()     { KKeyDialog::configureKeys(kacc); }
	void toggleMenubar();
    void toggleUMark();
	void toggleKeyboard();
	void saveSettings();

 protected:
	bool eventFilter(QObject *, QEvent *);

 private:
	KAccel                  *kacc;
	QArray<KRadioAction *>   levelAction;
	Status                  *status;
	
	void readSettings();
	void changeLevel(uint i);
};

#endif // MAIN_H
