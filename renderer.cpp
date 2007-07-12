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

static QString elementToSvgId( KMinesRenderer::SvgElement e )
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

void KMinesRenderer::rerenderPixmaps()
{
    if(m_cellSize == 0)
        return;

    QPainter p;
    // cell up
    QPixmap pix( m_cellSize, m_cellSize );
    pix.fill( Qt::transparent );
    p.begin( &pix );
    m_renderer->render( &p, elementToSvgId(CellUp) );
    p.end();
    m_pixHash[CellUp] = pix;

    // cell down
    pix = QPixmap( m_cellSize, m_cellSize );
    pix.fill( Qt::transparent );
    p.begin( &pix );
    m_renderer->render( &p, elementToSvgId(CellDown) );
    p.end();
    m_pixHash[CellDown] = pix;

    // question (on top of cellup)
    pix = m_pixHash[CellUp];
    p.begin( &pix );
    m_renderer->render( &p, elementToSvgId(Question) );
    p.end();
    m_pixHash[Question] = pix;

    // flag (on top of cellup)
    pix = m_pixHash[CellUp];
    p.begin( &pix );
    m_renderer->render( &p, elementToSvgId(Flag) );
    p.end();
    m_pixHash[Flag] = pix;
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

QPixmap KMinesRenderer::pixmapForElement( SvgElement element ) const
{
    return m_pixHash[element];
}
