#include "main.h"

#include <time.h>
#include <qdatetime.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmenubar.h>
#include <kaboutdialog.h>

#include "defines.h"
#include "version.h"
#include "status.h"

MainWidget::MainWidget()
{
	installEventFilter(this);
	
	status = new Status(this);
	status->installEventFilter(this);
	setView(status);
	
	kacc = new KAccel(this);
	// KAccel initialization
	kacc->insertStdItem(KAccel::New, i18n("New game"));
	kacc->insertItem(i18n("Pause game"), "Pause", "P");
	kacc->insertItem(i18n("High scores"), "HighScores", "H");
	kacc->insertStdItem(KAccel::Help);

	// KAccel connections
	kacc->connectItem(KAccel::Quit, qApp, SLOT(quit()));
	kacc->connectItem(KAccel::New, status, SLOT(restartGame()));
	kacc->connectItem("Pause", status, SLOT(pauseGame()));
	kacc->connectItem("HighScores", status, SLOT(showHighScores()));
	kacc->connectItem(KAccel::Print, status, SLOT(print()));
	
	// KAccel settings
	kacc->readSettings();

	// Menu
	popup = new QPopupMenu;
	int id;
	tog_id = popup->insertItem("", this, SLOT(toggleMenu()) );
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
	id = popup->insertItem(i18n("&Quit"), qApp, SLOT(quit()) );
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

	menuBar()->insertItem(i18n("&File"), popup);
	menuBar()->insertItem(i18n("&Level"), level);
	menuBar()->insertItem(i18n("&Options"), options);
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), helpMenu());

	helpMenu()->connectItem(helpMenu()->idAt(2), this, SLOT(about())); // HACK
	
	_toggleMenu(TRUE);
	_toggleUMark(TRUE);
	changeLevel(-1);
}

void MainWidget::changeLevel(int lev)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	if ( lev==-1 ) {
		lev = conf->readUnsignedNumEntry(OP_LEVEL, 0);
		if ( lev<0 || lev>2 ) lev = 0;
	}
	conf->writeEntry(OP_LEVEL, lev);
	if ( !status->newGame(lev) ) return;
	for(int i = 0; i<4; i++) level->setItemChecked(i, i==lev);
}

bool MainWidget::eventFilter(QObject *, QEvent *e)
{
	switch (e->type()) {
	 case QEvent::MouseButtonPress : 
		if ( ((QMouseEvent *)e)->button()!=RightButton ) return FALSE;
		popup->popup(QCursor::pos());
		return TRUE;
	 default : return FALSE;
	}
}

void MainWidget::_toggleMenu(bool first)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	bool show = conf->readBoolEntry(OP_MENU, TRUE);
	if ( !first ) show = !show;
	conf->writeEntry(OP_MENU, show);
	if (show) menuBar()->show(); else menuBar()->hide();
	popup->changeItem((show ? i18n("Hide menu") : i18n("Show menu")), tog_id);
}

void MainWidget::_toggleUMark(bool first)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	bool um = conf->readBoolEntry(OP_UMARK, TRUE);
	if ( !first ) um = !um;
	conf->writeEntry(OP_UMARK, um);
	options->setItemChecked(um_id, um);
	status->changeUMark(um);
}

void MainWidget::about()
{
	QDate date(YEAR, MONTH, DAY);
	QString sdate = KGlobal::locale()->formatDate(date);

	KAboutDialog about(KAboutDialog::AbtAppStandard, kapp->name(),
					   KDialogBase::Close, KDialogBase::Close, this, 0, true);

	about.setProduct("KMines", QString("%2 (%3)").arg(VERSION).arg(sdate),
					 "Nicolas Hadacek (hadacek@kde.org)", "1996-1999");

	QString sa = i18n(
		"KMines is a classical mine sweeper game.");
	about.addTextPage(i18n("About"), sa);

	QString sl = i18n(
	   "This program is free software; you can redistribute it and/or modify\n"
  	   "it under the terms of the GNU General Public License as published by\n"
	   "the Free Software Foundation; either version 2 of the License, or\n"
	   "(at your option) any later version.");
	about.addTextPage(i18n("Licence"), sl);

	about.show();
}

//----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    KApplication a(argc, argv, "kmines");
	MainWidget *mw = new MainWidget;
    a.setMainWidget(mw);
    mw->show();
    return a.exec();
}
