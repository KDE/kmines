#include <stdio.h>

#include <qpopmenu.h>

#include <kmsgbox.h>
#include <kkeyconf.h>

#include "main.h"

#include "main.moc"

KMines::KMines( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
	installEventFilter(this);
	
	status = new KStatus(this);
	status->installEventFilter(this);
	
	/* KKeyCode initialization */
	kKeys->addKey(klocale->translate("Quit"), "CTRL+Q");
	kKeys->addKey(klocale->translate("New game"), "F2");
	kKeys->addKey(klocale->translate("Pause game"), "P");
	kKeys->addKey(klocale->translate("Help"), "F1");
	kKeys->addKey(klocale->translate("Options"), "O");
	kKeys->addKey(klocale->translate("High scores"), "H");
	kKeys->addKey(klocale->translate("Close dialog"), "Return");
	kKeys->addKey(klocale->translate("Ok dialog"), "Return");
	kKeys->addKey(klocale->translate("Cancel dialog"), "Escape");

	/* connections for kmines */
	kKeys->registerWidget(K_KMINES, this);
	kKeys->connectFunction(K_KMINES, klocale->translate("Quit"), this, SLOT(quit()));
	kKeys->connectFunction(K_KMINES, klocale->translate("New game"), status, SLOT(restartGame()));
	kKeys->connectFunction(K_KMINES, klocale->translate("Pause game"), status, SLOT(pauseGame()));
	kKeys->connectFunction(K_KMINES, klocale->translate("Help"), this, SLOT(help()));
	kKeys->connectFunction(K_KMINES, klocale->translate("Options"), status, SLOT(options()));
	kKeys->connectFunction(K_KMINES, klocale->translate("High scores"), status, SLOT(showHighScores()));
	
	connect( this,   SIGNAL(restartGame()), 
			 status, SLOT(restartGame()) );
	connect( this,   SIGNAL(newGame(int, int, int)),
			 status, SLOT(newGame(int, int, int)) );
	connect( this,   SIGNAL(getNumbers(int *, int *, int *)),
			 status, SLOT(getNumbers(int *, int *, int *)) );
	connect( status, SIGNAL(quit()),
			 this, SLOT(quit()) );

	/* menu */
	popup = new QPopupMenu;
	tog_id = popup->insertItem(klocale->translate("Hide menu bar"), this, SLOT(toggleMenu()) );
	popup->insertSeparator();
	popup->insertItem(klocale->translate("New game"), status, SLOT(restartGame()) );
	popup->insertItem(klocale->translate("Pause game"), status, SLOT(pauseGame()) );
	popup->insertSeparator();
	popup->insertItem(klocale->translate("High scores"), status, SLOT(showHighScores()) );
	popup->insertSeparator();
	popup->insertItem(klocale->translate("Quit"), this, SLOT(quit()) );
  
	QPopupMenu *options = new QPopupMenu;
	options->insertItem(klocale->translate("? mark"), status, SLOT(options()) );
	options->insertItem(klocale->translate("Keys"), this, SLOT(configKeys()) );
	
	QPopupMenu *level = new QPopupMenu;
	level->insertItem(klocale->translate("Easy"));
	level->insertItem(klocale->translate("Normal"));
	level->insertItem(klocale->translate("Expert"));
	level->insertSeparator();
	level->insertItem(klocale->translate("Custom"));
	connect(level, SIGNAL(activated(int)), SLOT(change_level(int)));

	QPopupMenu *help = new QPopupMenu;
	help->insertItem(klocale->translate("Help"), this, SLOT(help()) );
	help->insertSeparator();
	help->insertItem(klocale->translate("About..."), this, SLOT(about()) );
  
	menu = new QMenuBar(this);
	menu->insertItem(klocale->translate("File"), popup );
	menu->insertItem(klocale->translate("Options"), options );
	menu->insertItem(klocale->translate("Level"), level );
	menu->insertSeparator();
	menu->insertItem(klocale->translate("Help"), help );

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
	str.sprintf(klocale->translate("%s (%s) \n\nby Nicolas HADACEK  (hadacek@kde.org)\
\n http://www.via.ecp.fr/~hadacek/KDE/kmines.html"), SNAME, SDATE);
	KMsgBox::message(0, klocale->translate("kmines : About"),
					 (const char *)str, KMsgBox::INFORMATION, klocale->translate("Close"));
}

void KMines::help()
{
	kapp->invokeHTMLHelp("", "");
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
		popup->changeItem(klocale->translate("Show menu bar"), tog_id);
		menu->hide();
	} else {
		popup->changeItem(klocale->translate("Hide menu bar"), tog_id);
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
				  mh + STATUS_H + LABEL_H + aff_h*CASE_W + 2*FRAME_W);
    status->setGeometry( 0, mh, aff_w*CASE_W + 2*FRAME_W,
						 STATUS_H + LABEL_H + aff_h*CASE_W + 2*FRAME_W );
}


/* MAIN */
int main( int argc, char ** argv )
{
    KApplication a(argc, argv, NAME);
	KMines *km = new KMines();
	km->setCaption(SNAME);

	a.setMainWidget(km);
	km->show();
	
    return a.exec();
}
