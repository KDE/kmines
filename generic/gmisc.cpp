/*
    This file is part of the KDE games library
    Copyright (C) 2002 Nicolas Hadacek (hadacek@kde.org)

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

#include "gmisc.h"
#include "gmisc.moc"

#include <qlayout.h>

#include "ghighscores.h"


//-----------------------------------------------------------------------------
class ConfigItem : public KConfigItemBase
{
 public:
    ConfigItem(KExtHighscores::ConfigWidget &cw)
        : _cw(cw) {
        connect(&cw, SIGNAL(modified()), SLOT(modifiedSlot()));
    }

 protected:
    void loadState() { _cw.load(); }
    bool saveState() { return _cw.save(); }
    void setDefaultState() {}
    bool hasDefault() const { return true; }

 private:
    KExtHighscores::ConfigWidget &_cw;
};

//-----------------------------------------------------------------------------
HighscoresConfigWidget::HighscoresConfigWidget()
{
    QVBoxLayout *top = new QVBoxLayout(this);
    KExtHighscores::ConfigWidget *cw =
        KExtHighscores::createConfigWidget(this);
    top->addWidget(cw);

    configCollection()->insert( new ConfigItem(*cw) );

    setTitle(cw->title());
    setIcon(cw->icon());
}
