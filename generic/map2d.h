#ifndef MAP2D_H
#define MAP2D_H

#include <utility>
#include <set>
#include <vector>


namespace Map2D
{
    typedef pair<int, int> Coord;
    Coord operator +(const Coord &, const Coord &);
    ostream &operator <<(ostream &, const Coord &);

    typedef set<Coord, less<Coord> > CoordSet;
    ostream &operator <<(ostream &, const CoordSet &);

    enum Direction { Left = 1, Right = 2, Down = 4, Up = 8 };

    template <class Type>
    class Matrix
    {

    public:
        Matrix() : _width(0), _height(0) {}
        Matrix(uint width, uint height) { resize(width, height); }

        void resize(uint width, uint height, const Type &value = Type()) {
            _width = width;
            _height = height;
            _vector.resize(width*height);
            fill(_vector.begin(), _vector.end(), value);
        }

        uint width() const { return _width; }
        uint height() const { return _height; }

        const Type &operator [](const Coord &c) const
            { return _vector[c.first + c.second*_width]; }
        Type &operator [](const Coord &c)
            { return _vector[c.first + c.second*_width]; }

        Coord at(uint index) const {
            assert( index < _vector.size() );
            return Coord(index % _width, index / _width);
        }

        bool inside(const Coord &c) const {
            return ( c.first>=0 && c.first<(int)_width
                     && c.second>=0 && c.second<(int)_height );
        }

        Coord neighbour(const Coord &c, Direction dir) const {
            switch (dir) {
            case Left:  return _neighbour(c, 0);
            case Right: return _neighbour(c, 1);
            case Up:    return _neighbour(c, 2);
            case Down:  return _neighbour(c, 3);
            }
            return Coord();
        }

        void neighbours(const Coord &c, CoordSet &neighbours,
                      bool insideOnly = true, bool closestOnly = false) const {
             for (uint i=0; i<(closestOnly ? 4 : 8); i++) {
                 Coord n = _neighbour(c, i);
                 if ( insideOnly && !inside(n) ) continue;
                 neighbours.insert(n);
             }
        }

    private:
        uint         _width, _height;
        vector<Type> _vector;

        Coord _neighbour(const Coord &c, uint i) const {
            assert( i<8 );
            switch (i) {
            case 0: return c + Coord(-1,  0);
            case 1: return c + Coord( 1,  0);
            case 2: return c + Coord( 0, -1);
            case 3: return c + Coord( 0,  1);
            case 4: return c + Coord(-1, -1);
            case 5: return c + Coord(-1,  1);
            case 6: return c + Coord( 1, -1);
            case 7: return c + Coord( 1,  1);
            }
            return Coord();
        }
    };
};

#endif








