#include "map2d.h"


using namespace Map2D;

//-----------------------------------------------------------------------------
Coord Map2D::operator +(const Coord &c1, const Coord &c2)
{
    Coord c;
    c.first = c1.first + c2.first;
    c.second = c1.second + c2.second;
    return c;
}

ostream &Map2D::operator <<(ostream &_, const Coord &c)
{
	return _ << '(' << c.second << ", " << c.first << ')';
}

ostream &Map2D::operator <<(ostream &_, const CoordSet &set)
{
	copy(set.begin(), set.end(), ostream_iterator<Coord>(_, " "));
	return _;
}
