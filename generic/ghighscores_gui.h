/*
    This file is part of the KDE games library
    Copyright (C) 2001-02 Nicolas Hadacek (hadacek@kde.org)

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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qvbox.h>

#include <klistview.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kdialogbase.h>

#include "ghighscores.h"
#include "gsettings.h"


namespace KExtHighscore
{

class ItemContainer;
class ItemArray;
class Score;

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
    ScoresList(const ItemArray &, QWidget *parent);

    void reload();

 protected:
    void addHeader();
    QListViewItem *addLine(uint index, bool highlight);
    virtual QString itemText(const ItemContainer &, uint row) const = 0;
    virtual bool showColumn(const ItemContainer &) const { return true; }

 private:
    const ItemArray &_items;

    void addLine(uint index, QListViewItem *item);
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
    HighscoresWidget(int localRank, QWidget *parent);

    void reload();

 private slots:
    void showURL(const QString &) const;

 private:
    HighscoresList *_scoresList, *_playersList;
    KURLLabel      *_scoresUrl, *_playersUrl;
};

class HighscoresDialog : public KDialogBase
{
 Q_OBJECT
 public:
    HighscoresDialog(uint nbGameTypes, int rank, QWidget *parent);

 private slots:
    void exportToText();
    void configure();

 private:
    QValueVector<HighscoresWidget *> _widgets;

    void reload();
};

//-----------------------------------------------------------------------------
class MultipleScoresList : public ScoresList
{
 Q_OBJECT
 public:
    MultipleScoresList(const ScoreVector &, QWidget *parent);

 private:
    const ScoreVector _scores;

    QString itemText(const ItemContainer &, uint row) const;
    bool showColumn(const ItemContainer &) const;
};

//-----------------------------------------------------------------------------
class ConfigItem : public KConfigItemBase
{
 Q_OBJECT
 public:
    ConfigItem(QWidget *widget);

 private slots:
    void removeSlot();

 private:
    QWidget     *_widget;
    QCheckBox   *_WWHEnabled;
    QLineEdit   *_nickname, *_comment;
    KLineEdit   *_key, *_registeredName;
    KPushButton *_removeButton;

    void loadState();
    bool saveState();
    void setDefaultState() {}
    bool hasDefault() const { return true; }
};

}; // namespace

#endif
