/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "status.h"
#include "status.moc"

#include <qpainter.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qtextedit.h>
#include <qtimer.h>

#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <knotifyclient.h>
#include <kexthighscore.h>

#include "settings.h"
#include "solver/solver.h"
#include "dialogs.h"
#include "version.h"


Status::Status(QWidget *parent)
    : QWidget(parent, "status"), _oldLevel(Level::Easy)
{
    _timer  = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), SLOT(replayStep()));

    _solver = new Solver(this);
    connect(_solver, SIGNAL(solvingDone(bool)), SLOT(solvingDone(bool)));

// top layout
	QGridLayout *top = new QGridLayout(this, 2, 5, 10, 10);
    top->setResizeMode(QLayout::Fixed);
    top->setColStretch(1, 1);
    top->setColStretch(3, 1);

// status bar
	// mines left LCD
	left = new KGameLCD(5, this);
    left->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    left->setDefaultBackgroundColor(black);
    left->setDefaultColor(white);
	QWhatsThis::add(left, i18n("<qt>Mines left.<br/>"
                               "It turns <font color=\"red\">red</font> "
                               "when you have flagged more cases than "
                               "present mines.</qt>"));
    top->addWidget(left, 0, 0);

	// smiley
	smiley = new Smiley(this);
	connect(smiley, SIGNAL(clicked()), SLOT(smileyClicked()));
	smiley->setFocusPolicy(QWidget::NoFocus);
	QWhatsThis::add(smiley, i18n("Press to start a new game"));
    top->addWidget(smiley, 0, 2);

	// digital clock LCD
	dg = new DigitalClock(this);
	QWhatsThis::add(dg, i18n("<qt>Time elapsed.<br/>"
                             "It turns <font color=\"blue\">blue</font> "
                             "if it is a highscore "
                             "and <font color=\"red\">red</font> "
                             "if it is the best time.</qt>"));
    top->addWidget(dg, 0, 4);

// mines field
    _fieldContainer = new QWidget(this);
    QGridLayout *g = new QGridLayout(_fieldContainer, 1, 1);
    field = new Field(_fieldContainer);
    field->readSettings();
    g->addWidget(field, 0, 0, AlignCenter);
	connect( field, SIGNAL(updateStatus(bool)), SLOT(update(bool)) );
	connect(field, SIGNAL(gameStateChanged(GameState)),
			SLOT(gameStateChangedSlot(GameState)) );
    connect(field, SIGNAL(setMood(Mood)), smiley, SLOT(setMood(Mood)));
    connect(field, SIGNAL(setCheating()), dg, SLOT(setCheating()));
    connect(field,SIGNAL(addAction(const KGrid2D::Coord &, Field::ActionType)),
            SLOT(addAction(const KGrid2D::Coord &, Field::ActionType)));
	QWhatsThis::add(field, i18n("Mines field."));

// resume button
    _resumeContainer = new QWidget(this);
    g = new QGridLayout(_resumeContainer, 1, 1);
    QFont f = font();
    f.setBold(true);
    QPushButton *pb
        = new QPushButton(i18n("Press to Resume"), _resumeContainer);
    pb->setFont(f);
    connect(pb, SIGNAL(clicked()), SIGNAL(pause()));
    g->addWidget(pb, 0, 0, AlignCenter);

    _stack = new QWidgetStack(this);
    _stack->addWidget(_fieldContainer);
    _stack->addWidget(_resumeContainer);
    _stack->raiseWidget(_fieldContainer);
    top->addMultiCellWidget(_stack, 1, 1, 0, 4);
}

void Status::smileyClicked()
{
    if ( field->gameState()==Paused ) emit pause();
    else restartGame();
}

void Status::newGame(int t)
{
    if ( field->gameState()==Paused ) emit pause();
    Level::Type type = (Level::Type)t;
    Settings::setLevel(type);
    if ( type!=Level::Custom ) newGame( Level(type) );
    else newGame( Settings::customLevel() );
}

void Status::newGame(const Level &level)
{
    _timer->stop();
    if ( level.type()!=Level::Custom )
        KExtHighscore::setGameType(level.type());
    field->setLevel(level);
}

bool Status::checkBlackMark()
{
    bool bm = ( field->gameState()==Playing );
    if (bm) KExtHighscore::submitScore(KExtHighscore::Lost, this);
    return bm;
}

void Status::restartGame()
{
    if ( field->gameState()==Paused ) emit pause();
    else if ( field->gameState()==Replaying ) {
        _timer->stop();
        field->setLevel(_oldLevel);
    } else {
        bool bm = checkBlackMark();
        field->reset(bm);
    }
}

void Status::settingsChanged()
{
    field->readSettings();

    if ( Settings::level()!=Level::Custom ) return;
    Level l = Settings::customLevel();
    if ( l==field->level() ) return;
    if ( field->gameState()==Paused ) emit pause();
    field->setLevel(l);
}

void Status::update(bool mine)
{
	int r = field->nbMines() - field->nbMarked();
    QColor color = (r<0 && !field->isSolved() ? red : white);
    left->setColor(color);
	left->display(r);

	if ( field->isSolved() && !mine )
        gameStateChanged(GameOver, true); // ends only for wins
}

void Status::setGameOver(bool won)
{
    field->showAllMines(won);
    smiley->setMood(won ? Happy : Sad);
    if ( field->gameState()==Replaying ) return;

    field->setGameOver();
    dg->stop();
    if ( field->level().type()!=Level::Custom && !dg->cheating() ) {
        if (won) KExtHighscore::submitScore(dg->score(), this);
        else KExtHighscore::submitScore(KExtHighscore::Lost, this);
    }

    KNotifyClient::event(winId(), won ? "won" : "lost",
                         won ? i18n("Game won!") : i18n("Game lost!"));

    // game log
    _logRoot.setAttribute("count", dg->nbActions());

    if ( Settings::magicReveal() )
        _logRoot.setAttribute("complete_reveal", "true");
    QString sa = "none";
    if ( field->solvingState()==Solved ) sa = "solving";
    else if ( field->solvingState()==Advised ) sa = "advising";
    _logRoot.setAttribute("solver", sa);

    QDomElement f = _log.createElement("Field");
    _logRoot.appendChild(f);
    QDomText data = _log.createTextNode(field->string());
    f.appendChild(data);
}

void Status::setStopped()
{
    smiley->setMood(Normal);
    update(false);
    bool custom = ( field->level().type()==Level::Custom );
    dg->reset(custom);
    field->setSolvingState(Regular);
}

void Status::setPlaying()
{
    smiley->setMood(Normal);
    dg->start();
    if ( field->gameState()==Paused ) return; // do not restart log on resume...

    const Level &level = field->level();
    _log = QDomDocument("kmineslog");
    _logRoot = _log.createElement("kmineslog");
    _logRoot.setAttribute("version", VERSION);
    QDateTime date = QDateTime::currentDateTime();
    _logRoot.setAttribute("date", date.toString(Qt::ISODate));
    _logRoot.setAttribute("width", level.width());
    _logRoot.setAttribute("height", level.height());
    _logRoot.setAttribute("mines", level.nbMines());
    _log.appendChild(_logRoot);
    _logList = _log.createElement("ActionList");
    _logRoot.appendChild(_logList);
}

void Status::gameStateChanged(GameState state, bool won)
{
    QWidget *w = _fieldContainer;

    switch (state) {
    case Playing:
        setPlaying();
        break;
    case GameOver:
        setGameOver(won);
        break;
    case Paused:
        smiley->setMood(Sleeping);
        dg->stop();
        w = _resumeContainer;
        break;
    case Stopped:
    case Init:
        setStopped();
        break;
    case Replaying:
        smiley->setMood(Normal);
        break;
    case NB_STATES:
        Q_ASSERT(false);
        break;
    }

    _stack->raiseWidget(w);
    emit gameStateChangedSignal(state);
}

void Status::addAction(const KGrid2D::Coord &c, Field::ActionType type)
{
    QDomElement action = _log.createElement("Action");
    action.setAttribute("time", dg->pretty());
    action.setAttribute("column", c.first);
    action.setAttribute("line", c.second);
    action.setAttribute("type", Field::ACTION_NAMES[type]);
    _logList.appendChild(action);
    dg->addAction();
}

void Status::advise()
{
    int res = KMessageBox::warningContinueCancel(this,
               i18n("When the solver gives "
               "you advice, your score will not be added to the highscores."),
                QString::null, QString::null, "advice_warning");
    if ( res==KMessageBox::Cancel ) return;
    dg->setCheating();
    float probability;
    KGrid2D::Coord c = _solver->advise(*field, probability);
    field->setAdvised(c, probability);
}

void Status::solve()
{
    dg->setCheating();
    _solver->solve(*field, false);
    field->setSolvingState(Solved);
}

void Status::solvingDone(bool success)
{
    if ( !success ) gameStateChanged(GameOver, false);
}

void Status::solveRate()
{
    SolvingRateDialog sd(*field, this);
    sd.exec();
}

void Status::viewLog()
{
    KDialogBase d(this, "view_log", true, i18n("View Game Log"),
                  KDialogBase::Close, KDialogBase::Close);
    QTextEdit *view = new QTextEdit(&d);
    view->setReadOnly(true);
    view->setTextFormat(PlainText);
    view->setText(_log.toString());
    d.setMainWidget(view);
    d.resize(500, 400);
    d.exec();
}

void Status::saveLog()
{
    KURL url = KFileDialog::getSaveURL(QString::null, QString::null, this);
    if ( url.isEmpty() ) return;
    if ( KIO::NetAccess::exists(url, false, this) ) {
        KGuiItem gi = KStdGuiItem::save();
        gi.setText(i18n("Overwrite"));
        int res = KMessageBox::warningYesNo(this,
                                 i18n("The file already exists. Overwrite?"),
                                 i18n("File Exists"), gi, KStdGuiItem::cancel());
        if ( res==KMessageBox::No ) return;
    }
    KTempFile tmp;
    (*tmp.textStream()) << _log.toString();
    tmp.close();
    KIO::NetAccess::upload(tmp.name(), url, this);
    tmp.unlink();
}

void Status::loadLog()
{
    KURL url = KFileDialog::getOpenURL(QString::null, QString::null, this);
    if ( url.isEmpty() ) return;
    QString tmpFile;
    bool success = false;
    QDomDocument doc;
    if( KIO::NetAccess::download(url, tmpFile, this) ) {
        QFile file(tmpFile);
        if ( file.open(IO_ReadOnly) ) {
            int errorLine;
            bool ok = doc.setContent(&file, 0, &errorLine);
            if ( !ok ) {
               KMessageBox::sorry(this, i18n("Cannot read XML file on line %1")
                                  .arg(errorLine));
               return;
            }
            success = true;
        }
        KIO::NetAccess::removeTempFile(tmpFile);

    }
    if ( !success ) {
        KMessageBox::sorry(this, i18n("Cannot load file."));
        return;
    }

    if ( !checkLog(doc) )
        KMessageBox::sorry(this, i18n("Log file not recognized."));
    else {
        _log = doc;
        _logRoot = doc.namedItem("kmineslog").toElement();
        emit gameStateChangedSignal(GameOver);
    }
}

bool Status::checkLog(const QDomDocument &doc)
{
    // check root element
    if ( doc.doctype().name()!="kmineslog" ) return false;
    QDomElement root = doc.namedItem("kmineslog").toElement();
    if ( root.isNull() ) return false;
    bool ok;
    uint w = root.attribute("width").toUInt(&ok);
    if ( !ok || w>CustomConfig::maxWidth || w<CustomConfig::minWidth )
        return false;
    uint h = root.attribute("height").toUInt(&ok);
    if ( !ok || h>CustomConfig::maxHeight || h<CustomConfig::minHeight )
        return false;
    uint nb = root.attribute("mines").toUInt(&ok);
    if ( !ok || nb==0 || nb>Level::maxNbMines(w, h) ) return false;

    // check field
    QDomElement field = root.namedItem("Field").toElement();
    if ( field.isNull() ) return false;
    QString ftext = field.text();
    if ( !BaseField::checkField(w, h, nb, ftext) ) return false;

    // check action list
    QDomElement list = root.namedItem("ActionList").toElement();
    if ( list.isNull() ) return false;
    QDomNodeList actions = list.elementsByTagName("Action");
    if ( actions.count()==0 ) return false;
    for (uint i=0; i<actions.count(); i++) {
        QDomElement a = actions.item(i).toElement();
        if ( a.isNull() ) return false;
        uint i0 = a.attribute("line").toUInt(&ok);
        if ( !ok || i0>=w ) return false;
        uint j = a.attribute("column").toUInt(&ok);
        if ( !ok || j>=h ) return false;
        QString type = a.attribute("type");
        uint k = 0;
        for (; k<Field::Nb_Actions; k++)
            if ( type==Field::ACTION_NAMES[k] ) break;
        if ( k==Field::Nb_Actions ) return false;
    }

    return true;
}


void Status::replayLog()
{
    uint w = _logRoot.attribute("width").toUInt();
    uint h = _logRoot.attribute("height").toUInt();
    uint n = _logRoot.attribute("mines").toUInt();
    Level level(w, h, n);
    QDomNode f = _logRoot.namedItem("Field");
    _oldLevel = field->level();
    newGame(level);
    field->setReplayField(f.toElement().text());
    QString s = _logRoot.attribute("complete_reveal");
    _completeReveal = ( s=="true" );

    f = _logRoot.namedItem("ActionList");
    _actions = f.toElement().elementsByTagName("Action");
    _index = 0;
    _timer->start(500);
}

void Status::replayStep()
{
    if ( _index>=_actions.count() ) {
        _timer->stop();
        _actions = QDomNodeList();
        return;
    }

    _timer->changeInterval(200);
    QDomElement a = _actions.item(_index).toElement();
    dg->setTime(a.attribute("time"));
    uint i = a.attribute("column").toUInt();
    uint j = a.attribute("line").toUInt();
    QString type = a.attribute("type");
    for (uint k=0; k<Field::Nb_Actions; k++)
        if ( type==Field::ACTION_NAMES[k] ) {
            field->doAction((Field::ActionType)k,
                            KGrid2D::Coord(i, j), _completeReveal);
            break;
        }
    _index++;
}


