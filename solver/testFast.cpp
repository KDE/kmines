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
