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

#include "corebindingsplugin.h"

#include "canvasitem.h"

#include <QDeclarativeContext>
#include <KGameRenderer>

void CoreBindingsPlugin::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    QObject *property = engine->rootContext()->contextProperty("renderer").value<QObject*>();
    KGameRenderer *renderer = (KGameRenderer*) property;
    CanvasItem::setRenderer(renderer);
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.games.core"));

    qmlRegisterType<CanvasItem>(uri, 0, 1, "CanvasItem");
}

#include "corebindingsplugin.moc"
