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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ADVISE_FULL_H
#define __ADVISE_FULL_H

#include <list>
#include <algorithm>

#include "headerP.h"


namespace AdviseFull {
    class EquationSet {
    public: // Well, why is it necessary?
        struct Equation {
            set<short> pointSets;
            short mines;
        };
        typedef map<short, short> Solution;

    public:
        EquationSet();
        EquationSet(AdviseFast::FactSet const &facts);

        list<Equation> _equations;
        map<short, CoordSet> _pointSets;

        /** Make sure no _pointSets have
         * non-empty intersection */
        void normalize();

        /** Returns in *results a set of equation sets
         * which can be solved separately.
         * *this assumed normalized :) */
        void separate(list<EquationSet> *results) const;

        /** Solves... returns _pointSets.
         * It's nice to have *this separated :) */
        map<short, CoordSet> const &solve(
            list<Solution> *results) const;

        void prettyprint() const;

    private:
        /** One more than max(_pointSets[i].first) */
        short _maxPointSet;

        /** Substitutes a pointSet in all equations */
        void substitute(
            short out,
            set<short> const &in);
    };

    bool surePoints(
        map<short, CoordSet> const &m,
        list<EquationSet::Solution> const &l,
        CoordSet *surePoints);

    /** The fourth argument is a fraction of mines in the "pool" */
    void getProbabilities(
        map<short, CoordSet> const &m,
        list<EquationSet::Solution> const &l,
        ProbabilityMap *probabilities,
        float fraction = 0);

    /** Get the quotient of the number of variants of
     * point distribution satisfying dividend and divisor
     * solutions */
    /** The fourth argument is a fraction of mines in the "pool" */
    float variantNumberFraction(
        map<short, CoordSet> const &m,
        EquationSet::Solution const &dividend,
        EquationSet::Solution const &divisor,
        float fraction = 0);
}

#endif
