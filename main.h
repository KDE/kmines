#ifndef MAIN_H
#define MAIN_H

#include "status.h"

#include <kconfig.h>
#include <kapp.h>

#include <qmenubar.h>
#include <qpopmenu.h>

/*/ Main widget */
/** This widget contains the menu bar */
class KMines : public QWidget
{
  Q_OBJECT
	
 public:
	/*/ Construct the KMines object */
	/** create the KStatus object, the menu and initialize an easy game */
	KMines( QWidget *parent=0, const char *name=0 );
  
 private slots:
	/*/ menu slot : change level */
	/** initialize a new game for default level ( with the constant MODES)
	  	or launch the custom dialog with the current value of width,
	  	height and number of mines */
	void change_level(int level);
	
	void about();
	void help();
	void configKeys();
	void quit();
	void toggleMenu();

 signals:
	/*/ signal : start a new game */
	void newGame(int width, int height, int nb_mines);
	/*/ signal : restart the game */
	void restartGame();
	/*/ signal : ask for these numbers (used for the custom dialog */
	void getNumbers(int *width, int *height, int *nb_mines);

 protected:
	bool eventFilter(QObject *, QEvent *e);
	
 private:
	KConfig *kconf;
	
	QMenuBar *menu;
	QPopupMenu* popup;
	int tog_id;

	/*/ status field object */
	KStatus   *status;
  
	/*/ set the size of the application and of the status object */
	int nb_w, nb_h, nb_m;
	void changedSize();
};

#endif // MAIN_H
