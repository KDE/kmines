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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FIELD_H
#define FIELD_H

#include <QPixmap>
#include <QPushButton>

#include <ksvgrenderer.h>

#include "solver/bfield.h"
#include "defines.h"
#include "kminestheme.h"

//-----------------------------------------------------------------------------
class Field : public QWidget, public BaseField
{
 Q_OBJECT
 public:
    enum ActionType { Reveal = 0, AutoReveal, SetFlag, UnsetFlag, SetUncertain,
                      UnsetUncertain, Nb_Actions };
    struct ActionData {
      const char *name, *event, *eventMessage;
    };
    static const ActionData ACTION_DATA[Nb_Actions];

 public:
    explicit Field(QWidget *parent);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    //virtual void  zoomChanged() { adjustSize(); }

    void setLevel(const Level &level);
    void setReplayField(const QString &field);
    const Level &level() const { return _level; }
    void reset(bool init);

    GameState gameState() const { return _state; }
    bool isActive() const { return _state!=Paused && _state!=GameOver; }
    void pause();
    void setGameOver() { _state = GameOver; }

    void moveCursor(Neighbour);
    void moveToEdge(Neighbour);
    void doReveal() { doReveal(_cursor); }
    void doMark()   { doMark(_cursor); }
    void doUmark()  { doUmark(_cursor); }
    void keyboardAutoReveal();
    CaseState doAction(ActionType type, const KGrid2D::Coord &c,
                     bool completeReveal, KGrid2D::CoordList *autorevealed = 0,
                     bool *caseUncovered = 0);

    void readSettings();

    void setAdvised(const KGrid2D::Coord &c, double proba);
    void setSolvingState(SolvingState state) { _solvingState = state; }
    SolvingState solvingState() const { return _solvingState; }

    void adjustCaseSize(const QSize & boardsize);
    
    //Moved from old digital clock class
    bool cheating() const { return _cheating; }
    uint nbActions() const { return _nbActions; }
    void setCheating(bool cheatState) { _cheating = cheatState; }
    void resetNbAction() { _nbActions=0; }
    void addNbAction() { _nbActions++; }
	
 signals:
    void updateStatus(bool);
    void gameStateChanged(GameState);
    void setMood(Mood);
    void addAction(const KGrid2D::Coord &, Field::ActionType);

 protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

//was Frame
    enum PixmapType { FlagPixmap = 0, Num1Pixmap, Num2Pixmap, Num3Pixmap,
         Num4Pixmap, Num5Pixmap, Num6Pixmap, Num7Pixmap, Num8Pixmap,
         MinePixmap, ExplodedPixmap, ErrorPixmap, QuestionPixmap, 
         Nb_Pixmap_Types, NoPixmap = Nb_Pixmap_Types };
    enum { Nb_Advised = 5 };

    void drawBox(QPainter &, const QPoint &, bool pressed,
                 PixmapType, const QString &text,
                 uint nbMines, int advised, bool hasFocus) ;

 private slots:
     void keyboardAutoRevealSlot();

 private:
    void updatePixmaps();

//was Frame
    QPixmap        _pixmaps[Nb_Pixmap_Types];
    QPixmap        _advised[Nb_Advised];
    KSvgRenderer svg;
    KMinesTheme theme;

    //previously in digital clock
    uint _nbActions;
    bool _cheating;

    void drawPixmap(QPixmap &, PixmapType, bool mask);
    void drawAdvised(QPixmap &, uint i, bool mask) ;
    void initPixmap(QPixmap &, bool mask) ;//end Frame

	GameState   _state;
	bool              _reveal;
	SolvingState _solvingState;
	KGrid2D::Coord  _cursor, _cursor_back, _advisedCoord;
        KGrid2D::CoordList     _pressedCoords;
	double        _advisedProba;
	int               _currentAction;
	Level          _level;
        int     borderSize;

	void pressCase(const KGrid2D::Coord &, bool);
	void pressClearFunction(const KGrid2D::Coord &, bool);
	void placeCursor(const KGrid2D::Coord &);
	void revealActions(bool press);

    void doAutoReveal(const KGrid2D::Coord &);
    bool doReveal(const KGrid2D::Coord &, KGrid2D::CoordList *autorevealed = 0,
                  bool *caseUncovered = 0);
    void doMark(const KGrid2D::Coord &);
    void doUmark(const KGrid2D::Coord &);
    void changeCase(const KGrid2D::Coord &, CaseState newState);
    void addMarkAction(const KGrid2D::Coord &, CaseState newS, CaseState oldS);

    QPoint toPoint(const KGrid2D::Coord &) const;
    KGrid2D::Coord fromPoint(const QPoint &) const;
    QRect toRect(const KGrid2D::Coord &) const;

    void drawCase(QPainter &, const KGrid2D::Coord &,
                  bool forcePressed = false);

    int mapMouseButton(QMouseEvent *) const;
    void resetAdvised();
    void setState(GameState);
};

#endif // FIELD_H
