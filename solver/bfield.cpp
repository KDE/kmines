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

#include "bfield.h"


using namespace KGrid2D;

BaseField::BaseField(long seed)
    : _nbUncovered(0), _nbMarked(0), _nbUncertain(0), _random(seed)
{}

CoordList BaseField::coveredNeighbours(const Coord &p) const
{
    CoordList n;
    CoordList tmp = neighbours(p);
    for (CoordList::const_iterator it=tmp.begin(); it!=tmp.end(); ++it)
        if ( state(*it)!=Uncovered ) n.append(*it);
    return n;
}

uint BaseField::nbMinesAround(const Coord &p) const
{
	uint nb = 0;
    CoordList n = neighbours(p);
    for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
        if ( hasMine(*it) ) nb++;
	return nb;
}

void BaseField::reset(uint width, uint height, uint nbMines)
{
	_firstReveal = true;
    _nbMarked = 0;
    _nbUncertain = 0;
    _nbUncovered = 0;
    _nbMines = nbMines;

    Case tmp;
	tmp.mine = false;
	tmp.state = Covered;
    resize(width, height);
    fill(tmp);
}

bool BaseField::checkField(uint w, uint h, uint nb, const QString &s)
{
    if ( s.length()!=w*h ) return false;
    uint n = 0;
	unsigned int strLength(s.length());
    for (uint i=0; i<strLength; i++)
        if ( s[i]=="1" ) n++;
        else if ( s[i]!="0" ) return false;
    return ( n==nb );
}

void BaseField::initReplay(const QString &s)
{
    Q_ASSERT( checkField(width(), height(), _nbMines, s) );

    _firstReveal = false;

    Case tmp;
    tmp.state = Covered;
	unsigned int strLength(s.length());
    for (uint i=0; i<strLength; i++) {
        tmp.mine = ( s[i]=="1" );
        at(i) = tmp;
    }
}

void BaseField::changeState(KMines::CaseState state, int inc)
{
    switch (state) {
    case Uncovered: _nbUncovered += inc; break;
	case Uncertain: _nbUncertain += inc; break;
	case Marked:    _nbMarked    += inc; break;
	default: break;
    }
}

void BaseField::changeCase(const Coord &p, KMines::CaseState newState)
{
    changeState(state(p), -1);
    changeState(newState, 1);
	(*this)[p].state = newState;
}

void BaseField::uncover(const Coord &p, CoordList *autorevealed)
{
    if ( state(p)!=Covered ) return;
    changeCase(p, Uncovered);

	if ( nbMinesAround(p)==0 ) {
        CoordList n = coveredNeighbours(p);
        if (autorevealed) *autorevealed += n;
        for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
            uncover(*it, autorevealed);
    }
}

void BaseField::showAllMines(bool won)
{
	for (uint i=0; i<size(); i++) {
            Coord p = coord(i);
		    if ( hasMine(p) && state(p)!=Exploded && state(p)!=Marked ) {
				changeCase(p, won ? Marked : Uncovered);
                if ( !won ) _nbUncovered--; // not an empty case ...
            }
    }
}

bool BaseField::autoReveal(const Coord &p, bool *caseUncovered)
{
    if ( state(p)!=Uncovered ) return true;

	uint nb = nbMinesAround(p);
    CoordList n = neighbours(p);
    for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
        if ( state(*it)==Marked ) nb--;
    if ( nb==0 ) // number of surrounding mines == number of marks :)
        for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
            if ( !reveal(*it, 0, caseUncovered) ) return false;
    return true;
}

bool BaseField::reveal(const Coord &p, CoordList *autorevealed,
                       bool *caseUncovered)
{
	if ( state(p)!=Covered ) return true;

    if (_firstReveal) {
        _firstReveal = false;
        // set mines positions on field ; must avoid the first
		// revealed case
        uint n = size() - 1; // minus one case free
        Q_ASSERT( _nbMines<n );
		for(uint k=0; k<_nbMines; k++) {
            uint pos = _random.getLong(n - k);
            uint i = 0;
            Coord tmp;
            for (;;) {
                tmp = coord(i);
                if ( !(tmp==p) && !hasMine(tmp) ) {
                    if ( pos==0 ) break;
                    pos--;
                }
                i++;
            }
            (*this)[tmp].mine = true;
		}
    }

    if ( !hasMine(p) ) {
        uncover(p, autorevealed);
        if (caseUncovered) *caseUncovered = true;
        return true;
    }

    // explosion
    changeCase(p, Exploded);

    // find all errors
    for (uint i=0; i<size(); i++) {
        Coord p = coord(i);
        if ( state(p)==Marked && !hasMine(p) ) changeCase(p, Error);
    }
    return false;
}

void BaseField::completeReveal()
{
    for (;;) {
        bool changed = false;
        for (uint i=0; i<size(); i++) {
            Coord c = coord(i);
            if ( state(c)!=Uncovered ) continue;
            autoReveal(c, &changed);
            uint nb = nbMinesAround(c);
            CoordList n = neighbours(c);
            for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
                if ( state(*it)!=Uncovered ) nb--;
            if (nb) continue;
            for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
                if ( state(*it)!=Uncovered && state(*it)!=Marked ) {
                    changed = true;
                    changeCase(*it, Marked);
                }
        }
        if ( !changed ) break;
    }
}

void BaseField::doMark(const Coord &c)
{
    if ( state(c)!=Covered ) return;
    changeCase(c, Marked);
}

QCString BaseField::string() const
{
    QCString s(size());
    for (uint i=0; i<size(); i++)
        s[i] = (hasMine(coord(i)) ? '1' : '0');
    return s;
}
