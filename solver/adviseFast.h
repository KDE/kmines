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

#include "headerP.h"


namespace AdviseFast {

    class GeneralRule : public Rule {
    public:
        GeneralRule(Coord fact, RuleSet *rules);
        virtual bool apply(CoordSet *surePoints);
    private:
        Coord _fact;
	};

	class EmptyRule : public Rule {
    public:
        EmptyRule(Coord fact, RuleSet *rules);
        virtual bool apply(CoordSet *surePoints);
    private:
        Coord _fact;
	};

	class FullRule : public Rule {
    public:
        FullRule(Coord fact, RuleSet *rules);
        virtual bool apply(CoordSet *surePoints);
    private:
        Coord _fact;
	};

	class InclusionRule : public Rule {
    public:
        InclusionRule(Coord bigger, Coord smaller,
                      RuleSet *rules);
        virtual bool apply(CoordSet *surePoints);
    private:
        Coord _bigger, _smaller;
	};

	class IntersectionRule : public Rule {
    public:
        IntersectionRule(Coord bigger, Coord smaller,
                         RuleSet *rules);
        virtual bool apply(CoordSet *surePoints);
    private:
        Coord _bigger, _smaller;
	};
}
