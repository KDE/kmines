/*
    This file is part of the KDE games library
    Copyright (C) 2001 Nicolas Hadacek (hadacek@kde.org)

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

#include "gsettings.h"
#include "gsettings.moc"

#include <qlayout.h>

#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>


//-----------------------------------------------------------------------------
SettingsWidget::SettingsWidget(const QString &name,
                               const QString &icon)
    : _name(name), _icon(icon)
{}

//-----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *parent)
    : KDialogBase(IconList, i18n("Configure"), Ok|Cancel|Default, Cancel,
                  parent, "option_dialog", true, true)
{
    setIconListAllVisible(true);
}

void SettingsDialog::addModule(SettingsWidget *w)
{
    w->load();
    QFrame *page = addPage(w->name(), QString::null,
                           BarIcon(w->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);

    uint s = _widgets.size();
    _widgets.resize(s+1);
    _widgets.insert(s, w);
}

void SettingsDialog::slotDefault()
{
    int i = activePageIndex();
    _widgets[i]->defaults();
}

void SettingsDialog::accept()
{
    for (uint i=0; i<_widgets.size(); i++) {
        _widgets[i]->save();
        if ( !_widgets[i]->isSaved() ) return;
    }
    KDialogBase::accept();
}
