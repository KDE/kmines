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
