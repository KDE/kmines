#include "ghighscores_item.h"

#include <khighscore.h>
#include <kglobal.h>


//-----------------------------------------------------------------------------
const char *ItemBase::ANONYMOUS = "_";

ItemBase::ItemBase(const QVariant &def, const QString &label, int alignment,
                   bool canHaveSubGroup)
    : _default(def), _label(label), _alignment(alignment),
      _format(NoFormat), _special(NoSpecial),
      _canHaveSubGroup(canHaveSubGroup)
{}

void ItemBase::set(const QString &n, const QString &grp, const QString &sgrp)
{
    _name = n;
    _group = grp;
    _subGroup = sgrp;
}

void ItemBase::setPrettyFormat(Format format)
{
    bool buint = ( _default.type()==QVariant::UInt );
    bool bdouble = ( _default.type()==QVariant::Double );
    bool bnum = ( buint || bdouble || _default.type()==QVariant::Int );

    switch (format) {
    case OneDecimal:
    case Percentage:
        Q_ASSERT(bdouble);
        break;
    case Time:
        Q_ASSERT(bnum);
        break;
    case DateTime:
    	Q_ASSERT( _default.type()==QVariant::DateTime );
	break;
    case NoFormat:
        break;
    }

    _format = format;
}

void ItemBase::setPrettySpecial(Special special)
{
    bool buint = ( _default.type()==QVariant::UInt );
    bool bdouble = ( _default.type()==QVariant::Double );
    bool bnum = ( buint || bdouble || _default.type()==QVariant::Int );

    switch (special) {
    case ZeroNotDefined:
        Q_ASSERT(bnum);
        break;
    case NegativeNotDefined:
        Q_ASSERT(bnum && !buint);
        break;
    case Anonymous:
        Q_ASSERT( _default.type()==QVariant::String );
        break;
    case NoSpecial:
        break;
    }

     _special = special;
}

QString ItemBase::entryName() const
{
    if ( !_canHaveSubGroup || _subGroup.isEmpty() ) return _name;
    return _name + "_" + _subGroup;
}

QVariant ItemBase::read(uint i) const
{
    Q_ASSERT( stored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    if ( !hs.hasEntry(i+1, entryName()) ) return _default;
    QVariant v = hs.readEntry(i+1, entryName());
    if ( v.cast(_default.type()) ) return v;
    return _default;
}

QString ItemBase::pretty(uint i) const
{
    switch (_special) {
    case ZeroNotDefined:
        if ( read(i).toUInt()==0 ) return "--";
        break;
    case NegativeNotDefined:
        if ( read(i).toInt()<0 ) return "--";
        break;
    case Anonymous:
        if ( read(i).toString()==ANONYMOUS ) return i18n("anonymous");
        break;
    case NoFormat:
        break;
    }

    switch (_format) {
    case OneDecimal:
        return QString::number(read(i).toDouble(), 'f', 1);
    case Percentage:
        return QString::number(read(i).toDouble(), 'f', 1) + "%";
    case Time:
        return timeFormat(read(i).toUInt());
    case DateTime:
        if ( read(i).toDateTime().isNull() ) return "--";
        return KGlobal::locale()->formatDateTime(read(i).toDateTime());
    case NoSpecial:
        break;
    }

    return read(i).toString();
}

void ItemBase::write(uint i, const QVariant &value) const
{
    Q_ASSERT( stored() );
    KHighscore hs;
    hs.setHighscoreGroup(_group);
    hs.writeEntry(i+1, entryName(), value.toString());
}

void ItemBase::moveDown(uint newIndex) const
{
    Q_ASSERT( newIndex!=0 );
    write(newIndex, read(newIndex-1));
}

QString ItemBase::timeFormat(uint n)
{
    Q_ASSERT( n<3600 && n!=0 );
    n = 3600 - n;
    return QString::number(n / 60).rightJustify(2, '0') + ':'
        + QString::number(n % 60).rightJustify(2, '0');
}

//-----------------------------------------------------------------------------
ItemContainer::ItemContainer(const QString &group, const QString &subGroup)
    : _group(group), _subGroup(subGroup)
{
    _items.setAutoDelete(true);
    _names.setAutoDelete(true);
}

void ItemContainer::addItem(const QString &key, ItemBase *item, bool stored)
{
    uint i = _items.count();
    _names.insert(key, new uint(i));
    _items.append(item);
    _items.at(i)->set(key, (stored ? _group : QString::null), _subGroup);
}

const ItemBase &ItemContainer::item(const QString &n) const
{
    QPtrListIterator<ItemBase> it(_items);
    it += name(n);
    return *it.current();
}

//-----------------------------------------------------------------------------
void DataContainer::addData(const QString &key, ItemBase *item, bool stored,
                            QVariant value)
{
    ItemContainer::addItem(key, item, stored);
    _data.append(value);
}

QString DataContainer::prettyData(const QString &name) const
{
	return data(name).toString();
}

void DataContainer::read(uint i)
{
    QPtrListIterator<ItemBase> it(items());
    while( it.current() ) {
        const ItemBase *item = it.current();
        data(item->name()) = item->read(i);
        ++it;
    }
}

void DataContainer::write(uint i, uint nb) const
{
    QPtrListIterator<ItemBase> it(items());
    while( it.current() ) {
        const ItemBase *item = it.current();
        ++it;
        if ( !item->stored() ) continue;
        for (uint j=nb-1; j>i; j--) item->moveDown(j);
        item->write(i, data(item->name()));
    }
}

QDataStream &operator <<(QDataStream &stream, const DataContainer &c)
{
    stream << c._data;
    return stream;
}

QDataStream &operator >>(QDataStream &stream, DataContainer &c)
{
    stream >> c._data;
    return stream;
}
