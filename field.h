#ifndef FIELD_H
#define FIELD_H

#include <qwidget.h>
#include <qlabel.h>
#include <qpushbt.h>


/* mines field widget */
class Field : public QWidget
{
  Q_OBJECT
	
 public:
	Field( QWidget *parent=0, const char *name=0 );
	
 public slots:
	void start(uint, uint, uint);
	void stop() { _stop = TRUE; };
	void pause();
	void resume();
	void changeUMark(bool um) { u_mark = um; };
	
 signals:
	void changeCase(uint, uint);
	void updateStatus(bool);
	void updateSmiley(int);
	void endGame(int);
	void startTimer();
	void freezeTimer();
  
 protected:
	void paintEvent( QPaintEvent * );
	void mousePressEvent( QMouseEvent * );
	void mouseReleaseEvent( QMouseEvent * );
	void mouseMoveEvent( QMouseEvent * );
	
 private:
	uint **pfield;         /* array of cases */
	uint nb_w, nb_h, nb_m;
  
	bool _stop;             /* end of game ? */
	bool noGame, isPaused;
	bool first_click;
	bool u_mark;

	uint ic, jc;
	bool left_down;        /* left button pressed */
	bool mid_down;         /* mid button pressed */
  
	QPainter *pt;
	QPixmap  *pm_flag, *pm_mine, *pm_exploded, *pm_error;
	QLabel   *msg;
	QPushButton *pb;
  
	uint  computeNeighbours(uint, uint);
	void drawCase(uint, uint, uint);
	void uncover(uint, uint);
	void showMines(uint, uint);
	void changeCaseState(uint, uint, uint);
	void createMinePixmap();
	void pressCase(uint, uint, uint);
	void pressClearFunction(uint, uint, uint);
	void clearFunction(uint, uint);
	void uncoverCase(uint, uint);
	void adjustSize();
};

#endif // FIELD_H
