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

#include "adviseFull.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

using std::list;
using std::map;
using std::set;
using namespace AdviseFull;

AdviseFull::EquationSet::EquationSet() :
	_maxPointSet(0)
{}

AdviseFull::EquationSet::EquationSet(
	AdviseFast::FactSet const &facts) :
	_maxPointSet(0)
{
	AdviseFast::FactSet::const_iterator i;
	for(i=facts.begin(); i!=facts.end(); ++i, ++_maxPointSet){
		Equation e;
		e.pointSets.insert(_maxPointSet);
		e.mines = i->second.mines;
		_equations.push_back(e);

		_pointSets[_maxPointSet] = i->second.pointSet;
	}
}

void AdviseFull::EquationSet::normalize()
{
	short i=0;
	set<short> empty;
	for(i=0; i<_maxPointSet; ++i){
		if(!_pointSets.count(i)) continue;
		if(_pointSets[i].empty()){
			this->substitute(i, empty);
			continue;
		}
		for(short j=i+1;j<_maxPointSet; ++j){
			if(!_pointSets.count(j)) continue;
			if(_pointSets[j].empty()){
				this->substitute(j, empty);
				continue;
			}

			CoordSet intersect;
			set_intersection(
				_pointSets[i].begin(),
				_pointSets[i].end(),
				_pointSets[j].begin(),
				_pointSets[j].end(),
				inserter(intersect, intersect.begin()));
			if(intersect.empty()) continue;

			CoordSet _i, _j;
			set_difference(
				_pointSets[i].begin(),
				_pointSets[i].end(),
				_pointSets[j].begin(),
				_pointSets[j].end(),
				inserter(_i, _i.begin()));
			set_difference(
				_pointSets[j].begin(),
				_pointSets[j].end(),
				_pointSets[i].begin(),
				_pointSets[i].end(),
				inserter(_j, _j.begin()));

			set<short> _ip, _jp;
			_ip.insert(_maxPointSet);
			_jp.insert(_maxPointSet);
			_pointSets[_maxPointSet++] = intersect;
			_ip.insert(_maxPointSet);
			_pointSets[_maxPointSet++] = _i;
			_jp.insert(_maxPointSet);
			_pointSets[_maxPointSet++] = _j;

			this->substitute(i, _ip);
			this->substitute(j, _jp);
			break;
		}
	}
}

void AdviseFull::EquationSet::separate(
	list<EquationSet> *result) const
{
	result->clear(); // :)

	list<Equation> equations = _equations;

	while(!equations.empty()){
		// Add a new equation set to *results
		result->push_back(EquationSet());
		EquationSet &workingSet = result->back();
		workingSet._maxPointSet = _maxPointSet;

		// Start iteration process
		// recentlyDeceased is a set of pointSets added on the
		// last iteration
		workingSet._equations.push_back(equations.front());
		set<short> recentlyDeceased = equations.front().pointSets;
		equations.pop_front();

		// The iteration process
		while(!recentlyDeceased.empty()){

			// Temporary set<short>
			set<short> rd;

			list<Equation>::iterator i = equations.begin();
			while(i != equations.end()){
				set<short> intersect;
				set_intersection(
					i->pointSets.begin(),
					i->pointSets.end(),
					recentlyDeceased.begin(),
					recentlyDeceased.end(),
					inserter(intersect, intersect.begin()));
				if(intersect.empty()){
					++i;
				} else {
					set_difference(
						i->pointSets.begin(),
						i->pointSets.end(),
						intersect.begin(),
						intersect.end(),
						inserter(rd, rd.begin()));
					workingSet._equations.push_back(*i);
					i = equations.erase(i);
				}
			}

			// Now switch recentlyDeceased
			set<short>::iterator j;
			for(	j = recentlyDeceased.begin();
				j != recentlyDeceased.end();
				++j)
			{
				workingSet._pointSets[*j] =
					(const_cast<
						map<short, CoordSet> &>(
						_pointSets))[*j];
			}

			recentlyDeceased = rd;
		}
	}
}

map<short, CoordSet> const &AdviseFull::EquationSet::solve(
	list<Solution> *results) const
{

#ifdef DEBUG
	printf("Entering EquationSet::solve\n");
	prettyprint();
#endif

	EquationSet eqs = *this;

	// Get the most evident solutions
	Solution only;
	list<Equation> &EQS = eqs._equations;

	bool success;
	do {

	success = false;
	list<Equation>::iterator i = EQS.begin();

	while(i!=EQS.end()){
#if defined(DEBUG) && DEBUG >= 2
		printf("Taking look at the equation...\n");
#endif
		// Substitute known values
		{	set<short>::iterator j;
			set<short> known;
			for(	j = i->pointSets.begin();
				j != i->pointSets.end();
				++j)
			{
				if(only.count(*j)){
					i->mines -= only[*j];
					known.insert(*j);
				}
			}

			// STL bug ??
			for(	j = known.begin();
				j != known.end();
				++j)
				i->pointSets.erase(*j);
		}
		// From now on the equation has no known values
#if defined(DEBUG) && DEBUG >= 2
		printf("Substituted known values.\n");
#endif
		if(i->mines < 0)
			// Discrepancy
			return _pointSets;


		if(i->pointSets.empty()){
#if defined(DEBUG) && DEBUG >= 2
			printf("Empty equation.\n");
#endif
			if(i->mines){
				// No points, non-zero mine count
				// No solution
				return _pointSets;
			} else {
				i = EQS.erase(i);
				continue;
			}
		}

		if(i->mines == 0){
			set<short>::iterator j;
			for(
				j=i->pointSets.begin();
				j!=i->pointSets.end();
				++j)
				only[*j] = 0;

			EQS.erase(i);
			success = true;
			break;
		}

		if(i->pointSets.size() == 1){
			short j = *i->pointSets.begin();

			if((int)eqs._pointSets[j].size() < i->mines)
				// Discrepancy !!
				return _pointSets;

			only[j] = i->mines;

			EQS.erase(i);
			success = true;
			break;
		}

		++i;
	}

	} while(success);

	// If no equations left we have a unique solution
	if(EQS.empty()){
#ifdef DEBUG
		printf("Got a single solution!\n");
#endif
		results->push_back(only);
		return _pointSets;
	}

	// Otherwise the first equation is not empty.
	// Find the range for first element
	short var = *EQS.begin()->pointSets.begin();
	std::pair<short, short> range;
	range.first = 0;
	range.second = eqs._pointSets[var].size();

	// A list of equations containing var
	list<list<Equation>::iterator> containers;
	list<Equation>::iterator i;
	for(	i = EQS.begin();
		i != EQS.end();
		++i)
	{
		if(i->pointSets.count(var)){
			i->pointSets.erase(var);
			containers.push_back(i);

			if(i->mines < range.second)
				range.second = i->mines;

			// The total size of other point sets
			// in the equation
			short totalsize = 0;
			set<short>::iterator j;
			for(	j = i->pointSets.begin();
				j != i->pointSets.end();
				++j)
				totalsize += eqs._pointSets[*j].size();

			if(range.first < i->mines - totalsize)
				range.first = i->mines - totalsize;
		}
	}
	// Found the range

	// Now set properly equation set for first recursion
	list<list<Equation>::iterator>::iterator super_iter; // ;)
	short varvalue = range.first;
	for(	super_iter = containers.begin();
		super_iter != containers.end();
		++super_iter)
		(*super_iter)->mines -= varvalue;

	// Recursive calls here
	while(varvalue <= range.second){
		only[var] = varvalue;
		list<Solution> tempResults;
		eqs.solve(&tempResults);

		// Mix solutions with only and put them
		// in *results
		list<Solution>::iterator j;
		for(	j=tempResults.begin();
			j!=tempResults.end();
			++j)
		{
			j->insert(only.begin(), only.end());
			results->push_back(*j);
		}

		// Prepare next recursive call
		++varvalue;
		for(	super_iter = containers.begin();
			super_iter != containers.end();
			++super_iter)
			--(*super_iter)->mines;
	}

	return _pointSets;
}

void AdviseFull::EquationSet::prettyprint() const
{

#if defined(DEBUG)
	printf("Point Sets:\n");
	map<short, CoordSet>::const_iterator i;
	for(i=_pointSets.begin(); i!=_pointSets.end(); ++i){
		printf("%d:", i->first);
		CoordSet::const_iterator j;
		for(j=i->second.begin(); j!=i->second.end(); ++j)
			printf("\t(%d,%d)\n", j->second, j->first);
	}
#endif

	printf("Equations:\n");
	list<Equation>::const_iterator l;
	for(l=_equations.begin(); l!=_equations.end(); ++l){
		set<short>::const_iterator j;
		for(j=l->pointSets.begin(); j!=l->pointSets.end(); ++j)
			printf("%d ", *j);
		printf("= %d\n", l->mines);
	}
}

void AdviseFull::EquationSet::substitute(
	short out,
	set<short> const &in)
{
	list<Equation>::iterator i;
	for(	i = _equations.begin();
		i != _equations.end();
		++i)
	{
		if(i->pointSets.count(out)){
			i->pointSets.erase(out);
			i->pointSets.insert(in.begin(), in.end());
		}
	}

	_pointSets.erase(out);
}

bool AdviseFull::surePoints(
	map<short, CoordSet> const &m,
	list<EquationSet::Solution> const &l,
	CoordSet *surePoints)
{
	// A set of candidates to be surePoints
	list<short> sp;
	{
		map<short, CoordSet>::const_iterator i;
		for(i=m.begin(); i!=m.end(); ++i) sp.push_back(i->first);
	}

	// Scan solution list
	list<EquationSet::Solution>::const_iterator i;
	for(i=l.begin(); i!=l.end(); ++i){
		list<short>::iterator j = sp.begin();
		while(j != sp.end()){
			// Non-empty possibility
			if((const_cast<EquationSet::Solution &>(*i))[*j]){
				j = sp.erase(j);
				if(sp.empty()) // No candidates left
					return false;
			} else // Stay alive for now
				++j;
		}
	}

	// There are SOME sure points;
	// Fill *surePoints
	list<short>::iterator isp;
	map<short, CoordSet> &mm = const_cast<map<short, CoordSet> &>(m);
	for(isp = sp.begin(); isp != sp.end(); ++isp)
		surePoints->insert(mm[*isp].begin(), mm[*isp].end());

	return true;
}

float AdviseFull::variantNumberFraction(
	map<short, CoordSet> const &m,
	EquationSet::Solution const &dividend,
	EquationSet::Solution const &divisor,
	float fraction)
{
	short count_difference = 0;
	float quotient = 1;

	EquationSet::Solution::const_iterator i;
	for(i=divisor.begin(); i!=divisor.end(); ++i){
		int j = i->first;
		assert(m.count(j));
		int size = (const_cast<map<short, CoordSet> &>(m))[j].size();
		int dvd = (const_cast<EquationSet::Solution &>(dividend))[j];
		int dvr = (const_cast<EquationSet::Solution &>(divisor))[j];

		count_difference += dvd - dvr;

		if(dvd < dvr){
			dvr = size - dvr;
			dvd = size - dvd;
		}
		while(dvr < dvd) {
                        float num = size-dvr++;
			quotient *= num/dvr;
                }
	}

	// Sorry, expensive call, but I'm lazy :((
	if(count_difference){
		assert(fraction != 0.);
#if defined(DEBUG)
        float correction = pow( fraction/(1-fraction), count_difference );
		cout << "Got into correction, " <<
			count_difference << ' ' << correction << endl;
#endif
		quotient *= pow( (1-fraction)/fraction , -count_difference );
	}

#if defined(DEBUG) && DEBUG >= 2
	printf("variantNumberFraction: %.02f.\n", quotient);
#endif

	return quotient;
}

void AdviseFull::getProbabilities(
	map<short, CoordSet> const &m,
	list<EquationSet::Solution> const &l,
	ProbabilityMap *probabilities,
	float fraction)
{
	assert(!l.empty());
	EquationSet::Solution const &front = l.front();

	float probabilitiesSum = 0;
	map<short, float> probs;
	{ map<short, CoordSet>::const_iterator i;
		for(i=m.begin(); i!=m.end(); ++i)
			probs[i->first] = 0;
	}

	list<EquationSet::Solution>::const_iterator i;
	for(i=l.begin(); i!=l.end(); ++i){
		float frac = variantNumberFraction(m, *i, front, fraction);
		EquationSet::Solution::const_iterator j;
		for(j=i->begin(); j!=i->end(); ++j)
			probs[j->first] += j->second*frac;
		probabilitiesSum += frac;
	}

	probabilities->clear();

	map<short, float>::iterator j;
	for(j=probs.begin(); j!= probs.end(); ++j){
		CoordSet const &ps = const_cast<map<short, CoordSet> &>(m)[j->first];
		j->second /= ps.size() * probabilitiesSum;
		CoordSet::const_iterator k;
		for(k=ps.begin(); k!=ps.end(); ++k)
			probabilities->insert(
				std::pair<float, Coord>(j->second, *k));
	}

	// That's it :)
}

void AdviseFull::adviseFull(
	AdviseFast::FactSet *facts,
	CoordSet *surePoints,
	ProbabilityMap *probabilities)
{
	EquationSet eqs(*facts);

#if defined(DEBUG) && DEBUG >= 2
	eqs.prettyprint();
#endif

	eqs.normalize();
#if defined(DEBUG) && DEBUG >= 2
	eqs.prettyprint();
#endif

	list<EquationSet> eqss;
	eqs.separate(&eqss);
#ifdef DEBUG
	{list<EquationSet>::iterator i;
		for(i=eqss.begin(); i!=eqss.end(); ++i)
			i->prettyprint();
	}
#endif


	// OK, uneffective, but simple :(
	surePoints->clear();
	probabilities->clear();

	// Get a fraction;
	float fraction;
	{ BaseField const *f = facts->getField();
		fraction = ((float)f->nbMines()) / (f->width() * f->height());
	}

	/* From now on the first equation set on the list includes
	* the equation corresponding to "total" fact.  This is the
	* first equation on the set.
	*
	* Give it a special treatment ;) */
	if(!eqss.empty()) do {
		EquationSet prime = eqss.front();
		EquationSet::Equation total = prime._equations.front();
		prime._equations.pop_front();

		list<EquationSet> prime_sep;
		prime.separate(&prime_sep);

		// Find a pool
		list<EquationSet::Equation>::iterator i = prime._equations.begin();
		while(!prime._equations.empty()){
			set<short>::iterator j;
			for(	j = i->pointSets.begin();
				j != i->pointSets.end();
				++j)
				prime._pointSets.erase(*j);
			i = prime._equations.erase(i);
		}

		assert(prime._pointSets.size() <= 1);
		if(prime._pointSets.size() == 0) break;

		short pool = prime._pointSets.begin()->first;
		CoordSet const &p = prime._pointSets[pool];
#ifdef DEBUG
		cout << "Prime equation set:" << endl <<
			"	separated into " << prime_sep.size() << endl <<
			"	pool size is " << p.size() << endl;
#endif
		// Euristic
		// if( prime_sep.size () > 6 && p.size() >= prime_sep.size() * 10){
		if(p.size() < (prime_sep.size()+1) * 10)
			// No special treatment!!
			break;


		// Actually, just substitute prime (!!!)
		eqss.pop_front();
		eqss.insert(eqss.begin(),
			prime_sep.begin(),
			prime_sep.end());

		prime._equations.clear();
		EquationSet::Equation o;
		o.pointSets.insert(pool);
        // #### is the convertion right ? (NH)
		o.mines = (ushort)(fraction * p.size());
		// A precaution
		if(o.mines == 0) o.mines = 1; // ;)

		prime._equations.push_front(o);
		eqss.push_front(prime);


#ifdef DEBUG
			cout << "Specially treated:" << endl;
	{
		list<EquationSet>::iterator i;
		for(i=eqss.begin(); i!=eqss.end(); ++i)
			i->prettyprint();
	}
#endif
	} while (false);

	list<EquationSet>::const_iterator i;
	for(i=eqss.begin(); i!=eqss.end(); ++i){
		CoordSet sp; ProbabilityMap pb;

		list<EquationSet::Solution> solutions;
		map<short, CoordSet> const &m = i->solve(&solutions);
#ifdef DEBUG
		printf("Got solutions.\n");
#if defined(DEBUG) && DEBUG >= 2
		{ list<EquationSet::Solution>::iterator i;
			for(	i = solutions.begin();
				i != solutions.end();
				++i)
			{
				EquationSet::Solution::iterator j;
				for(j=i->begin(); j!=i->end(); ++j)
					printf("%d:\t%d\n",
						j->first, j->second);
				printf("\n");
			}
		}
#endif
#endif

		//bool sure =
        AdviseFull::surePoints(m, solutions, &sp);
		surePoints->insert(sp.begin(), sp.end());

		getProbabilities(m, solutions, &pb, fraction);
		probabilities->insert(pb.begin(), pb.end());
	}

	// That's it
	return;
}
