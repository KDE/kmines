#ifndef KMINES_F_H
#define KMINES_F_H

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
	void Start(int, int, int);
	void Stop();
	void setUMark(int);
	void pause();
	void resume();
	
 signals:
	void changeCase(int,int);
	void updateStatus();
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
	int  **pfield;           /* array of cases */
	int  nb_w, nb_h, nb_m;
  
	bool stop;             /* end of game ? */
	bool noGame, isPaused;
	bool first_click;
	int  u_mark;

	int  ic, jc;
	int  left_down;        /* left button pressed */
	int  mid_down;         /* mid button pressed */
  
	QPainter *pt;
	QPixmap  *pm_flag, *pm_mine, *pm_exploded, *pm_error;
	QLabel   *msg;
	QPushButton *pb;
  
	int  computeNeighbours(int, int);
	void drawCase(int, int, int);
	void uncover(int, int);
	void showMines(int, int);
	void changeCaseState(int, int, int);
	void createMinePixmap();
	void pressCase(int, int, int);
	void pressClearFunction(int, int, int);
	void clearFunction(int, int);
	void uncoverCase(int, int);
	void adjustSize();
};

#endif
