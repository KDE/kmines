#ifndef FIELD_H
#define FIELD_H

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>

/* mines field widget */
class Field : public QFrame
{
  Q_OBJECT
	
 public:
	Field(QWidget *parent, const char *name=0);

	QSize sizeHint() const;
	QSizePolicy sizePolicy() const;
	
	void start(uint w, uint h, uint nb);
	void restart(bool repaint = TRUE);
	void pause();
	void stop() { _stop = TRUE; }
	
	uint nbWidth() const  { return nb_w; }
	uint nbHeight() const { return nb_h; }
	uint nbMines() const  { return nb_m; }
	
 public slots:
	void resume();
	void changeUMark(bool um) { u_mark = um; };
	
 signals:
	void changeCase(uint, uint);
	void updateStatus(bool);
	void updateSmiley(int);
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
	uint nb_w, nb_h, nb_m;
  
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
	void showMines(uint, uint);
	void changeCaseState(uint, uint, uint);
	void createMinePixmap(QPainter &p) const;
	void pressCase(uint, uint, uint);
	void pressClearFunction(uint, uint, uint);
	void clearFunction(uint, uint);
	void uncoverCase(uint, uint);
	
	uint &pfield(uint i, uint j) const;
	int xToI(int x) const;
	int yToJ(int y) const;
	int iToX(uint i) const;
	int jToY(uint j) const;
};

#endif // FIELD_H
