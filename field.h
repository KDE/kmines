#ifndef FIELD_H
#define FIELD_H

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <krandomsequence.h>

#include "defines.h"
#include "dialogs.h"


//-----------------------------------------------------------------------------
class Field : public QFrame, public KMines
{
 Q_OBJECT
 public:
	Field(QWidget *parent);

	QSize sizeHint() const;
	QSizePolicy sizePolicy() const;

	void restart();
	bool isPaused() const { return state==Paused; }
	void pause();
	void stop() { state = Stopped; }
	void showMines();

	void up();
	void down();
	void left();
	void right();
	void reveal();
	void mark();
	void umark();
	void keyboardAutoReveal();

	void setLevel(const Level &);
	const Level &level() const { return _level; }
	void readSettings();
	void setCaseSize(uint s);
	uint caseSize() const { return cp.size; }
	uint nbActions() const { return nb_actions; }

 public slots:
	void resume();

 signals:
	void changeCase(CaseState, int);
	void updateStatus(bool);
	void setMood(Smiley::Mood);
	void endGame();
	void startTimer();
	void freezeTimer();
	void gameStateChanged(GameState);

 protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

 private slots:
	void keyboardAutoRevealSlot();

 private:
	QMemArray<Case> _pfield;
	Level           _level;
	KRandomSequence random;

	GameState state;
	uint      nb_actions;
	bool      u_mark, cursor, firstReveal;

	uint  ic, jc;              // current pos
	MouseAction mb[3];         // mouse bindings
	MouseAction currentAction;

	CaseProperties cp;
	QPixmap        pm_flag, pm_mine, pm_exploded, pm_error;
	QPushButton    button;

	uint computeNeighbours(uint, uint) const;
	void uncover(uint, uint);
	void changeCaseState(uint, uint, CaseState);
	void minePixmap(QPixmap &, bool mask, CaseState) const;
	void pressCase(uint, uint, bool);
	void pressClearFunction(uint, uint, bool);
	void uncoverCase(uint, uint);
	bool inside(int, int) const;
	bool placeCursor(int, int);
	void flagPixmap(QPixmap &, bool mask) const;
	void autoReveal();
	void _endGame();
	bool revealActions(bool press);

	const Case &pfield(uint i, uint j) const;
	Case &pfield(uint i, uint j);
	int xToI(int x) const;
	int yToJ(int y) const;
	int iToX(uint i) const;
	int jToY(uint j) const;

	void drawCase(QPainter &, uint, uint);
	void drawBox(QPainter &, uint, uint, bool, const QPixmap * = 0,
                 const QString &text = QString::null,
                 const QColor &textColor = black);
	void eraseField();

	void setUMark(bool um) { u_mark = um; }
	void setCaseProperties(const CaseProperties &);
	void setCursor(bool show);
	MouseAction mapMouseButton(QMouseEvent *) const;
};

#endif // FIELD_H
