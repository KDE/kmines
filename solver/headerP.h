/*
 * Copyright (c) 2001 Mikhail Kourinny (mkourinny@yahoo.com)
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
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __HEADERP_H
#define __HEADERP_H

//#define DEBUG 2

#include <set>
#include <list>
#include <map>
#include <memory>
#include <iostream>

#include "bfield.h"


using namespace KGrid2D;
using std::cout;
using std::endl;

typedef std::set<Coord, std::less<Coord> > CoordSet;

inline std::ostream &operator <<(std::ostream &s, const Coord &c)
{
  s << '(' << c.first << ',' << c.second << ')' << endl;
  return s;
}

inline std::ostream &operator <<(std::ostream &s, const CoordSet &set)
{
  for(CoordSet::const_iterator i=set.begin(); i!=set.end(); ++i)
    s << *i;
  return s;
}

inline std::ostream &operator <<(std::ostream &s, const BaseField &f)
{
  for (uint j=0; j<f.height(); j++) {
    for (uint i=0; i<f.width(); i++) {
      Coord c(i, j);
      if ( f.isCovered(c) ) s << "? ";
      else s << f.nbMinesAround(c) << ' ';
    }
    s << endl;
  }
  return s;
}

namespace AdviseFast {

    /** A fact - number of mines in adjacent cells */
    struct Fact {
        CoordSet pointSet;
        short mines;
    };
    std::ostream &operator <<(std::ostream &, Fact const &);

    /** A set of facts that can be generated out of Field */
    class FactSet : public std::map<Coord, Fact> {
    public:
        FactSet(BaseField *);
        BaseField const *getField() const { return _field;}

        /** Reveals a point on the field underlining
         * Returns false on blowup !!! */
        bool reveal(
            Coord what,
            CoordSet *affectedFacts);
        void mark(
            Coord what,
            CoordSet *affectedFacts);
        CoordSet const *getContainingFacts(
            Coord const &)
            const;
        /** May be used to substitute fact */
        void addFact(Coord const &, Fact const &);
        void deleteFact(Coord const &);
        void retrieveFact(Coord which, Fact *where);

    private:
        BaseField *_field;
		std::map<Coord, CoordSet> _containingFacts;
        CoordSet _marked;
    };
    std::ostream &operator <<(std::ostream &, FactSet const &);

    /** A Rule abstraction that can be applied.
     * Applying the rule results in either modifyling the
     * RuleSet which it belongs to or FactSet it is based on
     * or both ;)
		*/
    class RuleSet;
    struct Rule {
        Rule(RuleSet *parent);
        virtual ~Rule();
        virtual bool apply(CoordSet *surePoints) = 0;

        RuleSet *_parent;
        FactSet *_facts;
#if defined(DEBUG)
#  if DEBUG >= 2
    private:
        static int leaks;
#  endif
#endif
    };

    /** A set of rules */
    class RuleSet {
    public:
        enum RuleType {
            EMPTY,
            FULL,
            INCLUDE,
            INCLUDE1,
            INTERSECT,
            INTERSECT1,
            GENERAL};

        typedef std::pair<RuleType, CoordSet> Entry;

        RuleSet(FactSet *);
        ~RuleSet();
        void addRule(Entry const &);

        /** A factory method */
        Rule *newRule(Entry const &);

        /** Remove all references to a point from RuleSet */
        void removeRef(Coord);

        /** removeRef + add a General Rule */
        void addGeneral(Coord);

        /** Returns false on blowup */
        bool reveal(Coord p);

        /** Returns false on failure */
        bool getSurePoint(Coord *sp);
        /** Works until is stuck :) */
        void solve();

        FactSet *facts;

    private:
		std::set<Entry> _rules;
        CoordSet _surePoints;

        /** Fills _surePoints.
         * Returns false if nothing done. */
        bool apply();
    };

    /** Returns true on success */
    bool adviseFast(
        Coord *point,
        FactSet *facts,
        RuleSet *rules);

}


namespace AdviseFull {
    typedef std::multimap<float, Coord> ProbabilityMap;

    /** If there are sure free cells,
     * sets surePoints, otherwise sets probabilities */
    void adviseFull(
        AdviseFast::FactSet *facts,
        CoordSet *surePoints,
        ProbabilityMap *probabilities);

}

#endif
