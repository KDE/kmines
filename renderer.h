/*
    Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef RENDERER_H
#define RENDERER_H

#include <QPixmap>
#include "commondefs.h"

class KSvgRenderer;
class KPixmapCache;
/**
 * This class is responsible for rendering all the game graphics.
 * Graphics is rendered from svg file specified by current theme.
 * Only one instance of this class exists during a program run.
 * It can be accessed with static function KLinesRenderer::self().
 */
class KMinesRenderer
{
public:
    /**
     * Returns one and the only instance of KLinesRenderer
     */
    static KMinesRenderer* self();
    /**
     * Loads new theme. Resets cache and puts new flashy rerendered
     * pixmaps there
     * @param themeName specifies theme name which is the part of the
     * theme's file path relative to $KDEDIR/share/apps/kmines, for example
     * it might be "themes/default.desktop"
     */
    bool loadTheme( const QString& themeName );
    /**
     * Sets cell render size
     * Call to this function will reset the cache and put fresh pixmaps to it
     */
    void setCellSize( int size );
    /**
     * @return current cell render size
     */
    int cellSize() const { return m_cellSize; }
    /**
     * @return pixmap for background painting.
     */
    QPixmap backgroundPixmap(const QSize& size) const;
    /**
     * @return pixmap for corresponding element
     */
    QPixmap pixmapForCellState( KMinesState::CellState state ) const;
    /**
     * @return pixmap for item that holds a digit
     */
    QPixmap pixmapForDigitElement( int digit ) const;
    /**
     * @return pixmap for item with mine
     */
    QPixmap pixmapMine() const;
    /**
     * @return pixmap for item with exploded mine
     */
    QPixmap pixmapExplodedMine() const;
    /**
     * @return pixmap for border part
     */
    QPixmap pixmapForBorderElement(KMinesState::BorderElement e) const;
private:
    // disable copy - it's singleton
    KMinesRenderer();
    KMinesRenderer( const KMinesRenderer& );
    KMinesRenderer& operator=( const KMinesRenderer& );
    ~KMinesRenderer();

    /**
     * Renderer used to render all graphics from svg file
     */
    KSvgRenderer *m_renderer;
    /**
     * Pixmap cache
     */
    KPixmapCache *m_cache;
    /**
     * Current cell render size
     */
    int m_cellSize;
    /**
     * Elements from svg file
     */
    enum SvgElement
    {
        CellUp=0,
        CellDown,
        Flag,
        Question,
        Digit1,
        Digit2,
        Digit3,
        Digit4,
        Digit5,
        Digit6,
        Digit7,
        Digit8,
        Mine,
        Explosion,
        ExplodedMine, // for convenience - doesn't exist in svg.
        Error,
        Hint,
        BorderEdgeNorth,
        BorderEdgeSouth,
        BorderEdgeEast,
        BorderEdgeWest,
        BorderOutsideCornerNE,
        BorderOutsideCornerNW,
        BorderOutsideCornerSW,
        BorderOutsideCornerSE,
        NumElements
    };
    /**
     * Translates enum value to QString name used in svg file.
     * Names (in .cpp) should be in sync with those defined in svg files
     */
    QString elementToSvgId( SvgElement e ) const;
    /**
     * Name of currently loaded theme
     */
    QString m_currentTheme;
};

#endif
