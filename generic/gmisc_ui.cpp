#include "gmisc_ui.h"
#include "gmisc_ui.moc"

#include <qlayout.h>
#include <qlabel.h>


//-----------------------------------------------------------------------------
LCD::LCD(uint nbDigits, QWidget *parent, const char *name)
    : QLCDNumber(nbDigits, parent, name)
{
    const QPalette &p = palette();
    _fgColor = p.color(QPalette::Active, QColorGroup::Foreground);
    _hlColor = p.color(QPalette::Active, QColorGroup::HighlightedText);

    connect(&_timer, SIGNAL(timeout()), SLOT(timeout()));

    setFrameStyle(Panel | Plain);
	setSegmentStyle(Flat);

    showValue(0);
}

void LCD::setDefaultColors(const QColor &fgColor, const QColor &bgColor)
{
    _fgColor = fgColor;
    QPalette p = palette();
    p.setColor(QColorGroup::Foreground, fgColor);
    p.setColor(QColorGroup::Background, bgColor);
    setPalette(p);
}

void LCD::setLeadString(const QString &s)
{
    _lead = s;
    showValue(0);
}

void LCD::resetColor()
{
    setColor(QColor());
}

void LCD::setColor(const QColor &color)
{
    const QColor &c = (color.isValid() ? color : _fgColor);
    QPalette p = palette();
    p.setColor(QColorGroup::Foreground, c);
    setPalette(p);
}

void LCD::showValue(uint v)
{
    int n = numDigits() - _lead.length();
    display(_lead + QString::number(v).rightJustify(n));
}

void LCD::highlight()
{
    highlight(true);
    _timer.start(800, true);
}

void LCD::highlight(bool light)
{
    if (light) setColor(_hlColor);
    else resetColor();
}

//-----------------------------------------------------------------------------
LCDClock::LCDClock(QWidget *parent, const char *name)
: LCD(5, parent, name)
{
    connect(&_timerClock, SIGNAL(timeout()), SLOT(timeoutClock()));
}

void LCDClock::timeoutClock()
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

void LCDClock::showTime()
{
    QString sec = QString::number(_sec).rightJustify(2, '0', true);
    QString min = QString::number(_min).rightJustify(2, '0', true);
    display(min + ':' + sec);
}

void LCDClock::reset()
{
    _timerClock.stop();
	_sec = 0;
    _min = 0;
	showTime();
}

void LCDClock::start()
{
    _timerClock.start(1000); // 1 second
}

void LCDClock::stop()
{
    _timerClock.stop();
}

uint LCDClock::time() const
{
    return 3600 - (_min*60 + _sec);
}


//-----------------------------------------------------------------------------
LCDList::LCDList(const QString &label, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QVBoxLayout *top = new QVBoxLayout(this, 5);

    QLabel *lab = new QLabel(label, this);
    lab->setAlignment(AlignCenter);
    top->addWidget(lab, AlignCenter);
}

uint LCDList::append(LCD *lcd)
{
    uint n = _lcds.size();
    _lcds.resize(n+1);
    _lcds.insert(n, lcd);
    ((QVBoxLayout *)layout())->addWidget(lcd);
    return n;
}
