#ifndef KCONFIGSTRING_H
#define KCONFIGSTRING_H

#include <qcstring.h>
#include <qstring.h>
#include <qrect.h>
#include <qsize.h>
#include <qfont.h>
#include <qcolor.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qvaluelist.h>


class KConfigString
{
 public:
    static bool isUtf8(const char *);

    // "to" methods (for QCString)
    static int toInt(const QCString &s, int def = 0);
    static uint toUnsignedInt(const QCString &s, uint def = 0);
    static long toLong(const QCString &s, long def = 0);
    static unsigned long toUnsignedLong(const QCString &s,
                                        unsigned long def = 0);
    static double toDouble(const QCString &s, double def = 0.0);
    static bool toBool(const QCString &s, bool def = false);

    static uint toList(const QCString &s, QStrList &str, char sep = ',');
    static QRect toRect(const QCString &s, const QRect *def = 0);
    static QPoint toPoint(const QCString &s, const QPoint *def = 0);
    static QSize toSize(const QCString &s, const QSize *def = 0);
    static QDateTime toDateTime(const QCString &s, const QDateTime *def = 0);

    // "to" methods (for QString)
    static QStringList toList(const QString &s, char sep = ',');
    static QValueList<int> toIntList(const QString &s);
    static QFont toFont(const QString &s, const QFont *def = 0);
    static QColor toColor(const QString &s, const QColor *def = 0);
    static QVariant toVariant(const QString &s, QVariant::Type type);
    static QVariant toVariant(const QString &s, const QVariant &def);

    // "from" methods
    static QString from(int value) { return QString::number(value); }
    static QString from(uint value) { return QString::number(value); }
    static QString from(long value) { return QString::number(value); }
    static QString from(unsigned long value) { return QString::number(value); }
    static QString from(bool value) { return (value ? "true" : "false"); }
    static QString from(double value) { return QString::number(value); }

    static QString from(const QStrList &value, char sep = ',');
    static QString from(const QStringList &value, char sep = ',');
    static QString from(const QValueList<int> &value);
    static QString from(const QFont &value) { return value.toString(); }
    static QString from(const QRect &value);
    static QString from(const QPoint &value);
    static QString from(const QSize &value);
    static QString from(const QColor &value);
    static QString from(const QDateTime &value);
    static QString from(const QVariant &value);
};

#endif
