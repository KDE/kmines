/** A program to test advisory library */

#include "bfield.h"
#include "solver.h"

int main(int argc, char *argv[]){

	assert(argc >= 4);

	long seed = time(0);
    QTextOStream out(stdout);
    out << "seed = " << seed << endl;

	short W, H, M;
	W = atoi(argv[1]); assert(W > 0);
	H = atoi(argv[2]); assert(H > 0);
	M = atoi(argv[3]); assert(M >= 0); // ;)

    BaseField field(seed);
    field.reset(W, H, M);

    Solver solver;
    if( !solver.solveOneStep(field) ) out << "OOPS!!" << endl;
    else out << "Solved!" << endl;

    out << field << endl;

	return 0;
}
