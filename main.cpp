#include <stdio.h>

#include <qpopmenu.h>

#include <kmsgbox.h>
#include <kkeyconf.h>

#include "main.h"
#include "version.h"

#include "main.moc"

KMines::KMines(QWidget *parent, const char *name)
: QWidget( parent, name )
{
   	setCaption(kapp->getCaption());
	installEventFilter(this);
	
	status = new KStatus(this);
	status->installEventFilter(this);
	
	/* KKeyCode initialization */
	kKeys->addKey(i18n("Quit"), "CTRL+Q");
	kKeys->addKey(i18n("New game"), "F2");
	kKeys->addKey(i18n("Pause game"), "P");
	kKeys->addKey(i18n("High scores"), "H");
	kKeys->addKey(i18n("Close dialog"), "Return");
	kKeys->addKey(i18n("Ok dialog"), "Return");
	kKeys->addKey(i18n("Cancel dialog"), "Escape");

	/* connections for kmines */
	kKeys->registerWidget(K_KMINES, this);
	kKeys->connectFunction(K_KMINES, i18n("Quit"), this, SLOT(quit()));
	kKeys->connectFunction(K_KMINES, i18n("New game"), status, SLOT(restartGame()));
	kKeys->connectFunction(K_KMINES, i18n("Pause game"), status, SLOT(pauseGame()));
	kKeys->connectFunction(K_KMINES, i18n("High scores"), status, SLOT(showHighScores()));
	
	connect( this, SIGNAL(restartGame()), status, SLOT(restartGame()) );
	connect( this, SIGNAL(newGame(uint, uint, uint)),
			 status, SLOT(newGame(uint, uint, uint)) );
	connect( this, SIGNAL(getNumbers(uint *, uint *, uint *)),
			 status, SLOT(getNumbers(uint *, uint *, uint *)) );
	connect( status, SIGNAL(quit()), this, SLOT(quit()) );

	/* menu */
	popup = new QPopupMenu;
	tog_id = popup->insertItem(i18n("Hide menu bar"),
							   this, SLOT(toggleMenu()) );
	popup->insertSeparator();
	popup->insertItem(i18n("New game"), status, SLOT(restartGame()) );
	popup->insertItem(i18n("Pause game"), status, SLOT(pauseGame()) );
	popup->insertSeparator();
	popup->insertItem(i18n("High scores"), status, SLOT(showHighScores()) );
	popup->insertSeparator();
	popup->insertItem(i18n("Quit"), this, SLOT(quit()) );
  
	options = new QPopupMenu;
	options->setCheckable(TRUE);
	um_id = options->insertItem(i18n("? mark"), this, SLOT(toggleUMark()) );
	options->insertItem(i18n("Keys"), this, SLOT(configKeys()) );
	
	QPopupMenu *level = new QPopupMenu;
	level->insertItem(i18n("Easy"));
	level->insertItem(i18n("Normal"));
	level->insertItem(i18n("Expert"));
	level->insertSeparator();
	level->insertItem(i18n("Custom"));
	connect(level, SIGNAL(activated(int)), SLOT(change_level(int)));

	QPopupMenu *help = kapp->getHelpMenu(true, QString(i18n("Minesweeper"))
                                         + " " + KMINES_VERSION 
										 + " (" + KMINES_DATE + ")"
                                         + i18n("\n\nby Nicolas Hadacek")
                                         + " (hadacek@kde.org)");  

	menu = new QMenuBar(this);
	menu->insertItem(i18n("File"), popup );
	menu->insertItem(i18n("Options"), options );
	menu->insertItem(i18n("Level"), level );
	menu->insertSeparator();
	menu->insertItem(i18n("Help"), help );

	/* read the menu visible/invisible config */
	kconf = kapp->getConfig();
	kconf->setGroup("");
	if ( !kconf->hasKey(OP_MENUBAR_VIS) )
		kconf->writeEntry(OP_MENUBAR_VIS, 1);
	if ( kconf->readNumEntry(OP_MENUBAR_VIS)!=1 ) menu->show();
	else menu->hide();

	/* read uncertain mark option */
	kconf->setGroup(OP_GRP);
	if ( !kconf->hasKey(OP_UMARK_KEY) )
		kconf->writeEntry(OP_UMARK_KEY, TRUE);
	bool um = kconf->readBoolEntry(OP_UMARK_KEY);
	options->setItemChecked(um_id, um);
	emit UMarkChanged(um);
	
	/* begin easy game */
	change_level(0);

	toggleMenu();
}

void KMines::change_level(int lev)
{
	bool go;
	
	if (lev==4) {
		emit getNumbers(&nb_w, &nb_h, &nb_m);
		
		Custom cu(&nb_w, &nb_h, &nb_m, this);
		go = cu.exec();
	} else {
		nb_w = MODES[lev][0]; 
		nb_h = MODES[lev][1];
		nb_m = MODES[lev][2];
		go = TRUE;
	}

	if ( go ) {
		changedSize();
		newGame(nb_w, nb_h, nb_m); 
	}
}

bool KMines::eventFilter(QObject *, QEvent *e)
{
	if ( e->type()!=Event_MouseButtonPress ) return FALSE;
	
	QMouseEvent *em = (QMouseEvent*) e;
	if ( em->button()!=RightButton ) return FALSE;
	
	popup->popup(QCursor::pos());
	return TRUE;
}

void KMines::quit()
{
	kconf->setGroup("");
	kconf->writeEntry(OP_MENUBAR_VIS, menu->isVisible());
	kconf->setGroup(OP_GRP);
	kconf->writeEntry(OP_UMARK_KEY, options->isItemChecked(um_id));
	kapp->quit();
}

void KMines::toggleMenu()
{
	if ( menu->isVisible() ) {
		popup->changeItem(i18n("Show menu bar"), tog_id);
		menu->hide();
	} else {
		popup->changeItem(i18n("Hide menu bar"), tog_id);
		menu->show();
	}
	
	changedSize();
}

void KMines::toggleUMark()
{
	bool um = !options->isItemChecked(um_id);
	options->setItemChecked(um_id, um);
	emit UMarkChanged(um);
}

void KMines::changedSize()
{
	int aff_w, aff_h;
	
	/* if under the minimum size for application */
	aff_h = nb_h;
	if (nb_w < MIN_W) {
		aff_w = MIN_W;
		aff_h = nb_h + (MIN_W - nb_w);
	} else 
		aff_w = nb_w;
	
	int mh = 0;
    if ( menu->isVisible() ) mh += menu->height();
	
	setFixedSize( aff_w*CASE_W + 2*FRAME_W,
				  mh + STATUS_H + LABEL_H + aff_h*CASE_W + 2*FRAME_W);
    status->setGeometry( 0, mh, aff_w*CASE_W + 2*FRAME_W,
						 STATUS_H + LABEL_H + aff_h*CASE_W + 2*FRAME_W );
}


/* MAIN */
int main( int argc, char ** argv )
{
    KApplication a(argc, argv, NAME);
	KMines *km = new KMines();

	a.setMainWidget(km);
	km->show();
	
    return a.exec();
}
