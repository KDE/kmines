#include "main.h"
#include "main.moc"

#include "defines.h"
#include "version.h"
#include "status.h"
#include "dialogs.h"


MainWidget::MainWidget()
{
	installEventFilter(this);
	
	status = new KMinesStatus(this);
	status->installEventFilter(this);
	connect( this, SIGNAL(newGame(uint, uint, uint)),
			 status, SLOT(newGame(uint, uint, uint)) );
	connect( this, SIGNAL(getNumbers(uint &, uint &, uint &)),
			 status, SLOT(getNumbers(uint &, uint &, uint &)) );
	connect( status, SIGNAL(quit()), this, SLOT(quit()) );
	setView(status);
	
	kacc = new KAccel(this);
	// KAccel initialization
	kacc->insertStdItem(KAccel::New, i18n("New game"));
	kacc->insertItem(i18n("Pause game"), "Pause", "P");
	kacc->insertItem(i18n("High scores"), "HighScores", "H");
	kacc->insertStdItem(KAccel::Help);

	// KAccel connections
	kacc->connectItem(KAccel::Quit, this, SLOT(quit()));
	kacc->connectItem(KAccel::New, status, SLOT(restartGame()));
	kacc->connectItem("Pause", status, SLOT(pauseGame()));
	kacc->connectItem("HighScores", status, SLOT(showHighScores()));
	kacc->connectItem(KAccel::Print, status, SLOT(print()));
	
	// KAccel settings
	kacc->readSettings();

	// Menu
	popup = new QPopupMenu;
	int id;
	tog_id = popup->insertItem(i18n("Hide menu bar"), this, SLOT(toggleMenu()) );
	popup->insertSeparator();
	id = popup->insertItem(i18n("&New game"), status, SLOT(restartGame()) );
	kacc->changeMenuAccel(popup, id, KAccel::New);
	id = popup->insertItem(i18n("&Pause game"), status, SLOT(pauseGame()) );
	kacc->changeMenuAccel(popup, id, "Pause");
	popup->insertSeparator();
	id = popup->insertItem(i18n("&High scores"), status, SLOT(showHighScores()) );
	kacc->changeMenuAccel(popup, id, "HighScores");
	id = popup->insertItem(i18n("&Print"), status, SLOT(print()));
	kacc->changeMenuAccel(popup, id, KAccel::Print);
	popup->insertSeparator();
	id = popup->insertItem(i18n("&Quit"), this, SLOT(quit()) );
	kacc->changeMenuAccel(popup, id, KAccel::Quit);

	options = new QPopupMenu;
	options->setCheckable(TRUE);
	um_id = options->insertItem(i18n("? mark"), this, SLOT(toggleUMark()) );
	options->insertItem(i18n("Keys"), this, SLOT(configKeys()) );
	
	level = new QPopupMenu;
	level->insertItem(i18n("Easy"),   0);
	level->insertItem(i18n("Normal"), 1);
	level->insertItem(i18n("Expert"), 2);
	level->insertSeparator();
	level->insertItem(i18n("Custom"), 3);
	connect(level, SIGNAL(activated(int)), SLOT(changeLevel(int)));

	QPopupMenu *help = kapp->getHelpMenu(true, i18n("%1 %2 (%3)\n\nby %4")
										 .arg(KMINES_NAME).arg(KMINES_VERSION)
										 .arg(KMINES_DATE)
										 .arg(KMINES_AUTHOR));

	menu = new KMenuBar(this);
	connect(menu, SIGNAL(moved(menuPosition)),
		this, SLOT(menuMoved()));
	menu->insertItem(i18n("&File"), popup );
	menu->insertItem(i18n("&Level"), level );
	menu->insertItem(i18n("&Options"), options );
	menu->insertSeparator();
	menu->insertItem(i18n("&Help"), help );
	setMenu(menu);

	kconf = kapp->getConfig();
	kconf->setGroup(OP_GRP);

	// read menu visible/invisible config
	if ( !kconf->hasKey(OP_MENUBAR_VIS) )
		kconf->writeEntry(OP_MENUBAR_VIS, 1);
	if ( kconf->readNumEntry(OP_MENUBAR_VIS)!=1 ) menu->show();
	else menu->hide();
	toggleMenu();

	// read uncertain mark option
	kconf->setGroup(OP_GRP);
	if ( !kconf->hasKey(OP_UMARK_KEY) )
		kconf->writeEntry(OP_UMARK_KEY, TRUE);
	bool um = kconf->readBoolEntry(OP_UMARK_KEY);
	options->setItemChecked(um_id, um);
	emit UMarkChanged(um);
	
	// begin easy game
	changeLevel(0);
}

void MainWidget::changeLevel(int lev)
{
	for(int i = 0; i<4; i++)
		level->setItemChecked(i, i==lev);
	
	if (lev==3) {
		emit getNumbers(nb_w, nb_h, nb_m);
		Custom cu(&nb_w, &nb_h, &nb_m, this);
		if ( !cu.exec() ) return;
	} else {
		nb_w = MODES[lev][0];
		nb_h = MODES[lev][1];
		nb_m = MODES[lev][2];
	}

	changedSize();
	newGame(nb_w, nb_h, nb_m);
}

bool MainWidget::eventFilter(QObject *, QEvent *e)
{
	if ( e->type()!=QEvent::MouseButtonPress ) return FALSE;
	
	QMouseEvent *em = (QMouseEvent*)e;
	if ( em->button()!=RightButton ) return FALSE;
	
	popup->popup(QCursor::pos());
	return TRUE;
}

void MainWidget::quit()
{
	kconf->setGroup(OP_GRP);
	kconf->writeEntry(OP_MENUBAR_VIS, menu->isVisible());
	kconf->writeEntry(OP_UMARK_KEY, options->isItemChecked(um_id));
	kapp->quit();
}

void MainWidget::toggleMenu()
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

void MainWidget::toggleUMark()
{
	bool um = !options->isItemChecked(um_id);
	options->setItemChecked(um_id, um);
	emit UMarkChanged(um);
}

void MainWidget::changedSize()
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
    if ( menu->isVisible() && 
	 (menu->menuBarPos() == KMenuBar::Top ||
	  menu->menuBarPos() == KMenuBar::Bottom)) 
      mh += menu->height();
	    
    setFixedSize( aff_w*CASE_W + 2*FRAME_W,
		  mh + STAT_H + LABEL_H + aff_h*CASE_W + 2*FRAME_W);
    status->setGeometry( 0, mh, aff_w*CASE_W + 2*FRAME_W,
			 STAT_H + LABEL_H + aff_h*CASE_W + 2*FRAME_W );
}

void MainWidget::menuMoved()
{
	changedSize();
}

/* MAIN */
int main( int argc, char ** argv )
{
    KApplication a(argc, argv, KMINES_NAME);
	MainWidget *mw = new MainWidget;
    a.setMainWidget(mw);
    mw->show();
	
    return a.exec();
}
