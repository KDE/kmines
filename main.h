#ifndef MAIN_H
#define MAIN_H

#include <qpopmenu.h>

#include <kconfig.h>
#include <kaccel.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <ktmainwindow.h>

#include "status.h"

class MainWidget : public KTMainWindow
{
  Q_OBJECT
	
 public:
	MainWidget();

 private slots:
	/** menu slot : change level
	  * initialize a new game for default level ( with the constant MODES)
	  *	or launch the custom dialog with the current value of width,
	  *	height and number of mines
	  */
	void changeLevel(int level);
    void configKeys() { KKeyDialog::configureKeys(kacc); }
	void toggleMenu();
	void toggleUMark();
	void quit();

signals:
	void newGame(uint width, uint height, uint nb_mines);
	void restartGame();
	void UMarkChanged(bool);

 protected:
	bool eventFilter(QObject *, QEvent *e);

 private:
	KConfig      *kconf;
	KAccel       *kacc;
	KMenuBar     *menu;
	QPopupMenu   *popup, *options, *level;
	int          tog_id, um_id;
	Status       *status;
};

#endif // MAIN_H
