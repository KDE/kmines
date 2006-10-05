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

#include <assert.h>
#include <time.h>

#include "bfield.h"
#include "solver.h"
#include "headerP.h"

int main(int argc, char *argv[])
{
  if ( argc!=4 )
    qFatal("Arguments: width height nbMines");

	long seed = time(0);
    cout << "seed = " << seed << endl;

	short W, H, M;
	W = atoi(argv[1]); assert(W > 0);
	H = atoi(argv[2]); assert(H > 0);
	M = atoi(argv[3]); assert(M >= 0); // ;)

    BaseField field(seed);
    field.reset(W, H, M);

    Solver solver;
    if( !solver.solveOneStep(field) ) cout << "OOPS!!" << endl;
    else cout << "Solved!" << endl;

    cout << field << endl;

	return 0;
}
