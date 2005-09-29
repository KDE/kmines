/*
 * Copyright (c) 2001 Mikhail Kourinny (mkourinny@yahoo.com)
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

#include "adviseFast.h"

#include <algorithm>


using namespace AdviseFast;

std::ostream &AdviseFast::operator <<(std::ostream &s, Fact const &f)
{
  return s << f.pointSet << "= " << f.mines;
}

AdviseFast::FactSet::FactSet(BaseField *field) :
	_field(field)
{
	Fact globalFact; globalFact.mines = field->nbMines();

	int i, j;
	for(i=field->height()-1; i>=0; --i)
		for(j=field->width()-1; j>=0; --j){
			Coord p(j, i);
            // #### hasMine implies isCovered (and the solver should not
            // know if there is a mine :) [NH]
            if ( field->isCovered(p) /*|| field->hasMine(p)*/ )
				globalFact.pointSet.insert(p);
			else {
				Fact f;
				this->retrieveFact(p, &f);
				this->addFact(p, f);
			}
		}

	this->addFact(Coord(-1,-1), globalFact);
}

void AdviseFast::FactSet::retrieveFact(
	Coord which,
	Fact *where)
{
    where->mines = (_field->isCovered(which) ? -1
                    : (int)_field->nbMinesAround(which));
    CoordList tmp = _field->coveredNeighbours(which);
    for (CoordList::const_iterator it = tmp.begin(); it!=tmp.end(); ++it)
        where->pointSet.insert(*it);
}

void AdviseFast::FactSet::addFact(
	Coord const &point,
	Fact const &fact)
{
	if(this->count(point)) this->deleteFact(point);

	Fact &f = ((*this)[point] = fact);


	// Remove marked points
	CoordSet marked;
	set_intersection(
		f.pointSet.begin(),
		f.pointSet.end(),
		_marked.begin(),
		_marked.end(),
		inserter(marked, marked.begin()));

	CoordSet::iterator i;
	for(i=marked.begin(); i!=marked.end(); ++i)
		f.pointSet.erase(*i);
	f.mines -= marked.size();

	// Don't insert empty fact
	if(f.pointSet.empty()) { this->erase(point); return;}

	for(i=f.pointSet.begin(); i!=f.pointSet.end(); ++i)
		_containingFacts[*i].insert(point);
}

void AdviseFast::FactSet::deleteFact(
	Coord const &point)
{
	if(!this->count(point)) return;
	CoordSet::iterator i;
	Fact &f = (*this)[point];
	for(i=f.pointSet.begin(); i!=f.pointSet.end(); ++i){
		_containingFacts[*i].erase(point);
		if(_containingFacts[*i].empty())
			_containingFacts.erase(*i);
	}
	this->erase(point);
}

bool AdviseFast::FactSet::reveal(
	Coord point,
	CoordSet *affectedFacts)
{
	// Tolerate :)
	if( !_field->isCovered(point) ) return true; // :)

	CoordList tmp;
	if(_field->doReveal(point, &tmp, 0) == false)
		// Blew up :(
		return false;

    CoordSet autorevealed;
    for (CoordList::const_iterator it = tmp.begin(); it!=tmp.end(); ++it)
        autorevealed.insert(*it);
	autorevealed.insert(point);
	affectedFacts->insert(autorevealed.begin(), autorevealed.end());

	CoordSet::const_iterator i;
	for(i=autorevealed.begin(); i!=autorevealed.end(); ++i)
	{
		// I still think that each poing will belong to
		// at least one fact, but don't want to waste time
		// proving it :)
		if(_containingFacts.count(*i)){
			CoordSet const &affF = _containingFacts[*i];
			affectedFacts->insert(
				affF.begin(), affF.end());
			for(CoordSet::const_iterator j=affF.begin();
				j!=affF.end();
				++j)
			{
				(*this)[*j].pointSet.erase(*i);
				if((*this)[*j].pointSet.empty())
					this->erase(*j);
			}
			_containingFacts.erase(*i);
		}

		Fact f; retrieveFact(*i, &f);
		this->addFact(*i, f);
	}

	return true;
}

void AdviseFast::FactSet::mark(
	Coord point,
	CoordSet *affectedFacts)
{
	if(_marked.count(point)) return;
	_marked.insert(point);

	// I still think that each poing will belong to
	// at least one fact, but don't want to waste time
	// proving it :)
	if(_containingFacts.count(point)){
		CoordSet const &affF = _containingFacts[point];
		affectedFacts->insert(affF.begin(), affF.end());
		for(CoordSet::const_iterator i=affF.begin(); i!=affF.end(); ++i){
			(*this)[*i].pointSet.erase(point);
			(*this)[*i].mines--;
			if((*this)[*i].pointSet.empty())
				this->erase(*i);
		}
		_containingFacts.erase(point);
	}

	_field->doMark(point);
}

CoordSet const *AdviseFast::FactSet::getContainingFacts(
	Coord const &point) const
{
	if(_containingFacts.count(point))
		return &const_cast<std::map<Coord, CoordSet> &>(_containingFacts)
			[point];
	else return 0;
}

std::ostream &AdviseFast::operator <<(std::ostream &s, FactSet const &fs)
{
	FactSet::const_iterator i;
	for(i=fs.begin(); i!=fs.end(); ++i)
        s << i->first << ": " << i->second << endl;
	return s;
}

bool AdviseFast::adviseFast(
	Coord *,
	FactSet *,
	RuleSet *)
{ return false;}
