/*
    Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
  
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
   
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CANVASITEM_H
#define CANVASITEM_H

#include <QDeclarativeItem>
#include <KGameRenderer>
#include <KGameRendererClient>

class CanvasItem : public QDeclarativeItem, KGameRendererClient
{
    Q_OBJECT
    Q_PROPERTY(QString spriteKey READ spriteKey WRITE setSpriteKey NOTIFY spriteKeyChanged)
    Q_PROPERTY(bool valid READ isValid)

public:
    CanvasItem(QDeclarativeItem *parent = 0);

    static void setRenderer(KGameRenderer*);

    void setSpriteKey(const QString &spriteKey);

    bool isValid() const;

    void setImplicitSize();

    void receivePixmap(const QPixmap& pixmap);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0);

signals:
    void spriteKeyChanged();

private:
    static KGameRenderer *m_renderer;
    QPixmap m_pixmap;

    QSize boundingSize();

};

#endif
