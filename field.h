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

/* mines field widget */
class Field : public QFrame
{
  Q_OBJECT
	
 public:
    Field(QWidget *parent, const char *name=0);

	QSize sizeHint() const;
	QSizePolicy sizePolicy() const;
	
	void restart(bool repaint = true);
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
	const Level &level() const { return lev; }
	void readSettings();
	
 public slots:
	void resume();
	
 signals:
	void changeCase(CaseState, uint);
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
	QArray<Case>    _pfield;
	Level           lev;
	KRandomSequence random;

	GameState state;
	bool      first_click;
	bool      u_mark, cursor;

	int  ic, jc;               // current pos
	bool _reveal, _autoreveal; // mouse button pressed
	MouseAction mb[3];         // mouse bindings

	CaseProperties cp;
	QPixmap        pm_flag, pm_mine, pm_exploded, pm_error, pm_cursor;
	QPushButton   *pb;

	uint computeNeighbours(uint, uint) const;
	void uncover(uint, uint);
	void changeCaseState(uint, uint, CaseState);
	void minePixmap(QPixmap &, bool mask, CaseState) const;
	void pressCase(uint, uint, bool, QPainter * = 0);
	void pressClearFunction(uint, uint, bool);
	void uncoverCase(uint, uint);
	bool inside(int, int) const;
	bool placeCursor(int, int, bool check = false);
	void flagPixmap(QPixmap &, bool mask) const;
	void cursorPixmap(QPixmap &, bool mask) const;
	void autoReveal();
	void _endGame();

	const Case &pfield(uint i, uint j) const;
	Case &pfield(uint i, uint j);
	int xToI(int x) const;
	int yToJ(int y) const;
	int iToX(uint i) const;
	int jToY(uint j) const;

	QPainter *begin(QPainter *);
	void end(QPainter *, const QPainter *);
	void drawCase(uint, uint, QPainter * = 0);
	void drawBox(int, int, bool, QPainter * = 0);
	void eraseField();
	void drawCursor(bool show, QPainter * = 0);

	void setUMark(bool um) { u_mark = um; }
	void setCaseProperties(const CaseProperties &);
	void setCursor(bool show);
	MouseAction mapMouseButton(QMouseEvent *e) const;
};

#endif // FIELD_H
