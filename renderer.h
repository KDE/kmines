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
#include <QHash>

class KSvgRenderer;
/**
 * This class is responsible for rendering all the game graphics.
 * Graphics is rendered from svg file specified by current theme.
 * Only one instance of this class exists during a program run.
 * It can be accessed with static function KLinesRenderer::self().
 */
class KMinesRenderer
{
public:
    enum SvgElement
    {
        CellUp=0,
        CellDown,
        NumElements
    };
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
    void setCellSize( int size ) { m_cellSize = size; rerenderPixmaps(); }
    int cellSize() const { return m_cellSize; }
    /**
     * @return pixmap for background painting.
     */
    QPixmap backgroundPixmap(const QSize& size) const;
    /**
     * @return pixmap for corresponding element
     */
    QPixmap pixmapForElement( SvgElement element ) const;
private:
    // disable copy - it's singleton
    KMinesRenderer();
    KMinesRenderer( const KMinesRenderer& );
    KMinesRenderer& operator=( const KMinesRenderer& );
    ~KMinesRenderer();

    /**
     * Rerenders all pixmaps according to m_cellSize and puts them
     * to m_pixHash
     */
    void rerenderPixmaps();
    /**
     * Cached background pixmap.
     */
    mutable QPixmap m_cachedBkgnd;
    KSvgRenderer *m_renderer;
    int m_cellSize;
    QHash<SvgElement, QPixmap> m_pixHash;
};

#endif
