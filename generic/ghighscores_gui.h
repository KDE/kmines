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

#ifndef KEXTHIGHSCORE_GUI_H
#define KEXTHIGHSCORE_GUI_H

#include <qcheckbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtabwidget.h>

#include <klistview.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kdialogbase.h>

#include "ghighscores.h"


namespace KExtHighscore
{

class ItemContainer;
class ItemArray;
class Score;
class AdditionalTab;

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

    void addHeader(const ItemArray &);

 protected:
    QListViewItem *addLine(const ItemArray &, uint index, bool highlight);
    virtual QString itemText(const ItemContainer &, uint row) const = 0;

 private:
    virtual void addLineItem(const ItemArray &, uint index,
                             QListViewItem *item);
};

//-----------------------------------------------------------------------------
class HighscoresList : public ScoresList
{
 Q_OBJECT
 public:
    HighscoresList(QWidget *parent);

    void load(const ItemArray &, int highlight);

 protected:
    QString itemText(const ItemContainer &, uint row) const;
};

class HighscoresWidget : public QWidget
{
 Q_OBJECT
 public:
    HighscoresWidget(QWidget *parent);

    void load(int rank);

 signals:
    void tabChanged(int i);

 public slots:
    void changeTab(int i);

 private slots:
    void showURL(const QString &) const;
    void tabChanged() { emit tabChanged(_tw->currentPageIndex()); }

 private:
    QTabWidget     *_tw;
    HighscoresList *_scoresList, *_playersList;
    KURLLabel      *_scoresUrl, *_playersUrl;
    AdditionalTab  *_statsTab, *_histoTab;
};

class HighscoresDialog : public KDialogBase
{
 Q_OBJECT
 public:
    HighscoresDialog(int rank, QWidget *parent);

 private slots:
    void slotUser1();
    void slotUser2();
    void tabChanged(int i) { _tab = i; }
    void createPage(QWidget *);

 private:
    int _rank, _tab;
    QWidget *_current;
    QValueVector<HighscoresWidget *> _widgets;
};

//-----------------------------------------------------------------------------
class LastMultipleScoresList : public ScoresList
{
    Q_OBJECT
public:
    LastMultipleScoresList(const QValueVector<Score> &, QWidget *parent);

private:
    void addLineItem(const ItemArray &, uint index, QListViewItem *line);
    QString itemText(const ItemContainer &, uint row) const;

private:
    const QValueVector<Score> &_scores;
};

class TotalMultipleScoresList : public ScoresList
{
    Q_OBJECT
public:
    TotalMultipleScoresList(const QValueVector<Score> &, QWidget *parent);

private:
    void addLineItem(const ItemArray &, uint index, QListViewItem *line);
    QString itemText(const ItemContainer &, uint row) const;

private:
    const QValueVector<Score> &_scores;
};

//-----------------------------------------------------------------------------
class ConfigDialog : public KDialogBase
{
 Q_OBJECT
 public:
    ConfigDialog(QWidget *parent);

    bool hasBeenSaved() const { return _saved; }

 private slots:
    void modifiedSlot();
    void removeSlot();
    void accept();
    void slotApply() { save(); }

 private:
    bool         _saved;
    QCheckBox   *_WWHEnabled;
    QLineEdit   *_nickname, *_comment;
    KLineEdit   *_key, *_registeredName;
    KPushButton *_removeButton;

    void load();
    bool save();
};

}; // namespace

#endif
