#include <stdio.h>

#include <qpopmenu.h>

#include <kmsgbox.h>
#include <kconfig.h>
#include <kkeyconf.h>

#include "kmines.h"

#include "kmines.moc"

KMines::KMines( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
	installEventFilter(this);
	
	status = new KStatus(this);
	status->installEventFilter(this);
	
	/* KKeyCode initialization */
	kKeys->addKey("Quit", "CTRL+Q");
	kKeys->addKey("New game", "F2");
	kKeys->addKey("Pause", "P");
	kKeys->addKey("Help", "F1");
	kKeys->addKey("Options", "O");
	kKeys->addKey("High scores", "H");
	kKeys->addKey("Close dialog", "Return");
	kKeys->addKey("Ok dialog", "Return");
	kKeys->addKey("Cancel dialog", "Escape");

	/* connections for kmines */
	kKeys->registerWidget("kmines", this);
	kKeys->connectFunction("kmines", "Quit", this, SLOT(quit()));
	kKeys->connectFunction("kmines", "New game", status, SLOT(restartGame()));
	kKeys->connectFunction("kmines", "Pause", status, SLOT(pauseGame()));
	kKeys->connectFunction("kmines", "Help", this, SLOT(help()));
	kKeys->connectFunction("kmines", "Options", status, SLOT(options()));
	kKeys->connectFunction("kmines", "High scores", 
						   status, SLOT(showHighScores()));
	
	connect( this,   SIGNAL(restartGame()), 
			 status, SLOT(restartGame()) );
	connect( this,   SIGNAL(newGame(int, int, int)),
			 status, SLOT(newGame(int, int, int)) );
	connect( this,   SIGNAL(getNumbers(int *, int *, int *)),
			 status, SLOT(getNumbers(int *, int *, int *)) );

	/* menu */
	popup = new QPopupMenu;
	tog_id = popup->insertItem( "Hide menuBar", this, SLOT(toggleMenu()) );
	popup->insertSeparator();
	popup->insertItem( "Restart game",  status, SLOT(restartGame()) );
	popup->insertItem( "Pause game",  status, SLOT(pauseGame()) );
	popup->insertSeparator();
	popup->insertItem( "High scores", status, SLOT(showHighScores()) );
	popup->insertSeparator();
	popup->insertItem( "Quit", this, SLOT(quit()) );
  
	QPopupMenu *options = new QPopupMenu;
	options->insertItem( "? mark", status, SLOT(options()) );
	options->insertItem( "Keys" , this, SLOT(configKeys()) );
	
	QPopupMenu *level = new QPopupMenu;
	level->insertItem( "Easy");
	level->insertItem( "Normal");
	level->insertItem( "Expert");
	level->insertSeparator();
	level->insertItem( "Custom");
	connect(level, SIGNAL(activated(int)), SLOT(change_level(int)));

	QPopupMenu *help = new QPopupMenu;
 	help->insertItem( "Help", this, SLOT(help()) ); 
	help->insertSeparator(); 
	help->insertItem( "About...", this, SLOT(about()) );
  
	menu = new QMenuBar(this);
	menu->insertItem( "File", popup );
	menu->insertItem( "Options", options );
	menu->insertItem( "Level", level );
	menu->insertSeparator();
	menu->insertItem( "Help", help );

	/* read the menu visible/invisible config */
	kconf = kapp->getConfig();
	kconf->setGroup("");
	if ( !kconf->hasKey(OP_MENUBAR_VIS) )
		kconf->writeEntry(OP_MENUBAR_VIS, 1);
	if ( kconf->readNumEntry(OP_MENUBAR_VIS)!=1 ) menu->show();
	else menu->hide();
	
	/* begin easy game */
	change_level(0);

	toggleMenu();
}

void KMines::about()
{   QString str;
	str.sprintf( "%s (%s) \n\nby Nicolas HADACEK  (hadacek@kde.org)\
\n http://www.via.ecp.fr/~hadacek/KDE/kmines.html",
			     SNAME, SDATE );
	KMsgBox::message( 0, kapp->getCaption(), (const char *)str,
					  KMsgBox::INFORMATION, "Close" );
}

void KMines::help()
{
	kapp->invokeHTMLHelp("","");
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

void KMines::configKeys()
{
	kKeys->configureKeys(this);
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
	kapp->quit();
}

void KMines::toggleMenu()
{
	if ( menu->isVisible() ) {
		popup->changeItem("Show menuBar", tog_id);
		menu->hide();
	} else {
		popup->changeItem("Hide menuBar", tog_id);
		menu->show();
	}
	
	changedSize();
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
				  mh + STATUS_H + aff_h*CASE_W + 2*FRAME_W );
    status->setGeometry( 0, mh, aff_w*CASE_W + 2*FRAME_W,
						 STATUS_H + aff_h*CASE_W + 2*FRAME_W );
}


/* MAIN */
int main( int argc, char ** argv )
{
    KApplication a(argc, argv, "kmines");
	KMines *km = new KMines();
	km->setCaption( a.getCaption() );

	a.setMainWidget(km);
	km->show();
	
    return a.exec();
}
