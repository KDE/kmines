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

#include "gmisc_ui.h"
#include "gmisc_ui.moc"

#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>

#include <kglobal.h>


//-----------------------------------------------------------------------------
KGameLCD::KGameLCD(uint nbDigits, QWidget *parent, const char *name)
    : QLCDNumber(nbDigits, parent, name), _htime(800)
{
    const QPalette &p = palette();
    _fgColor = p.color(QPalette::Active, QColorGroup::Foreground);
    _hlColor = p.color(QPalette::Active, QColorGroup::HighlightedText);

    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), SLOT(timeout()));

    setFrameStyle(Panel | Plain);
	setSegmentStyle(Flat);

    displayInt(0);
}

void KGameLCD::setDefaultColors(const QColor &fgColor, const QColor &bgColor)
{
    _fgColor = fgColor;
    QPalette p = palette();
    p.setColor(QColorGroup::Foreground, fgColor);
    p.setColor(QColorGroup::Background, bgColor);
    setPalette(p);
}

void KGameLCD::setLeadingString(const QString &s)
{
    _lead = s;
    displayInt(0);
}

void KGameLCD::setHighlightTime(uint time)
{
    _htime = time;
}

void KGameLCD::resetColor()
{
    setColor(QColor());
}

void KGameLCD::setColor(const QColor &color)
{
    const QColor &c = (color.isValid() ? color : _fgColor);
    QPalette p = palette();
    p.setColor(QColorGroup::Foreground, c);
    setPalette(p);
}

void KGameLCD::displayInt(int v)
{
    int n = numDigits() - _lead.length();
    display(_lead + QString::number(v).rightJustify(n));
}

void KGameLCD::highlight()
{
    highlight(true);
    _timer->start(_htime, true);
}

void KGameLCD::highlight(bool light)
{
    if (light) setColor(_hlColor);
    else resetColor();
}

//-----------------------------------------------------------------------------
KGameLCDClock::KGameLCDClock(QWidget *parent, const char *name)
: KGameLCD(5, parent, name)
{
    _timerClock = new QTimer(this);
    connect(_timerClock, SIGNAL(timeout()), SLOT(timeoutClock()));
}

void KGameLCDClock::timeoutClock()
{
    // waiting an hour does not restart timer
    if ( _min==59 && _sec==59 ) return;
    _sec++;
    if (_sec==60) {
        _min++;
        _sec = 0;
    }
    showTime();
}

QString KGameLCDClock::pretty() const
{
    QString sec = QString::number(_sec).rightJustify(2, '0', true);
    QString min = QString::number(_min).rightJustify(2, '0', true);
    return min + ':' + sec;
}

void KGameLCDClock::showTime()
{
    display(pretty());
}

void KGameLCDClock::reset()
{
    _timerClock->stop();
	_sec = 0;
    _min = 0;
	showTime();
}

void KGameLCDClock::start()
{
    _timerClock->start(1000); // 1 second
}

void KGameLCDClock::stop()
{
    _timerClock->stop();
}

uint KGameLCDClock::seconds() const
{
    return _min*60 + _sec;
}

void KGameLCDClock::setTime(uint sec)
{
    Q_ASSERT( sec<3600 );
    _sec = sec % 60;
    _min = sec / 60;
    showTime();
}

void KGameLCDClock::setTime(const QString &s)
{
    Q_ASSERT( s.length()==5 && s[2]==':' );
    uint min = kMin(s.section(':', 0, 0).toUInt(), uint(59));
    uint sec = kMin(s.section(':', 1, 1).toUInt(), uint(59));
    setTime(sec + min*60);
}


//-----------------------------------------------------------------------------
KGameLCDList::KGameLCDList(const QString &title, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(title);
}

KGameLCDList::KGameLCDList(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    init(QString::null);
}

void KGameLCDList::init(const QString &title)
{
    QVBoxLayout *top = new QVBoxLayout(this, 5);

    _title = new QLabel(title, this);
    _title->setAlignment(AlignCenter);
    top->addWidget(_title, AlignCenter);
}

uint KGameLCDList::append(QLCDNumber *lcd)
{
    uint n = _lcds.size();
    _lcds.resize(n+1);
    _lcds.insert(n, lcd);
    static_cast<QVBoxLayout *>(layout())->addWidget(lcd);
    return n;
}
