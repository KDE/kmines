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
    Solver solver;

	int i, solved = 0;
	for(i=0;i<1000;++i){
        field.reset(W, H, M);

	    if( !solver.solveOneStep(field)){
            cout << "OOPS!!" << endl;
            cout << field << endl;
	    } else ++solved;

	    cout << "Tried " << i+1 << ", solved " << solved << endl;
	}

	cout << "Solved total: " << solved << endl;

	return 0;
}
