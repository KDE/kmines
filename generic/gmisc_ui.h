#ifndef MISC_UI_H
#define MISC_UI_H

#include <qlcdnumber.h>
#include <qtimer.h>
#include <qptrvector.h>


//-----------------------------------------------------------------------------
class LCD : public QLCDNumber
{
 Q_OBJECT

 public:
    LCD(uint nbDigits, QWidget *parent = 0, const char *name = 0);

    void setDefaultColors(const QColor &foregroundColor,
                          const QColor &backgroundColor);
    void setLeadString(const QString &s);

    /**
     * Reset the foreground color to the default one.
     */
    void resetColor();

    /**
     * Set the foreground color to the given one.
     * If the given color is not valid, the default one is used.
     */
    void setColor(const QColor &color);

 public slots:
    void showValue(uint);
    void highlight();

 private slots:
    void timeout() { highlight(false); }

 private:
    QColor  _fgColor, _hlColor;
    QString _lead;
    QTimer  _timer;

    void highlight(bool light);
};

//-----------------------------------------------------------------------------
class LCDClock : public LCD
{
 Q_OBJECT
 public:
    LCDClock(QWidget *parent = 0, const char *name = 0);

    void reset();

    /**
     * @return the time as 3600 - (minutes*60 + seconds).
     */
    uint time() const;

    QString pretty() const;

 public slots:
	virtual void stop();
	virtual void start();

 protected slots:
    virtual void timeoutClock();

 private:
    QTimer _timerClock;
	uint   _sec, _min;

	void showTime();
};

//-----------------------------------------------------------------------------
class LCDList : public QWidget
{
 Q_OBJECT
 public:
    LCDList(const QString &label, QWidget *parent = 0, const char *name = 0);

    uint append(LCD *lcd);
    LCD *lcd(uint i) const { return _lcds[i]; }

 private:
    QPtrVector<LCD> _lcds;
};

#endif
