/*
 * Copyright (c) 2002 Nicolas HADACEK  (hadacek@kde.org)
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
/** A program to test advisory library */

#include "bfield.h"
#include "headerP.h"

#define W 10
#define H 10

int main(int argc, char *argv[])
{
    long seed = (argc < 2 ? time(0) : atoi(argv[1]));
    cout << "seed = " << seed << endl;

    BaseField f(seed);
    f.reset(W, H, 10);

    KRandomSequence random(seed);
    Coord c(random.getLong(W), random.getLong(H));
    f.doReveal(c, 0, 0);

	AdviseFast::FactSet facts(&f);
	AdviseFast::RuleSet rules(&facts);

	rules.solve();

	cout << f << endl;
	if(!f.isSolved()) cout << facts << endl;

	return 0;
}
