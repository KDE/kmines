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

#ifndef G_HIGHSCORES_TAB_H
#define G_HIGHSCORES_TAB_H

#include <qcombobox.h>
#include <qmemarray.h>

class QLabel;
class KListView;


namespace KExtHighscores
{

//-----------------------------------------------------------------------------
class PlayersCombo : public QComboBox
{
 Q_OBJECT
 public:
    PlayersCombo(bool addNoneItem, QWidget *parent = 0, const char *name = 0);

 signals:
    void playerSelected(uint i);
    void allSelected();
    void noneSelected();

 private slots:
    void activatedSlot(int i);
};

//-----------------------------------------------------------------------------
class AdditionalTab : public QWidget
{
 Q_OBJECT
 public:
    AdditionalTab(QWidget *parent, const char *name);

 private slots:
    void playerSelected(uint i) { display(i) ; }
    void allSelected();

 protected:
    void init();
    static QString percent(uint n, uint total, bool withBraces = false);
    virtual void display(uint i) = 0;

 private:
    QComboBox *_combo;
};

//-----------------------------------------------------------------------------
class StatisticsTab : public AdditionalTab
{
 Q_OBJECT
 public:
    StatisticsTab(QWidget *parent);

 private:
    enum Count { Total = 0, Won, Lost, BlackMark, Nb_Counts };
    static const char *COUNT_LABELS[Nb_Counts];
    enum Trend { CurrentTrend = 0, WonTrend, LostTrend, Nb_Trends };
    static const char *TREND_LABELS[Nb_Trends];
    struct Data {
        uint count[Nb_Counts];
        int trend[Nb_Trends];
    };
    QMemArray<Data> _data;
    QLabel *_nbs[Nb_Counts], *_percents[Nb_Counts], *_trends[Nb_Trends];

    QString percent(const Data &, Count) const;
    void display(uint i);
};

//-----------------------------------------------------------------------------
class HistogramTab : public AdditionalTab
{
 Q_OBJECT
 public:
    HistogramTab(QWidget *parent);

 private:
    struct Data {
        uint total, max;
    };
    QMemArray<uint> _counts;
    QMemArray<Data> _data;
    KListView       *_list;

    void display(uint i);
};

}; // namespace

#endif
