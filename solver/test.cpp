/** A program to test advisory library */

#include "bfield.h"
#include "headerP.h"

#define W 10
#define H 10

int main(int argc, char *argv[])
{
    long seed = (argc<2 ? time(0) : atoi(argv[1]));
    cout << "seed = " << seed << endl;

	BaseField f(seed);
    f.reset(W, H, 10);

    KRandomSequence random(seed);
    Coord c(random.getLong(W), random.getLong(H));
    f.doReveal(c, 0, 0);

	CoordSet sp;
	AdviseFull::ProbabilityMap pm;

	AdviseFast::FactSet facts(&f);
	AdviseFull::adviseFull(&facts, &sp, &pm);

	float pic[H][W];

	for(uint i=0; i<H; ++i)
        for(uint j=0; j<W; ++j) pic[i][j] = -1; // unknown
    pic[c.second][c.first] = -(int)f.nbMinesAround(c);

	AdviseFull::ProbabilityMap::iterator pmi;
	for(pmi = pm.begin(); pmi != pm.end(); ++pmi)
		pic[pmi->second.second][pmi->second.first] = pmi->first;

    QString s;
	for(uint i=0;i<H;++i) {
		for(uint j=0;j<W;++j)
            cout << s.sprintf("%+.02f ", pic[i][j]).latin1();
        cout << endl;
	}

	return 0;
}
