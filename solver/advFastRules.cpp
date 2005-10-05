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

#include <algorithm>
#include <assert.h>
#include "adviseFast.h"

using std::set;

AdviseFast::RuleSet::RuleSet(FactSet *f) :
	facts(f)
{
	FactSet::iterator i;
	for(i=facts->begin(); i!=facts->end(); ++i)
		addGeneral(i->first);
}

AdviseFast::RuleSet::~RuleSet(){
}

void AdviseFast::RuleSet::addRule(Entry const &entry)
{
	_rules.insert(entry);
}

bool AdviseFast::RuleSet::getSurePoint(Coord *sp)
{
	if(_surePoints.empty()){
		if(!apply()) return false;
	}

	CoordSet::iterator i = _surePoints.begin();
	*sp = *i;
	_surePoints.erase(i);

	return true;
}

bool AdviseFast::RuleSet::reveal(Coord what)
{
	CoordSet affectedFacts;
	if(!facts->reveal(what, &affectedFacts))
		// OOPS :(
		return false;

	CoordSet::iterator i;
	for(	i = affectedFacts.begin();
		i != affectedFacts.end();
		++i)
		this->addGeneral(*i);

	return true;
}

void AdviseFast::RuleSet::solve()
{
	Coord p;
	while(getSurePoint(&p)) {
          bool res = reveal(p);
          assert(res);
          Q_UNUSED(res);
        }
}

bool AdviseFast::RuleSet::apply()
{
	while(!_rules.empty()){
		set<Entry>::iterator i = _rules.begin();
		std::auto_ptr<Rule>  r (this->newRule(*i));
		_rules.erase(i);

		if(r->apply(&this->_surePoints)) return true;
	}

	return false;
}

AdviseFast::Rule *
AdviseFast::RuleSet::newRule(Entry const &e){
	CoordSet::const_iterator i = e.second.begin();
	Coord p, p1;
	switch(e.first){
		case EMPTY:
			assert(e.second.size() == 1);
			return new EmptyRule(*i, this);

		case FULL:
			assert(e.second.size() == 1);
			return new FullRule(*i, this);

		case INCLUDE:
			assert(e.second.size() == 2);
			p = *i; ++i; p1 = *i;
			return new InclusionRule(p, p1, this);

		case INCLUDE1:
			assert(e.second.size() == 2);
			p = *i; ++i; p1 = *i;
			return new InclusionRule(p1, p, this);

		case INTERSECT:
			assert(e.second.size() == 2);
			p = *i; ++i; p1 = *i;
			return new IntersectionRule(p, p1, this);

		case INTERSECT1:
			assert(e.second.size() == 2);
			p = *i; ++i; p1 = *i;
			return new IntersectionRule(p1, p, this);

		case GENERAL:
			assert(e.second.size() == 1);
			return new GeneralRule(*i, this);

		default:
			assert(false);
	}

	// Make compiler happy
	return 0;
}

void AdviseFast::RuleSet::removeRef(Coord p){
	set<Entry>::iterator i, j;

	for(	i = j = _rules.begin();
		i != _rules.end();
		i = j)
	{
		++j;
		if(i->second.count(p)) _rules.erase(i);
	}
}

void AdviseFast::RuleSet::addGeneral(Coord p){
	this->removeRef(p);
	Entry e;
	e.first = GENERAL;
	e.second.insert(p);
	this->addRule(e);
}

#if defined(DEBUG) && DEBUG >= 2
int AdviseFast::Rule::leaks = 0;
#endif

AdviseFast::Rule::Rule(RuleSet *parent) :
	_parent(parent),
	_facts(parent->facts)
{
#if defined(DEBUG) && DEBUG >= 2
	cout << "Rule::Rule, leaks = " << ++leaks << endl;
#endif
}

AdviseFast::Rule::~Rule()
{
#if defined(DEBUG) && DEBUG >= 2
	cout << "Rule::~Rule, leaks = " << --leaks << endl;
#endif
}

AdviseFast::GeneralRule::GeneralRule(
	Coord fact,
	RuleSet *parent) :
	Rule(parent),
	_fact(fact)
{}

bool AdviseFast::GeneralRule::apply(CoordSet *)
{

#if defined(DEBUG) && DEBUG >= 2
	operator <<(
			cout << "Applying general rule ",
			_fact) << endl;
#endif

	// Return if there's no more such fact
	if(!_facts->count(_fact)) return false;
	Fact const &f = (*_facts)[_fact];

#if defined(DEBUG) && DEBUG >= 2
	cout << f << endl;
#endif

	// Insert intersection rules first
	// relatedFacts -- facts which have non-zero intersection
	CoordSet relatedFacts;
	{
		CoordSet::const_iterator i;
		for(	i=f.pointSet.begin();
			i!=f.pointSet.end();
			++i){

			CoordSet const & ps =
				*_facts->getContainingFacts(*i);
			relatedFacts.insert(
				ps.begin(), ps.end());
		}
	}
	relatedFacts.erase(_fact); // ;)

	CoordSet::iterator i;
	for(	i=relatedFacts.begin();
		i!=relatedFacts.end();
		++i)
	{
		RuleSet::Entry e;
		e.second.insert(_fact);
		e.second.insert(*i);

		e.first = RuleSet::INTERSECT1; _parent->addRule(e);
		e.first = RuleSet::INTERSECT; _parent->addRule(e);
		e.first = RuleSet::INCLUDE1; _parent->addRule(e);
		e.first = RuleSet::INCLUDE; _parent->addRule(e);
	}

	// Now simple rules, so that they appear first in the list
	RuleSet::Entry e; e.second.insert(_fact);
	e.first = RuleSet::FULL; _parent->addRule(e);
	e.first = RuleSet::EMPTY; _parent->addRule(e);

	// No point revealed, so...
	return false;
}

AdviseFast::EmptyRule::EmptyRule(
	Coord fact,
	RuleSet *parent) :
	Rule(parent),
	_fact(fact)
{}

bool AdviseFast::EmptyRule::apply(
	CoordSet *surePoints)
{

#if defined(DEBUG) && DEBUG >= 2
	operator <<(
			cout << "Applying empty rule ",
			_fact) << endl;
#endif

	if(!_facts->count(_fact)) return false;
	Fact const &f = (*_facts)[_fact];

#if defined(DEBUG) && DEBUG >= 2
	cout << f << endl;
#endif

	// FactSet does not contain empty facts!!
	assert(!f.pointSet.empty());

	// If there are mines around, alas :(
	if(f.mines) return false;

#if defined(DEBUG) && DEBUG >= 2
	cout << "succeeded!" << endl;
#endif

	surePoints->insert(
		f.pointSet.begin(),
		f.pointSet.end());

	_parent->removeRef(_fact);

	return true;
}

AdviseFast::FullRule::FullRule(
	Coord fact,
	RuleSet *parent) :
	Rule(parent),
	_fact(fact)
{}

bool AdviseFast::FullRule::apply(
	CoordSet */*surePoints*/)
{

#if defined(DEBUG) && DEBUG >= 2
	operator <<(
			cout << "Applying full rule ",
			_fact) << endl;
#endif

	if(!_facts->count(_fact)) return false;
	Fact f = (*_facts)[_fact];

#if defined(DEBUG) && DEBUG >= 2
	cout << f << endl;
#endif

	// FactSet does not contain empty facts!!
	assert(!f.pointSet.empty());

	// The point set is not full of mines... :(
	if(f.mines != (int)f.pointSet.size()) return false;

#if defined(DEBUG) && DEBUG >= 2
	cout << "succeeded!" << endl;
#endif

	CoordSet affectedFacts;
	CoordSet::iterator i;
	for(	i=f.pointSet.begin();
		i!=f.pointSet.end();
		++i)
		_facts->mark(*i, &affectedFacts);

	for(	i=affectedFacts.begin();
		i!=affectedFacts.end();
		++i)
		_parent->addGeneral(*i);
	_parent->removeRef(_fact);

	// No mines revealed
	return false;
}

AdviseFast::InclusionRule::InclusionRule(
	Coord bigger, Coord smaller,
	RuleSet *parent) :
	Rule(parent),
	_bigger(bigger), _smaller(smaller)
{}

bool AdviseFast::InclusionRule::apply(
	CoordSet */*surePoints*/)
{

#if defined(DEBUG) && DEBUG >= 2
	cout << "Applying inclusion rule ";
	operator <<(cout, _bigger) << ' ';
	operator <<(cout, _smaller) << endl;
#endif

	if(!_facts->count(_bigger)) return false;
	if(!_facts->count(_smaller)) return false;

	Fact b = (*_facts)[_bigger];
	Fact s = (*_facts)[_smaller];

#if defined(DEBUG) && DEBUG >= 2
	cout << b << endl << s << endl;
#endif

	assert(!s.pointSet.empty());

	CoordSet diff;
	set_difference(
		s.pointSet.begin(),
		s.pointSet.end(),
		b.pointSet.begin(),
		b.pointSet.end(),
		inserter(diff, diff.begin()));
	if(!diff.empty())
		// That is s is not included in b
		return false;

#if defined(DEBUG) && DEBUG >= 2
	cout << "succeeded!" << endl;
#endif

	diff.clear();
	set_difference(
		b.pointSet.begin(),
		b.pointSet.end(),
		s.pointSet.begin(),
		s.pointSet.end(),
		inserter(diff, diff.begin()));

	if(diff.empty()){
		_facts->deleteFact(_bigger);
		_parent->removeRef(_bigger);
	} else {
		b.pointSet = diff;
		b.mines -= s.mines;
		_facts->addFact(_bigger, b);
		_parent->addGeneral(_bigger);
	}

	// No points revealed
	return false;
}

AdviseFast::IntersectionRule::IntersectionRule(
	Coord bigger, Coord smaller,
	RuleSet *parent) :
	Rule(parent),
	_bigger(bigger), _smaller(smaller)
{}

bool AdviseFast::IntersectionRule::apply(
	CoordSet *surePoints)
{

#if defined(DEBUG) && DEBUG >= 2
	cout << "Applying intersection rule ";
	operator <<(cout, _bigger) << ' ';
	operator <<(cout, _smaller) << endl;
#endif

	if(!_facts->count(_bigger)) return false;
	if(!_facts->count(_smaller)) return false;

	Fact b = (*_facts)[_bigger];
	Fact s = (*_facts)[_smaller];

#if defined(DEBUG) && DEBUG >= 2
	cout << b << endl << s << endl;
#endif

	CoordSet diff;
	set_difference(
		b.pointSet.begin(),
		b.pointSet.end(),
		s.pointSet.begin(),
		s.pointSet.end(),
		inserter(diff, diff.begin()));

	if((int)diff.size() != b.mines - s.mines)
		// Oops :(
		return false;

#if defined(DEBUG) && DEBUG >= 2
	cout << "succeeded!" << endl;
#endif

	CoordSet cross, diffs;
	set_difference(
		s.pointSet.begin(),
		s.pointSet.end(),
		b.pointSet.begin(),
		b.pointSet.end(),
		inserter(diffs, diffs.begin()));
	set_intersection(
		s.pointSet.begin(),
		s.pointSet.end(),
		b.pointSet.begin(),
		b.pointSet.end(),
		inserter(cross, cross.begin()));

	b.pointSet = diff;
	b.mines -= s.mines;
	_facts->addFact(_bigger, b);

	s.pointSet = cross;
	_facts->addFact(_smaller, s);

	{
		_parent->removeRef(_bigger);
		_parent->addGeneral(_smaller);

		RuleSet::Entry e;
		e.first = RuleSet::FULL;
		e.second.insert(_bigger);
		_parent->addRule(e);
	}

	if(diffs.empty()) return false;

	// Otherwise we have something to reveal!!
	surePoints->insert(diffs.begin(), diffs.end());
	return true;
}
