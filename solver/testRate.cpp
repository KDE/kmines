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
    Solver solver;

	int i, solved = 0;
	for(i=0;i<1000;++i){
        field.reset(W, H, M);

	    if( !solver.solveOneStep(field)){
            out << "OOPS!!" << endl;
            out << field << endl;
	    } else ++solved;

	    out << "Tried " << i+1 << ", solved " << solved << endl;
	}

	out << "Solved total: " << solved << endl;

	return 0;
}
