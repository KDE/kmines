#ifndef MAIN_H
#define MAIN_H

#include <qpopmenu.h>

#include <kkeydialog.h>
#include <ktmainwindow.h>

class Status;

class MainWidget : public KTMainWindow
{
  Q_OBJECT
	
 public:
	MainWidget();

 private slots:
	void changeLevel(int level);
    void configKeys()           { KKeyDialog::configureKeys(kacc); }
	void toggleMenu()           { _toggleMenu(FALSE); }
	void toggleUMark()          { _toggleUMark(FALSE); }

 protected:
	bool eventFilter(QObject *, QEvent *);

 private:
	KAccel       *kacc;
	QPopupMenu   *popup, *options, *level;
	int          tog_id, um_id;
	Status       *status;
	
	void _toggleMenu(bool first);
	void _toggleUMark(bool first);
};

#endif // MAIN_H
