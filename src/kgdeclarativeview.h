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

#ifndef KGDECLARATIVEVIEW_H
#define KGDECLARATIVEVIEW_H

#include <QDeclarativeView>

class KGameRenderer;

class KgDeclarativeView : public QDeclarativeView
{
    Q_OBJECT
public:
    KgDeclarativeView(KGameRenderer *renderer=0, QWidget *parent=0);
    void setupRenderer(KGameRenderer *renderer);

};

#endif //KGDECLARATIVEVIEW_H
