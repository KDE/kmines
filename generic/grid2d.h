#ifndef __GRID2D_H
#define __GRID2D_H

#include <set>
#include <math.h>

#include <qvaluevector.h>

#include <kglobal.h>


//-----------------------------------------------------------------------------
namespace Grid2D
{
    typedef std::pair<int, int> Coord;
};

inline Grid2D::Coord
operator +(const Grid2D::Coord &c1, const Grid2D::Coord &c2) {
    return Grid2D::Coord(c1.first + c2.first, c1.second + c2.second);
}
inline Grid2D::Coord
operator -(const Grid2D::Coord &c1, const Grid2D::Coord &c2) {
    return Grid2D::Coord(c1.first - c2.first, c1.second - c2.second);
}

inline Grid2D::Coord
maximum(const Grid2D::Coord &c1, const Grid2D::Coord &c2) {
    return Grid2D::Coord(kMax(c1.first, c2.first), kMax(c1.second, c2.second));
}
inline Grid2D::Coord
minimum(const Grid2D::Coord &c1, const Grid2D::Coord &c2) {
    return Grid2D::Coord(kMin(c1.first, c2.first), kMin(c1.second, c2.second));
}

inline QTextStream &operator <<(QTextStream &s, const Grid2D::Coord &c) {
    return s << '(' << c.second << ", " << c.first << ')';
}

inline QDataStream &operator <<(QDataStream &s, const Grid2D::Coord &c) {
    return s << Q_UINT32(c.first) << Q_UINT32(c.second);
}

inline QDataStream &operator >>(QDataStream &s, Grid2D::Coord &c) {
    Q_UINT32 first, second;
    s >> first >> second;
    c.first = first;
    c.second = second;
    return s;
}

//-----------------------------------------------------------------------------
namespace Grid2D
{
    typedef std::set<Coord, std::less<Coord> > CoordSet;
};

inline QTextStream &operator <<(QTextStream &s, const Grid2D::CoordSet &set) {
    for(Grid2D::CoordSet::iterator i=set.begin(); i!=set.end(); ++i)
        s << *i;
	return s;
}

inline QDataStream &operator <<(QDataStream &s, const Grid2D::CoordSet &set) {
    s << Q_UINT32(set.size());
    for(Grid2D::CoordSet::iterator i=set.begin(); i!=set.end(); ++i)
        s << *i;
    return s;
}

inline QDataStream &operator >>(QDataStream &s, Grid2D::CoordSet &set) {
    set.clear();
    Q_UINT32 nb;
    for (Q_UINT32 i=0; i<nb; i++) {
        Grid2D::Coord c;
        s >> c;
        set.insert(c);
    }
    return s;
}

//-----------------------------------------------------------------------------
namespace Grid2D
{
/**
 * This class represents a generic bidimensionnal grid.
 */
template <class Type>
class Generic
{
 public:
    Generic(uint width = 0, uint height = 0) {
        resize(width, height);
    }

    virtual ~Generic() {}

    void resize(uint width, uint height) {
        _width = width;
        _height = height;
        _vector.resize(width*height);
    }

    void fill(const Type &value) {
        std::fill(_vector.begin(), _vector.end(), value);
    }

    uint width() const  { return _width; }
    uint height() const { return _height; }
    uint size() const   { return _width*_height; }

    uint index(const Coord &c) const {
        return c.first + c.second*_width;
    }

    Coord coord(uint index) const {
        return Coord(index % _width, index / _width);
    }

    const Type &at(const Coord &c) const { return _vector[index(c)]; }
    Type &at(const Coord &c)             { return _vector[index(c)]; }
    const Type &operator [](const Coord &c) const { return _vector[index(c)]; }
    Type &operator [](const Coord &c)             { return _vector[index(c)]; }

    const Type &at(uint index) const          { return _vector[index]; }
    Type &at(uint index)                      { return _vector[index]; }
    const Type &operator [](uint index) const { return _vector[index]; }
    Type &operator [](uint index)             { return _vector[index]; }

    bool inside(const Coord &c) const {
        return ( c.first>=0 && c.first<(int)_width
                 && c.second>=0 && c.second<(int)_height );
    }

    void bound(Coord &c) const {
        c.first = kMax(kMin(c.first, (int)_width-1), 0);
        c.second = kMax(kMin(c.second, (int)_height-1), 0);
    }

    virtual void neighbours(const Coord &c, CoordSet &neighbours,
                   bool insideOnly = true, bool directOnly = false) const = 0;

 private:
    uint _width, _height;
    QValueVector<Type> _vector;
};
};

template <class Type>
QDataStream &operator <<(QDataStream &s, const Grid2D::Generic<Type> &m) {
    s << (Q_UINT32)m.width() << (Q_UINT32)m.height();
    for (uint i=0; i<m.size(); i++) s << m[i];
    return s;
}

template <class Type>
QDataStream &operator >>(QDataStream &s, Grid2D::Generic<Type> &m) {
    Q_UINT32 w, h;
    s >> w >> h;
    m.resize(w, h);
    for (uint i=0; i<m.size(); i++) s >> m[i];
    return s;
}


namespace Grid2D
{

//-----------------------------------------------------------------------------
class SquareBase
{
 public:
    enum Neighbour { Left=0, Right, Up, Down, LeftUp, LeftDown,
                     RightUp, RightDown, Nb_Neighbour };
    /**
     * @return the trigonometric angle in  radians.
     */
    static double angle(Neighbour n) {
        switch (n) {
        case Left:      return M_PI;
        case Right:     return 0;
        case Up:        return M_PI_2;
        case Down:      return -M_PI_2;
        case LeftUp:    return 3.0*M_PI_4;
        case LeftDown:  return -3.0*M_PI_4;
        case RightUp:   return M_PI_4;
        case RightDown: return -M_PI_4;
        }
        return 0;
    }

    static Neighbour opposed(Neighbour n) {
        switch (n) {
        case Left:      return Right;
        case Right:     return Left;
        case Up:        return Down;
        case Down:      return Up;
        case LeftUp:    return RightDown;
        case LeftDown:  return RightUp;
        case RightUp:   return LeftDown;
        case RightDown: return LeftUp;
        }
        return Nb_Neighbour;
    }

    static bool isDirect(Neighbour n) { return n<LeftUp; }

    static Coord neighbour(const Coord &c, Neighbour n) {
        switch (n) {
        case Left:      return c + Coord(-1,  0);
        case Right:     return c + Coord( 1,  0);
        case Up:        return c + Coord( 0, -1);
        case Down:      return c + Coord( 0,  1);
        case LeftUp:    return c + Coord(-1, -1);
        case LeftDown:  return c + Coord(-1,  1);
        case RightUp:   return c + Coord( 1, -1);
        case RightDown: return c + Coord( 1,  1);
        }
        return c;
    }
};

template <class Type>
class Square : public Generic<Type>, public SquareBase
{
 public:
    Square(uint width = 0, uint height = 0)
        : Generic<Type>(width, height) {}

    void neighbours(const Coord &c, CoordSet &neighbours,
                    bool insideOnly = true, bool directOnly = false) const {
        for (uint i=0; i<(directOnly ? LeftUp : Nb_Neighbour); i++) {
            Coord n = neighbour(c, (Neighbour)i);
            if ( insideOnly && !inside(n) ) continue;
            neighbours.insert(n);
        }
    }

    Coord toEdge(const Coord &c, Neighbour n) const {
        switch (n) {
        case Left:      return Coord(0, c.second);
        case Right:     return Coord(width()-1, c.second);
        case Up:        return Coord(c.first, 0);
        case Down:      return Coord(c.first, height()-1);
        case LeftUp:    return Coord(0, 0);
        case LeftDown:  return Coord(0, height()-1);
        case RightUp:   return Coord(width()-1, 0);
        case RightDown: return Coord(width()-1, height()-1);
        }
        return c;
    }
};

//-----------------------------------------------------------------------------
/**
 * This class represents an hexagonal grid where hexagons form horizontal
 * lines :
 * <pre>
 * (0,0)   (0,1)   (0,2)
 *     (1,0)   (1,1)   (1,2)
 * (2,0)   (2,1)   (2,2)
 * </pre>
 */
class HexagonalBase
{
 public:
     enum Neighbour { Left = 0, Right, LeftUp, LeftDown,
                     RightUp, RightDown, Nb_Neighbour };

     /**
     * @return the trigonometric angle in radians.
     */
    static double angle(Neighbour n) {
        switch (n) {
        case Left:      return M_PI;
        case Right:     return 0;
        case LeftUp:    return 2.0*M_PI/3;
        case LeftDown:  return -2.0*M_PI/3;
        case RightUp:   return M_PI/3;
        case RightDown: return -M_PI/3;
        }
        return 0;
    }

    static Neighbour opposed(Neighbour n) {
        switch (n) {
        case Left:      return Right;
        case Right:     return Left;
        case LeftUp:    return RightDown;
        case LeftDown:  return RightUp;
        case RightUp:   return LeftDown;
        case RightDown: return LeftUp;
        }
        return Nb_Neighbour;
    }

    static Coord neighbour(const Coord &c, Neighbour n) {
        bool oddRow = c.second%2;
        switch (n) {
        case Left:      return c + Coord(-1,  0);
        case Right:     return c + Coord( 1,  0);
        case LeftUp:    return c + (oddRow ? Coord( 0, -1) : Coord(-1, -1));
        case LeftDown:  return c + (oddRow ? Coord( 0,  1) : Coord(-1,  1));
        case RightUp:   return c + (oddRow ? Coord( 1, -1) : Coord( 0, -1));
        case RightDown: return c + (oddRow ? Coord( 1,  1) : Coord( 0,  1));
        }
        return c;
    }
};

template <class Type>
class Hexagonal : public Generic<Type>, public HexagonalBase
{
 public:
    Hexagonal(uint width = 0, uint height = 0)
        : Generic<Type>(width, height) {}

    /**
     * @param directOnly means nothing for hexagonal grid
     */
    void neighbours(const Coord &c, CoordSet &neighbours,
                    bool insideOnly = true, bool = false) const {
        for (uint i=0; i<Nb_Neighbour; i++) {
            Coord n = neighbour(c, (Neighbour)i);
            if ( insideOnly && !inside(n) ) continue;
            neighbours.insert(n);
        }
    }
};

}; // namespace

#endif
