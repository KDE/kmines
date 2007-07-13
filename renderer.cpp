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
        case KMinesRenderer::Explosion:
            return "explosion";
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

    if(!loadTheme( Settings::theme() ))
        kDebug() << "Failed to load any game theme!" << endl;
}

bool KMinesRenderer::loadTheme( const QString& themeName )
{
    KGameTheme theme;
    if ( !theme.load( themeName ) )
    {
        kDebug()<< "Failed to load theme " << Settings::theme() << endl;
        kDebug() << "Trying to load default" << endl;
        if(!theme.loadDefault())
            return false;
    }

    bool res = m_renderer->load( theme.graphics() );
    kDebug() << "loading " << theme.graphics() << endl;
    if ( !res )
        return false;

    rerenderPixmaps();

    return true;
}

#define RENDER_SVG_ELEMENT(SVG_ID)                      \
    p.begin( &pix );                                    \
    m_renderer->render( &p, elementToSvgId(SVG_ID) );   \
    p.end();                                            \
    m_pixHash[SVG_ID] = pix;

void KMinesRenderer::rerenderPixmaps()
{
    if(m_cellSize == 0)
        return;

    QPainter p;
    // cell up
    QPixmap pix(m_cellSize, m_cellSize);
    pix.fill( Qt::transparent);
    RENDER_SVG_ELEMENT(CellDown);

    pix = QPixmap(m_cellSize, m_cellSize);
    pix.fill( Qt::transparent);
    RENDER_SVG_ELEMENT(CellUp);

    // all digits are rendered on top of CellDown
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit1);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit2);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit3);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit4);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit5);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit6);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit7);
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Digit8);

    // question (on top of cellup)
    pix = m_pixHash[CellUp];
    RENDER_SVG_ELEMENT(Question);

    // flag (on top of cellup)
    pix = m_pixHash[CellUp];
    RENDER_SVG_ELEMENT(Flag);

    // mine is a mix of celldown & explosion & mine
    pix = m_pixHash[CellDown];
    RENDER_SVG_ELEMENT(Explosion);
    RENDER_SVG_ELEMENT(Mine);
}

QPixmap KMinesRenderer::backgroundPixmap( const QSize& size ) const
{
    if( m_cachedBkgnd.isNull() || m_cachedBkgnd.size() != size )
    {
        kDebug() << "re-rendering bkgnd" << endl;
        m_cachedBkgnd = QPixmap(size);
        m_cachedBkgnd.fill(Qt::transparent);
        QPainter p(&m_cachedBkgnd);
        m_renderer->render(&p, "mainWidget");
    }

    return m_cachedBkgnd;
}

KMinesRenderer::~KMinesRenderer()
{
    delete m_renderer;
}

QPixmap KMinesRenderer::pixmapForCellState( KMinesState::CellState state ) const
{
    switch(state)
    {
        case KMinesState::Released:
            return m_pixHash[CellUp];
        case KMinesState::Pressed:
            return m_pixHash[CellDown];
        case KMinesState::Exploded:
            // pixmap is already mixed with celldown & explosion ones
            return m_pixHash[Mine];
        case KMinesState::Revealed:
            // i.e. revealed & digit=0 case
            return m_pixHash[CellDown];
        case KMinesState::Questioned:
            return m_pixHash[Question];
        case KMinesState::Flagged:
            return m_pixHash[Flag];
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

    return m_pixHash[e];
}
