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

#ifndef G_HIGHSCORES_GUI_H
#define G_HIGHSCORES_GUI_H

#include <qptrvector.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtabwidget.h>

#include <klistview.h>
#include <kdialogbase.h>
#include <klocale.h>

#include "gsettings.h"
#include "ghighscores.h"


namespace KExtHighscores
{

class ItemContainer;

//-----------------------------------------------------------------------------
class ShowItem : public KListViewItem
{
 public:
    ShowItem(QListView *, bool highlight);

 protected:
    virtual void paintCell(QPainter *, const QColorGroup &, int column,
						   int width, int align);

 private:
    bool _highlight;
};

class ScoresList : public KListView
{
 Q_OBJECT
 public:
    ScoresList(QWidget *parent);

 protected:
    // index==-1 : header
    void addLine(const ItemArray &, int index, bool highlight);
    virtual QString itemText(const ItemContainer &, uint row) const = 0;
    virtual bool showColumn(const ItemContainer &) const { return true; }
};

//-----------------------------------------------------------------------------
class HighscoresList : public ScoresList
{
 Q_OBJECT
 public:
    HighscoresList(const ItemArray &, int highlight, QWidget *parent);

 protected:
    QString itemText(const ItemContainer &, uint row) const;
};

class HighscoresWidget : public QWidget
{
 Q_OBJECT
 public:
    HighscoresWidget(int localRank, QWidget *parent, const ScoreInfos &,
                     const PlayerInfos &, int spacingHint,
                     bool WWHSAvailable, const QString &highscoresURL,
                     const QString &playersURL);

 private slots:
    void showURL(const QString &) const;
};

//-----------------------------------------------------------------------------
class MultipleScoresList : public ScoresList
{
 Q_OBJECT
 public:
    MultipleScoresList(const ScoreList &, QWidget *parent);

 private:
    const ScoreList &_scores;

    QString itemText(const ItemContainer &, uint row) const;
    bool showColumn(const ItemContainer &) const;
};

//-----------------------------------------------------------------------------
class HighscoresSettingsWidget : public SettingsWidget
{
 Q_OBJECT
 public:
    HighscoresSettingsWidget(const PlayerInfos &infos, bool WWHSAvailable,
                             QWidget *parent);

    void load();
    void save();
    bool isSaved() const { return _ok; }

 private:
    bool               _ok;
    const PlayerInfos &_infos;
    QCheckBox         *_WWHEnabled;
    QLineEdit         *_nickname, *_comment;
};

}; // namespace

#endif
