#ifndef FIELD_H
#define FIELD_H

#include <krandomsequence.h>

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>
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
	
	void start(const Level &lev);
	void restart(bool repaint = TRUE);
	void pause();
	void stop() { _stop = TRUE; }
	void showMines();

	const Level &level() const { return lev; }
	void changeUMark(bool um) { u_mark = um; }
	
 public slots:
	void resume();
	
 signals:
	void changeCase(uint, uint);
	void updateStatus(bool);
	void setMood(Smiley::Mood);
	void endGame(int);
	void startTimer();
	void freezeTimer();
	void putMsg(const QString &msg);
  
 protected:
	void paintEvent( QPaintEvent * );
	void mousePressEvent( QMouseEvent * );
	void mouseReleaseEvent( QMouseEvent * );
	void mouseMoveEvent( QMouseEvent * );
	
 private:
	QArray<uint> _pfield;
	Level lev;

        KRandomSequence random;
  
	bool _stop;             /* end of game ? */
	bool noGame, isPaused;
	bool first_click;
	bool u_mark;

	uint ic, jc;
	bool left_down;        /* left button pressed */
	bool mid_down;         /* mid button pressed */
  
	QPainter pt;
	QPixmap  pm_flag, pm_mine, pm_exploded, pm_error;
	QPushButton *pb;
  
	uint computeNeighbours(uint, uint) const;
	void drawCase(uint, uint);
	void uncover(uint, uint);
	void changeCaseState(uint, uint, uint);
	void createMinePixmap(QPainter &p) const;
	void pressCase(uint, uint, uint);
	void pressClearFunction(uint, uint, uint);
	void clearFunction(uint, uint);
	void uncoverCase(uint, uint);
	bool inside(uint, uint) const;
	
	uint &pfield(uint i, uint j) const;
	int xToI(int x) const;
	int yToJ(int y) const;
	int iToX(uint i) const;
	int jToY(uint j) const;
};

#endif // FIELD_H
