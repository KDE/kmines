/*
    This file is part of the KDE games library
    Copyright (C) 2001-02 Nicolas Hadacek (hadacek@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KGRID2D_H_
#define __KGRID2D_H_

#include <math.h>

#include <qpair.h>
#include <qvaluelist.h>
#include <qvaluevector.h>

#include <kglobal.h>


//-----------------------------------------------------------------------------
namespace KGrid2D
{
    /**
     * This type represents coordinates on a bidimensionnal grid.
     */
    typedef QPair<int, int> Coord;

    /**
     * This type represents a list of @ref Coord.
     */
    typedef QValueList<Coord> CoordList;
};

inline KGrid2D::Coord
operator +(const KGrid2D::Coord &c1, const KGrid2D::Coord &c2) {
    return KGrid2D::Coord(c1.first + c2.first, c1.second + c2.second);
}

inline KGrid2D::Coord
operator -(const KGrid2D::Coord &c1, const KGrid2D::Coord &c2) {
    return KGrid2D::Coord(c1.first - c2.first, c1.second - c2.second);
}

/**
 * @return the maximum of both coordinates.
 */
inline KGrid2D::Coord
maximum(const KGrid2D::Coord &c1, const KGrid2D::Coord &c2) {
    return KGrid2D::Coord(kMax(c1.first, c2.first), kMax(c1.second, c2.second));
}
/**
 * @return the minimum of both coordinates.
 */
inline KGrid2D::Coord
minimum(const KGrid2D::Coord &c1, const KGrid2D::Coord &c2) {
    return KGrid2D::Coord(kMin(c1.first, c2.first), kMin(c1.second, c2.second));
}

inline QTextStream &operator <<(QTextStream &s, const KGrid2D::Coord &c) {
    return s << '(' << c.second << ", " << c.first << ')';
}

inline QTextStream &operator <<(QTextStream &s, const KGrid2D::CoordList &list)
{
    for(KGrid2D::CoordList::const_iterator i=list.begin(); i!=list.end(); ++i)
        s << *i;
	return s;
}

//-----------------------------------------------------------------------------
namespace KGrid2D
{
/**
 * This template class represents a generic bidimensionnal grid. Each node
 * contains an element of the template type.
 */
template <class Type>
class Generic
{
 public:
    /**
     * Constructor.
     */
    Generic(uint width = 0, uint height = 0) {
        resize(width, height);
    }

    virtual ~Generic() {}

    /**
     * Resize the grid.
     */
    void resize(uint width, uint height) {
        _width = width;
        _height = height;
        _vector.resize(width*height);
    }

    /**
     * Fill the nodes with the given value.
     */
    void fill(const Type &value) {
        for (uint i=0; i<_vector.count(); i++) _vector[i] = value;
    }

    /**
     * @return the width.
     */
    uint width() const  { return _width; }
    /**
     * @return the height.
     */
    uint height() const { return _height; }
    /**
     * @return the number of nodes (ie width*height).
     */
    uint size() const   { return _width*_height; }

    /**
     * @return the linear index for the given coordinate.
     */
    uint index(const Coord &c) const {
        return c.first + c.second*_width;
    }

    /**
     * @return the coordinate corresponding to the linear index.
     */
    Coord coord(uint index) const {
        return Coord(index % _width, index / _width);
    }

    /**
     * @return the value at the given coordinate.
     */
    const Type &at(const Coord &c) const { return _vector[index(c)]; }
    /**
     * @return the value at the given coordinate.
     */
    Type &at(const Coord &c)             { return _vector[index(c)]; }
    /**
     * @return the value at the given coordinate.
     */
    const Type &operator [](const Coord &c) const { return _vector[index(c)]; }
    /**
     * @return the value at the given coordinate.
     */
    Type &operator [](const Coord &c)             { return _vector[index(c)]; }

    /**
     * @return the value at the given linear index.
     */
    const Type &at(uint index) const          { return _vector[index]; }
    /**
     * @return the value at the given linear index.
     */
    Type &at(uint index)                      { return _vector[index]; }
    /**
     * @return the value at the given linear index.
     */
    const Type &operator [](uint index) const { return _vector[index]; }
    /**
     * @return the value at the given linear index.
     */
    Type &operator [](uint index)             { return _vector[index]; }

    /**
     * @return if the given coordinate is inside the grid.
     */
    bool inside(const Coord &c) const {
        return ( c.first>=0 && c.first<(int)_width
                 && c.second>=0 && c.second<(int)_height );
    }

    /**
     * Bound the given coordinate with the grid dimensions.
     */
    void bound(Coord &c) const {
        c.first = kMax(kMin(c.first, (int)_width-1), 0);
        c.second = kMax(kMin(c.second, (int)_height-1), 0);
    }

 protected:
    uint               _width, _height;
    QValueVector<Type> _vector;
};
};

template <class Type>
QDataStream &operator <<(QDataStream &s, const KGrid2D::Generic<Type> &m) {
    s << (Q_UINT32)m.width() << (Q_UINT32)m.height();
    for (uint i=0; i<m.size(); i++) s << m[i];
    return s;
}

template <class Type>
QDataStream &operator >>(QDataStream &s, KGrid2D::Generic<Type> &m) {
    Q_UINT32 w, h;
    s >> w >> h;
    m.resize(w, h);
    for (uint i=0; i<m.size(); i++) s >> m[i];
    return s;
}


namespace KGrid2D
{

//-----------------------------------------------------------------------------
/**
 * This class contains static methods to manipulate coordinates for a
 * square bidimensionnal grid.
 */
class SquareBase
{
 public:
    /**
     * Identify the eight neighbours.
     */
    enum Neighbour { Left=0, Right, Up, Down, LeftUp, LeftDown,
                     RightUp, RightDown, Nb_Neighbour };

    /**
     * @return the trigonometric angle in radians for the given neighbour.
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
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return 0;
    }

    /**
     * @return the opposed neighbour.
     */
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
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return Nb_Neighbour;
    }

    /**
     * @return true if the neighbour is a direct one (ie is one of the four
     * nearest).
     */
    static bool isDirect(Neighbour n) { return n<LeftUp; }

    /**
     * @return the neighbour for the given coordinate.
     */
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
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return c;
    }
};

/**
 * This template is a @ref Generic implementation for a square bidimensionnal
 * grid (@ref SquareBase).
 */
template <class Type>
class Square : public Generic<Type>, public SquareBase
{
 public:
    /**
     * Constructor.
     */
    Square(uint width = 0, uint height = 0)
        : Generic<Type>(width, height) {}

    /**
     * @return the neighbours of coordinate @param c
     * to the given set of coordinates @param neighbours.
     *
     * @param insideOnly only add coordinates that are inside the grid.
     * @param directOnly only add the four nearest neighbours.
     */
    CoordList neighbours(const Coord &c, bool insideOnly = true,
                         bool directOnly = false) const {
        CoordList neighbours;
        for (uint i=0; i<(directOnly ? LeftUp : Nb_Neighbour); i++) {
            Coord n = neighbour(c, (Neighbour)i);
            if ( insideOnly && !inside(n) ) continue;
            neighbours.append(n);
        }
        return neighbours;
    }

    /**
     * @return the "projection" of the given coordinate on the grid edges.
     *
     * @param n the direction of projection.
     */
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
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return c;
    }
};

//-----------------------------------------------------------------------------
/**
 * This class contains static methods to manipulate coordinates on an
 * hexagonal grid where hexagons form horizontal lines:
 * <pre>
 * (0,0)   (0,1)   (0,2)
 *     (1,0)   (1,1)   (1,2)
 * (2,0)   (2,1)   (2,2)
 * </pre>
 */
class HexagonalBase
{
 public:
    /**
     * Identify the six neighbours.
     */
    enum Neighbour { Left = 0, Right, LeftUp, LeftDown,
                     RightUp, RightDown, Nb_Neighbour };

     /**
     * @return the trigonometric angle in radians for the given neighbour.
     */
    static double angle(Neighbour n) {
        switch (n) {
        case Left:      return M_PI;
        case Right:     return 0;
        case LeftUp:    return 2.0*M_PI/3;
        case LeftDown:  return -2.0*M_PI/3;
        case RightUp:   return M_PI/3;
        case RightDown: return -M_PI/3;
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return 0;
    }

    /**
     * @return the opposed neighbour.
     */
    static Neighbour opposed(Neighbour n) {
        switch (n) {
        case Left:      return Right;
        case Right:     return Left;
        case LeftUp:    return RightDown;
        case LeftDown:  return RightUp;
        case RightUp:   return LeftDown;
        case RightDown: return LeftUp;
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return Nb_Neighbour;
    }

    /**
     * @return the neighbour of the given coordinate.
     */
    static Coord neighbour(const Coord &c, Neighbour n) {
        bool oddRow = c.second%2;
        switch (n) {
        case Left:      return c + Coord(-1,  0);
        case Right:     return c + Coord( 1,  0);
        case LeftUp:    return c + (oddRow ? Coord( 0, -1) : Coord(-1, -1));
        case LeftDown:  return c + (oddRow ? Coord( 0,  1) : Coord(-1,  1));
        case RightUp:   return c + (oddRow ? Coord( 1, -1) : Coord( 0, -1));
        case RightDown: return c + (oddRow ? Coord( 1,  1) : Coord( 0,  1));
        case Nb_Neighbour: Q_ASSERT(false);
        }
        return c;
    }

    /**
    * @return the distance between the two coordinates in term of hexagons.
    */
    static uint distance(const Coord &c1, const Coord &c2) {
        return kAbs(c1.first - c2.first) + kAbs(c1.second - c2.second)
            + (c1.first==c2.first || c1.second==c2.second ? 0 : -1);
    }
};

/**
 * This template implements @Generic for an hexagonal grid
 * where hexagons form horizontal lines:
 * <pre>
 * (0,0)   (0,1)   (0,2)
 *     (1,0)   (1,1)   (1,2)
 * (2,0)   (2,1)   (2,2)
 * </pre>
 */
template <class Type>
class Hexagonal : public Generic<Type>, public HexagonalBase
{
 public:
    /**
     * Constructor.
     */
    Hexagonal(uint width = 0, uint height = 0)
        : Generic<Type>(width, height) {}

    /**
     * @return the neighbours of coordinate @param c
     * to the given set of coordinates @param neighbours.
     *
     * @param insideOnly only add coordinates that are inside the grid.
     */
    CoordList neighbours(const Coord &c, bool insideOnly = true) const {
        CoordList neighbours;
        for (uint i=0; i<Nb_Neighbour; i++) {
            Coord n = neighbour(c, (Neighbour)i);
            if ( insideOnly && !inside(n) ) continue;
            neighbours.append(n);
        }
        return neighbours;
    }


    /**
     * @return the neighbours at distance @param distance of coordinate
     * @param c.
     *
     * @param distance distance to the neighbour (1 means at contact).
     * @param insideOnly only add coordinates that are inside the grid.
     * @param all returns all neighbours at distance equal and less than
     *        @param distance (the original coordinate is not included).
     */
    CoordList neighbours(const Coord &c, uint distance, bool all,
                        bool insideOnly = true) const {
        // brute force algorithm -- you're welcome to make it more efficient :)
        CoordList ring;
        if ( distance==0 ) return ring;
        ring = neighbours(c, insideOnly);
        if ( distance==1 ) return ring;
        CoordList center;
        center.append(c);
        for (uint i=1; i<distance; i++) {
            CoordList newRing;
            CoordList::const_iterator it;
            for (it=ring.begin(); it!=ring.end(); ++it) {
                CoordList n = neighbours(*it, insideOnly);
                CoordList::const_iterator it2;
                for (it2=n.begin(); it2!=n.end(); ++it2)
                    if ( center.find(*it2)==center.end()
                         && ring.find(*it2)==ring.end()
                         && newRing.find(*it2)==newRing.end() )
                        newRing.append(*it2);
                center.append(*it);
            }
            ring = newRing;
        }
        if ( !all ) return ring;
        CoordList::const_iterator it;
        for (it=ring.begin(); it!=ring.end(); ++it)
            center.append(*it);
        center.remove(c);
        return center;
    }
};

}; // namespace

#endif
