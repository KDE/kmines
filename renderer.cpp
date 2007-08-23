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

#include "renderer.h"

#include <QPainter>

#include <KSvgRenderer>
#include <KGameTheme>
#include <kpixmapcache.h>

#include "settings.h"

QString KMinesRenderer::elementToSvgId( SvgElement e ) const
{
    switch(e)
    {
        case KMinesRenderer::CellUp:
            return "cell_up";
        case KMinesRenderer::CellDown:
            return "cell_down";
        case KMinesRenderer::Flag:
            return "flag";
        case KMinesRenderer::Question:
            return "question";
        case KMinesRenderer::Digit1:
            return "arabicOne";
        case KMinesRenderer::Digit2:
            return "arabicTwo";
        case KMinesRenderer::Digit3:
            return "arabicThree";
        case KMinesRenderer::Digit4:
            return "arabicFour";
        case KMinesRenderer::Digit5:
            return "arabicFive";
        case KMinesRenderer::Digit6:
            return "arabicSix";
        case KMinesRenderer::Digit7:
            return "arabicSeven";
        case KMinesRenderer::Digit8:
            return "arabicEight";
        case KMinesRenderer::Mine:
            return "mine";
        case KMinesRenderer::ExplodedMine:
            return QString(); // dummy. shouldn't be called
        case KMinesRenderer::Explosion:
            return "explosion";
        case KMinesRenderer::Error:
            return "error";
        case KMinesRenderer::Hint:
            return "hint";
        case KMinesRenderer::BorderEdgeNorth:
            return "border.edge.north";
        case KMinesRenderer::BorderEdgeSouth:
            return "border.edge.south";
        case KMinesRenderer::BorderEdgeEast:
            return "border.edge.east";
        case KMinesRenderer::BorderEdgeWest:
            return "border.edge.west";
        case KMinesRenderer::BorderOutsideCornerNE:
            return "border.outsideCorner.ne";
        case KMinesRenderer::BorderOutsideCornerNW:
            return "border.outsideCorner.nw";
        case KMinesRenderer::BorderOutsideCornerSW:
            return "border.outsideCorner.sw";
        case KMinesRenderer::BorderOutsideCornerSE:
            return "border.outsideCorner.se";
        case KMinesRenderer::NumElements:
            return QString();
    }
    return QString();
}

KMinesRenderer* KMinesRenderer::self()
{
    static KMinesRenderer instance;
    return &instance;
}

KMinesRenderer::KMinesRenderer()
    : m_cellSize(0)
{
    m_renderer = new KSvgRenderer();
    m_cache = new KPixmapCache("kmines-cache");
    m_cache->setCacheLimit(3*1024);

    if(!loadTheme( Settings::theme() ))
        kDebug() << "Failed to load any game theme!";
}

bool KMinesRenderer::loadTheme( const QString& themeName )
{
    // variable saying whether to discard old cache upon successful new theme loading
    // we won't discard it if m_currentTheme is empty meaning that
    // this is the first time loadTheme() is called
    // (i.e. during startup) as we want to pick the cache from disc
    bool discardCache = !m_currentTheme.isEmpty();

    if( !m_currentTheme.isEmpty() && m_currentTheme == themeName )
    {
        kDebug() << "Notice: not loading the same theme";
        return true; // this is not an error
    }
    KGameTheme theme;
    if ( !theme.load( themeName ) )
    {
        kDebug()<< "Failed to load theme" << Settings::theme();
        kDebug() << "Trying to load default";
        if(!theme.loadDefault())
            return false;
    }

    m_currentTheme = themeName;

    bool res = m_renderer->load( theme.graphics() );
    kDebug() << "loading" << theme.graphics();
    if ( !res )
        return false;

    if(discardCache)
    {
        kDebug() << "discarding cache";
        m_cache->discard();
    }
    return true;
}

QPixmap KMinesRenderer::backgroundPixmap( const QSize& size ) const
{
    QPixmap bkgnd;
    QString cacheName = QString("mainWidget%1x%2").arg(size.width()).arg(size.height());
    if(!m_cache->find( cacheName, bkgnd ))
    {
        kDebug() << "re-rendering bkgnd";
        bkgnd = QPixmap(size);
        bkgnd.fill(Qt::transparent);
        QPainter p(&bkgnd);
        m_renderer->render(&p, "mainWidget");
        m_cache->insert(cacheName, bkgnd);
        kDebug() << "cache size:" << m_cache->size() << "kb";
    }
    return bkgnd;
}

KMinesRenderer::~KMinesRenderer()
{
    delete m_renderer;
    delete m_cache;
}

#define RENDER_SVG_ELEMENT(SVG_ID)                      \
    p.begin( &pix );                                    \
    m_renderer->render( &p, elementToSvgId(SVG_ID) );   \
    p.end();

QPixmap KMinesRenderer::pixmapForCellState( KMinesState::CellState state ) const
{
    QPainter p;
    switch(state)
    {
        case KMinesState::Released:
        {
            QPixmap pix;
            QString cacheName = elementToSvgId(CellUp)+QString::number(m_cellSize);
            if(!m_cache->find(cacheName, pix))
            {
//                kDebug() << "putting" << cacheName << "to cache";
                pix = QPixmap(m_cellSize, m_cellSize);
                pix.fill( Qt::transparent);
                RENDER_SVG_ELEMENT(CellUp);
                m_cache->insert(cacheName, pix);
            }
            return pix;
        }
        case KMinesState::Pressed:
        case KMinesState::Revealed:// i.e. revealed & digit=0 case
        {
            QPixmap pix;
            QString cacheName = elementToSvgId(CellDown)+QString::number(m_cellSize);
            if(!m_cache->find(cacheName, pix))
            {
//                kDebug() << "putting" << cacheName << "to cache";
                pix = QPixmap(m_cellSize, m_cellSize);
                pix.fill( Qt::transparent);
                RENDER_SVG_ELEMENT(CellDown);
                m_cache->insert(cacheName, pix);
            }
            return pix;
        }
        case KMinesState::Questioned:
        {
            QPixmap pix;
            QString cacheName = elementToSvgId(Question)+QString::number(m_cellSize);
            if(!m_cache->find(cacheName, pix))
            {
//                kDebug() << "putting" << cacheName << "to cache";
                // question (on top of cellup)
                pix = pixmapForCellState( KMinesState::Released );
                RENDER_SVG_ELEMENT(Question);
                m_cache->insert(cacheName, pix);
            }
            return pix;
        }
        case KMinesState::Flagged:
        {
            QPixmap pix;
            QString cacheName = elementToSvgId(Flag)+QString::number(m_cellSize);
            if(!m_cache->find(cacheName, pix))
            {
                // flag (on top of cellup)
//                kDebug() << "putting" << cacheName << "to cache";
                pix = pixmapForCellState( KMinesState::Released );
                RENDER_SVG_ELEMENT(Flag);
                m_cache->insert(cacheName, pix);
            }
            return pix;
        }
        case KMinesState::Error:
        {
            QPixmap pix;
            QString cacheName = elementToSvgId(Error)+QString::number(m_cellSize);
            if(!m_cache->find(cacheName, pix))
            {
//                kDebug() << "putting" << cacheName << "to cache";
                // flag (on top of mine)
                pix = pixmapMine();
                RENDER_SVG_ELEMENT(Error);
                m_cache->insert(cacheName, pix);
            }
            return pix;
        }
        case KMinesState::Hint:
        {
            QPixmap pix;
            QString cacheName = elementToSvgId(Hint)+QString::number(m_cellSize);
            if(!m_cache->find(cacheName, pix))
            {
//                kDebug() << "putting" << cacheName << "to cache";
                // hint (on top of cellup)
                pix = pixmapForCellState( KMinesState::Released );
                RENDER_SVG_ELEMENT(Hint);
                m_cache->insert(cacheName, pix);
            }
            return pix;
        }
        // no default! this way we'll get compiler warnings if
        // something is forgotten
    }
    return QPixmap();
}

QPixmap KMinesRenderer::pixmapForDigitElement( int digit ) const
{
    KMinesRenderer::SvgElement e;
    if(digit == 1)
        e = KMinesRenderer::Digit1;
    else if(digit == 2)
        e = KMinesRenderer::Digit2;
    else if(digit == 3)
        e = KMinesRenderer::Digit3;
    else if(digit == 4)
        e = KMinesRenderer::Digit4;
    else if(digit == 5)
        e = KMinesRenderer::Digit5;
    else if(digit == 6)
        e = KMinesRenderer::Digit6;
    else if(digit == 7)
        e = KMinesRenderer::Digit7;
    else if(digit == 8)
        e = KMinesRenderer::Digit8;
    else
        return QPixmap();

    QPainter p;
    QPixmap pix;
    QString cacheName = elementToSvgId(e)+QString::number(m_cellSize);
    if(!m_cache->find(cacheName, pix))
    {
//        kDebug() << "putting" << cacheName << "to cache";
        // digit (on top of celldown)
        pix = pixmapForCellState( KMinesState::Pressed );
        RENDER_SVG_ELEMENT(e);
        m_cache->insert(cacheName, pix);
    }
    return pix;
}

QPixmap KMinesRenderer::pixmapMine() const
{
    QPainter p;
    QPixmap pix;
    QString cacheName = elementToSvgId(Mine)+QString::number(m_cellSize);
    if(!m_cache->find(cacheName, pix))
    {
//        kDebug() << "putting" << cacheName << "to cache";
        // mine (on top of celldown)
        pix = pixmapForCellState( KMinesState::Pressed );
        RENDER_SVG_ELEMENT(Mine);
        m_cache->insert(cacheName, pix);
    }
    return pix;
}

QPixmap KMinesRenderer::pixmapExplodedMine() const
{
    QPainter p;
    QPixmap pix;
    QString cacheName = elementToSvgId(Explosion)+QString::number(m_cellSize);
    if(!m_cache->find(cacheName, pix))
    {
//        kDebug() << "putting" << cacheName << "to cache";
        // mine (on top of celldown)
        pix = pixmapForCellState( KMinesState::Pressed );
        RENDER_SVG_ELEMENT(Explosion);
        RENDER_SVG_ELEMENT(Mine);
        m_cache->insert(cacheName, pix);
    }
    return pix;
}

QPixmap KMinesRenderer::pixmapForBorderElement(KMinesState::BorderElement e) const
{
    SvgElement svgel = NumElements; // invalid
    switch(e)
    {
        case KMinesState::BorderNorth:
            svgel = BorderEdgeNorth;
            break;
        case KMinesState::BorderSouth:
            svgel = BorderEdgeSouth;
            break;
        case KMinesState::BorderEast:
            svgel = BorderEdgeEast;
            break;
        case KMinesState::BorderWest:
            svgel = BorderEdgeWest;
            break;
        case KMinesState::BorderCornerNW:
            svgel = BorderOutsideCornerNW;
            break;
        case KMinesState::BorderCornerSW:
            svgel = BorderOutsideCornerSW;
            break;
        case KMinesState::BorderCornerNE:
            svgel = BorderOutsideCornerNE;
            break;
        case KMinesState::BorderCornerSE:
            svgel = BorderOutsideCornerSE;
            break;
    }

    QPainter p;
    QPixmap pix;
    QString cacheName = elementToSvgId(svgel)+QString::number(m_cellSize);
    if(!m_cache->find(cacheName, pix))
    {
//        kDebug() << "putting" << cacheName << "to cache";
        pix = QPixmap(m_cellSize, m_cellSize);
        pix.fill( Qt::transparent);
        RENDER_SVG_ELEMENT(svgel);
        m_cache->insert(cacheName, pix);
    }
    return pix;
}

void KMinesRenderer::setCellSize( int size )
{
    m_cellSize = size;
}
