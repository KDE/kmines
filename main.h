#ifndef MAIN_H
#define MAIN_H

#include <qpopmenu.h>

#include <kconfig.h>
#include <kaccel.h>
#include <kkeydialog.h>
#include <kapp.h>
#include <kmenubar.h>
#include <ktmainwindow.h>

class KMinesStatus;

/** Main widget 
  * This widget contains the menu bar
  */
class MainWidget : public KTMainWindow
{
  Q_OBJECT
	
 public:
	/** Construct the KMines object
	  * create the KStatus object, the menu and initialize an easy game
	  */
	MainWidget();

 private slots:
	/** menu slot : change level
	  * initialize a new game for default level ( with the constant MODES)
	  *	or launch the custom dialog with the current value of width,
	  *	height and number of mines
	  */
	void changeLevel(int level);
    void configKeys() { KKeyDialog::configureKeys(kacc); }
	void quit();
	void toggleMenu();
	void toggleUMark();
	void menuMoved();

signals:
	/** signal : start a new game */
	void newGame(uint width, uint height, uint nb_mines);
	/** signal : restart the game */
	void restartGame();
	/** signal : ask for these numbers (used for the custom dialog) */
	void getNumbers(uint &width, uint &height, uint &nbMines);
	void UMarkChanged(bool);

 protected:
	bool eventFilter(QObject *, QEvent *e);
	
 private:
	KConfig      *kconf;
	KAccel       *kacc;

	KMenuBar     *menu;
	QPopupMenu   *popup, *options, *level;
	int          tog_id, um_id;
	
	KMinesStatus *status;
  	/* set the size of the application and of the status object */
	uint nb_w, nb_h, nb_m;

	void changedSize();
};

#endif // MAIN_H
