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

#ifndef G_SETTINGS_H
#define G_SETTINGS_H

#include <qptrvector.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kconfig.h>
#include <kcmodule.h>


//-----------------------------------------------------------------------------
/**
 * Extends the @ref KCModule class.
 */
class SettingsWidget : public KCModule
{
 Q_OBJECT
 public:
    /**
     * Constructor.
     *
     * @param name the translated module name.
     * @param icon the name of the module icon.
     */
    SettingsWidget(const QString &name, const QString &icon);

    QString name() const { return _name; }
    QString icon() const { return _icon; }

    /**
     * @return true if the module has been saved. It is sometimes useful
     * to have the possibility to fail to apply the settings (for e.g.
     * when it implies contacting a remote server).
     */
    virtual bool isSaved() const { return true; }

 private:
    QString _name, _icon;
};

//-----------------------------------------------------------------------------
/**
 * This class is used to display one or several @ref SettingWidget.
 */
class SettingsDialog : public KDialogBase
{
 Q_OBJECT
 public:
	SettingsDialog(QWidget *parent);

    void addModule(SettingsWidget *);

 private slots:
    void accept();
    void slotDefault();

 private:
    QPtrVector<SettingsWidget> _widgets;
};

#endif
