#ifndef G_HIGHSCORES_ITEM_H
#define G_HIGHSCORES_ITEM_H

#include <qvariant.h>
#include <qdatastream.h>
#include <qdict.h>
#include <qnamespace.h>
#include <qdatetime.h>

#include <klocale.h>


//-----------------------------------------------------------------------------
class ItemBase
{
 public:
    enum Format { NoFormat, // no formatting
                  OneDecimal, // double with one decimal
                  Percentage, // double with one decimal + %
                  Time, // MM:SS (3600 is 00:00, 1 is 59:59 and 0 is undefined)
		          DateTime // date & time according to locale
    };
    enum Special { NoSpecial, // no special value
                   ZeroNotDefined, // 0 is replaced by "--"
                   NegativeNotDefined, // negative value are replaced by "--"
                   Anonymous // replace special value by i18n("anonymous")
                   // for DateTime format : a null date is replaced by "--"
    };

    ItemBase() {}
    ItemBase(const QVariant &def, const QString &label, int alignment,
             bool canHaveSubGroup = false);
    virtual ~ItemBase() {}

    void set(const QString &name, const QString &group,
             const QString &subGroup = QString::null);
    void setPrettyFormat(Format); // default is NoFormat
    void setPrettySpecial(Special); // default is NoSpecial

    bool stored() const   { return !_group.isNull(); }
    bool shown() const    { return !_label.isEmpty(); }
    QString label() const { return _label; }
    int alignment() const { return _alignment; }
    QString name() const  { return _name; }

	virtual QVariant read(uint i) const;
	virtual QString pretty(uint i) const;
	void write(uint i, const QVariant &) const;

    void moveDown(uint newIndex) const;

    static const char *ANONYMOUS; // used to recognized anonymous players

 private:
	QVariant _default;
    QString  _name, _label, _group, _subGroup;
    int      _alignment;
    Format   _format;
    Special  _special;
    bool     _canHaveSubGroup;

    QString entryName() const;

    static QString timeFormat(uint);
};

//-----------------------------------------------------------------------------
class ItemContainer
{
 public:
    ItemContainer(const QString &group, const QString &subGroup);
    virtual ~ItemContainer() {}

    virtual uint nbEntries() const = 0;
    const QPtrList<ItemBase> &items() const { return _items; }

 protected:
    void addItem(const QString &key, ItemBase *, bool stored);
    QString group() const { return _group; }

    const ItemBase &item(const QString &name) const;
    uint name(const QString &name) const { return *_names[name]; }

 private:
    QString            _group, _subGroup;
    QPtrList<ItemBase> _items;
    QDict<uint>        _names;

    ItemContainer(const ItemContainer &c);
    void operator =(const ItemContainer &);
};

//-----------------------------------------------------------------------------
class DataContainer : public ItemContainer
{
 public:
    DataContainer(const QString &group, const QString &subGroup)
        : ItemContainer(group, subGroup) {}

    void read(uint i);
    void write(uint i, uint maxNbLines) const;
    QString prettyData(const QString &name) const;

 protected:
    const QVariant &data(const QString &n) const { return _data[name(n)]; }
    QVariant &data(const QString &n)             { return _data[name(n)]; }
    void addData(const QString &key, ItemBase *, bool stored, QVariant value);

 private:
    QValueList<QVariant> _data;

    friend QDataStream &operator >>(QDataStream &, DataContainer &);
    friend QDataStream &operator <<(QDataStream &, const DataContainer &);
};

QDataStream &operator >>(QDataStream &, DataContainer &);
QDataStream &operator <<(QDataStream &, const DataContainer &);

//-----------------------------------------------------------------------------
class RankItem : public ItemBase
{
 public:
    RankItem()
        : ItemBase((uint)0, i18n("Rank"), Qt::AlignRight) {}

    QVariant read(uint rank) const  { return rank; }
    QString pretty(uint rank) const { return QString::number(rank+1); }
};

class ScoreItem : public ItemBase
{
 public:
    ScoreItem()
        : ItemBase((uint)0, i18n("Score"), Qt::AlignRight) {}
};

class NameItem : public ItemBase
{
 public:
    NameItem()
        : ItemBase(QString::null, i18n("Name"), Qt::AlignLeft) {
            setPrettySpecial(Anonymous);
    }
};

class DateItem : public ItemBase
{
 public:
    DateItem()
        : ItemBase(QDateTime(), i18n("Date"), Qt::AlignRight) {
            setPrettyFormat(DateTime);
    }
};

class MeanScoreItem : public ItemBase
{
 public:
    MeanScoreItem()
        : ItemBase((double)0, i18n("Mean score"), Qt::AlignRight, true) {
            setPrettyFormat(OneDecimal);
            setPrettySpecial(ZeroNotDefined);
    }
};

class BestScoreItem : public ItemBase
{
 public:
    BestScoreItem()
        : ItemBase((uint)0, i18n("Best score"), Qt::AlignRight, true) {
            setPrettySpecial(ZeroNotDefined);
    }
};

class SuccessPercentageItem : public ItemBase
{
 public:
    SuccessPercentageItem()
        : ItemBase((double)-1, i18n("Success"), Qt::AlignRight, true) {
            setPrettyFormat(Percentage);
            setPrettySpecial(NegativeNotDefined);
    }
};

#endif
