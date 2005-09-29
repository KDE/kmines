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

#ifndef BASE_FIELD_H
#define BASE_FIELD_H

#include <qcstring.h>

#include <krandomsequence.h>
#include <kgrid2d.h>

#include "defines.h"


class BaseField : public KGrid2D::Square<KMines::Case>, public KMines
{
 public:
    // seed for KRandomSequence (used by solver check programs)
	BaseField(long seed = 0);
    virtual ~BaseField() {}

    void reset(uint width, uint height, uint nbMines);
    static bool checkField(uint width, uint height, uint nbMines,
                           const QString &field);
    void initReplay(const QString &field); // string == "0100011011000101..."

// --------------------------
// interface used by the solver
    uint nbMines() const { return _nbMines; }
    bool isCovered(const KGrid2D::Coord &p) const
        { return ( state(p)!=KMines::Uncovered ); }
    uint nbMinesAround(const KGrid2D::Coord &) const;
    KGrid2D::CoordList coveredNeighbours(const KGrid2D::Coord &p) const;
    bool isSolved() const { return (size() - _nbUncovered)==_nbMines; }

    // return false if the case revealed contains a mine.
	virtual bool doReveal(const KGrid2D::Coord &c,
                         KGrid2D::CoordList *autorevealed, bool *caseUncovered)
        { return reveal(c, autorevealed, caseUncovered); }
    virtual void doMark(const KGrid2D::Coord &);
// -------------------------

    uint nbMarked() const { return _nbMarked; }
    QCString string() const;

    void showAllMines(bool won);

 protected:
    bool firstReveal() const { return _firstReveal; }
    KMines::CaseState state(const KGrid2D::Coord &p) const
        { return (*this)[p].state; }
    bool hasMine(const KGrid2D::Coord &p) const { return (*this)[p].mine; }
    virtual void changeCase(const KGrid2D::Coord &, KMines::CaseState);
    bool reveal(const KGrid2D::Coord &c,
                KGrid2D::CoordList *autorevealed, bool *caseUncovered);
    bool autoReveal(const KGrid2D::Coord &, bool *caseUncovered);
    void completeReveal();

 private:
	bool            _firstReveal;
    uint            _nbUncovered, _nbMarked, _nbUncertain, _nbMines;
    KRandomSequence _random;

	void uncover(const KGrid2D::Coord &, KGrid2D::CoordList *autoreveal);
    void changeState(KMines::CaseState, int increment);
};

#endif
