#ifndef MAIN_H
#define MAIN_H

#include <qmenubar.h>
#include <qpopmenu.h>

#include <kconfig.h>
#include <kaccel.h>
#include <kkeydialog.h>
#include <kapp.h>


class KStatus;

/** Main widget 
  * This widget contains the menu bar
  */
class KMines : public QWidget
{
  Q_OBJECT
	
 public:
	/** Construct the KMines object
	  * create the KStatus object, the menu and initialize an easy game
	  */
	KMines( QWidget *parent=0, const char *name=0 );

        virtual ~KMines ();

 private slots:
	/** menu slot : change level
	  * initialize a new game for default level ( with the constant MODES)
	  *	or launch the custom dialog with the current value of width,
	  *	height and number of mines
	  */
	void change_level(int level);
        void configKeys() { KKeyDialog::configureKeys( kacc ); }
	void quit();
	void toggleMenu();
	void toggleUMark();

 signals:
	/** signal : start a new game */
	void newGame(uint width, uint height, uint nb_mines);
	/** signal : restart the game */
	void restartGame();
	/** signal : ask for these numbers (used for the custom dialog) */
	void getNumbers(uint *width, uint *height, uint *nb_mines);
	void UMarkChanged(bool);

 protected:
	bool eventFilter(QObject *, QEvent *e);
	
 private:
	KConfig *kconf;
	KAccel * kacc;
	
	QMenuBar *menu;
	QPopupMenu* popup, *options;
	int tog_id;
	int um_id;

	/* status field object */
	KStatus   *status;
  
	/* set the size of the application and of the status object */
	uint nb_w, nb_h, nb_m;
	void changedSize();
};

#endif // MAIN_H
