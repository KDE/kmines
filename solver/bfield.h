/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef BASE_FIELD_H
#define BASE_FIELD_H

#include <qcstring.h>

#include <krandomsequence.h>

#include "grid2d.h"
#include "defines.h"


class BaseField : public Grid2D::Square<KMines::Case>, public KMines
{
 public:
    // seed for KRandomSequence (used by solver check programs)
	BaseField(long seed = 0);
    virtual ~BaseField() {}

    void reset(uint width, uint height, uint nbMines);

// --------------------------
// interface used by the solver
    uint nbMines() const { return _nbMines; }

    /**
     * @return false if the case revealed contains a mine.
     */
	bool reveal(const Grid2D::Coord &c, Grid2D::CoordSet *autorevealed,
                bool *caseUncovered);

    bool isCovered(const Grid2D::Coord &p) const
        { return ( state(p)!=KMines::Uncovered ); }
    uint nbMinesAround(const Grid2D::Coord &) const;
    void coveredNeighbours(const Grid2D::Coord &p, Grid2D::CoordSet &n) const;
    bool isSolved() const
        { return (size() - _nbUncovered)==_nbMines; }
    void mark(const Grid2D::Coord &);
// -------------------------

    uint nbMarked() const { return _nbMarked; }
    QCString string() const;

    void showAllMines();

 protected:
    bool _uMark;

    bool firstReveal() const { return _firstReveal; }
    KMines::CaseState state(const Grid2D::Coord &p) const
        { return (*this)[p].state; }
    bool hasMine(const Grid2D::Coord &p) const { return (*this)[p].mine; }
    virtual void changeCase(const Grid2D::Coord &, KMines::CaseState);
    bool autoReveal(const Grid2D::Coord &, bool *caseUncovered);
    void umark(const Grid2D::Coord &);
    void completeReveal();

 private:
	bool            _firstReveal;
    uint            _nbUncovered, _nbMarked, _nbUncertain, _nbMines;
    KRandomSequence _random;

	void uncover(const Grid2D::Coord &, Grid2D::CoordSet *autoreveal);
    void changeState(KMines::CaseState, int increment);
};

QTextStream &operator <<(QTextStream &, const BaseField &);

#endif
