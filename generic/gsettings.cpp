/*
    This file is part of the KDE library
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

#include "gsettings.h"
#include "gsettings.moc"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qobjectlist.h>
#include <qtextedit.h>
#include <qdial.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qdom.h>
#include <qfile.h>
#include <qguardedptr.h>

#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kdebug.h>
#include <kaction.h>
#include <kcolorcombo.h>
#include <kdatepicker.h>
#include <kstandarddirs.h>

#include "gstring.h"


//-----------------------------------------------------------------------------
KConfigItemBase::KConfigItemBase()
    : _modified(false)
{}

KConfigItemBase::~KConfigItemBase()
{}

void KConfigItemBase::load()
{
    loadState();
    _modified = false;
}

bool KConfigItemBase::save()
{
    if ( !_modified ) return true;
    bool success = saveState();
    if (success) _modified = false;
    return success;
}

bool KConfigItemBase::setDefault()
{
    if ( hasDefault() ) return false;
    setDefaultState();
    setModified();
    return true;
}

void KConfigItemBase::setModified()
{
    _modified = true;
}

//-----------------------------------------------------------------------------
class KConfigItemPrivate
{
 public:
    struct Data {
        const char *typeName, *className, *signal, *property, *labelProperty;
        QVariant::Type type;
    };

    KConfigItemPrivate(KConfigItem::Type type)
        : _type(type), _object(0) {}

    void setObject(QObject *obj, QVariant::Type valueType) {
        _object = 0;
        if ( obj==0 ) return;

        for (uint i=0; i<DATA[_type].nb; i++) {
            const Data &data = DATA[_type].data[i];
            if ( valueType==data.type && obj->inherits(data.className) ) {
                _objectType = i;
                _object = obj;
                return;
            }
        }
        kdError() << "object not supported for type \"" << DATA[_type].name
                  << "\" and value type \"" << QVariant::typeToName(valueType)
                  << endl;
    }

    QObject *object() const { return _object; }

    void setText(const QString &text) {
        Q_ASSERT(_object);
        const char *p = data().labelProperty;
        if (p) _object->setProperty(p, text);
    }

    void setWhatsThis(const QString &text) {
        Q_ASSERT(_object);
        if ( _object->inherits("QWidget") )
            QWhatsThis::add(static_cast<QWidget *>((QObject *)_object), text);
        else if ( _object->inherits("KAction") )
            static_cast<KAction *>((QObject *)_object)->setWhatsThis(text);
    }

    void setToolTip(const QString &text) {
        Q_ASSERT(_object);
        if ( _object->inherits("QWidget") )
            QToolTip::add(static_cast<QWidget *>((QObject *)_object), text);
        else if ( _object->inherits("KAction") )
            static_cast<KAction *>((QObject *)_object)->setToolTip(text);
    }

    uint objectType() const {
        Q_ASSERT(_object);
        return _objectType;
    }

    static const char *name(KConfigItem::Type type) {
        return DATA[type].name;
    }

    const Data &data() const {
        Q_ASSERT(_object);
        return DATA[_type].data[_objectType];
    }

    enum SimpleType {
        CheckBox = 0, LineEdit, ColorButton, ToggleAction,
        ColorComboBox, EditableComboBox, DatePicker, FontAction,
        FontSizeAction, DateTimeEdit, TextEdit, NB_SIMPLE_TYPES
    };
    enum RangedType {
        IntInput = 0, DoubleInput, SpinBox, Slider, Dial,
        Selector, NB_RANGED_TYPES
    };
    enum MultiType {
        ReadOnlyComboBox = 0, RadioButtonGroup, SelectAction, NB_MULTI_TYPES
    };

 private:
    KConfigItem::Type    _type;
    int                  _objectType;
    QGuardedPtr<QObject> _object;

    static const Data SIMPLE_DATA[NB_SIMPLE_TYPES];
    static const Data RANGED_DATA[NB_RANGED_TYPES];
    static const Data MULTI_DATA[NB_MULTI_TYPES];

     struct ItemData {
         uint nb;
         const Data *data;
         const char *name;
    };
    static const ItemData DATA[KConfigItem::NB_ITEM_TYPES];
};

const KConfigItemPrivate::Data
KConfigItemPrivate::SIMPLE_DATA[NB_SIMPLE_TYPES] = {
    {"CheckBox", "QCheckBox", SIGNAL(toggled(bool)),
     "checked",     "text", QVariant::Bool},
    {"LineEdit", "QLineEdit", SIGNAL(textChanged(const QString &)),
     "text",        0,      QVariant::String},
    {"ColorButton", "KColorButton", SIGNAL(changed(const QColor &)),
     "color",       0,      QVariant::Color},
    {"ToggleAction", "KToggleAction", SIGNAL(toggled(bool)),
     "checked",     "text", QVariant::Bool},
    // KColorComboBox must to be before QComboBox
    {"ColorComboBox", "KColorCombo", SIGNAL(activated(const QString &)),
     "color",       0,      QVariant::UInt},
    {"ComboBox", "QComboBox", SIGNAL(activated(const QString &)),
     "currentText", 0,      QVariant::UInt},
    {"DatePicker", "KDatePicker", SIGNAL(dateChanged(QDate)),
     "date",        0,      QVariant::Date},
    {"FontAction", "KFontAction", SIGNAL(activated(int)),
     "font",        "text", QVariant::String},
    {"FontSizeAction", "KFontSizeAction", SIGNAL(activated(int)),
     "fontSize",    "text", QVariant::Int},
    {"DateTimeEdit", "QDateTimeEdit", SIGNAL(valueChanged(const QDateTime &)),
     "dateTime",    0,      QVariant::DateTime},
    {"TextEdit", "QTextEdit", SIGNAL(textChanged(const QString &)),
     "text",        0,      QVariant::String}
};

const KConfigItemPrivate::Data
KConfigItemPrivate::RANGED_DATA[NB_RANGED_TYPES] = {
    {"IntInput", "KIntNumInput", SIGNAL(valueChanged(int)),
     "value", "label", QVariant::Int},
    {"DoubleInput", "KDoubleNumInput", SIGNAL(valueChanged(double)),
     "value", "label", QVariant::Double},
    {"SpinBox", "QSpinBox", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    {"Slider", "QSlider", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    {"Dial", "QDial", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    {"Selector", "KSelector", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int}
};

const KConfigItemPrivate::Data
KConfigItemPrivate::MULTI_DATA[NB_MULTI_TYPES] = {
    {"ReadOnlyComboBox", "QComboBox", SIGNAL(activated(const QString &)),
     "currentText", 0, QVariant::String},
    {"RadioButtonGroup", "QButtonGroup", SIGNAL(clicked(int)),
     0, "text", QVariant::String},
    {"SelectAction", "KSelectAction", SIGNAL(activated(int)),
     "currentText", "text", QVariant::String}
};

const KConfigItemPrivate::ItemData
KConfigItemPrivate::DATA[KConfigItem::NB_ITEM_TYPES] = {
    { NB_SIMPLE_TYPES, SIMPLE_DATA, "simple" },
    { NB_RANGED_TYPES, RANGED_DATA, "ranged" },
    { NB_MULTI_TYPES,  MULTI_DATA,  "multi"  }
};

//-----------------------------------------------------------------------------
KConfigItem::KConfigItem(Type type, QVariant::Type valueType,
                        const QCString &group, const QCString &key,
                        const QVariant &def)
    : _group(group), _key(key), _def(def), _label(0)
{
    d = new KConfigItemPrivate(type);

    if ( !_def.cast(valueType) )
        kdWarning() << k_funcinfo << "cannot cast default value to type : "
                    << QVariant::typeToName(valueType) << endl;
}

KConfigItem::~KConfigItem()
{
    delete d;
}

void KConfigItem::setObject(QObject *o)
{
    d->setObject(o, _def.type());
    if (object()) {
        if ( !_text.isNull() ) d->setText(_text);
        if ( !_whatsthis.isNull() ) d->setWhatsThis(_whatsthis);
        if ( !_tooltip.isNull() ) d->setToolTip(_tooltip);
        initObject();
    }
}

const char *KConfigItem::signal() const
{
    return d->data().signal;
}

void KConfigItem::setText(const QString &text)
{
    _text = text;
    if (_label) _label->setText(_text);
    else if ( d->object() ) d->setText(_text);
}

void KConfigItem::setProxyLabel(QLabel *label)
{
    _label = label;
    if (label) {
        label->setText(_text);
        if ( !_whatsthis.isEmpty() ) QWhatsThis::add(_label, _whatsthis);
        if ( !_tooltip.isEmpty() ) QToolTip::add(_label, _tooltip);
    }
    if (object()) d->setText(label ? QString::null : _text);
}

void KConfigItem::setWhatsThis(const QString &text)
{
    _whatsthis = text;
    if (d->object() ) d->setWhatsThis(text);
    if (_label) QWhatsThis::add(_label, text);
}

void KConfigItem::setToolTip(const QString &text)
{
    _tooltip = text;
    if ( d->object() ) d->setToolTip(text);
    if (_label) QToolTip::add(_label, text);
}

void KConfigItem::loadState()
{
    setValue(configValue());
}

bool KConfigItem::saveState()
{
    KConfigGroupSaver cg(kapp->config(), _group);
    cg.config()->writeEntry(_key.data(), value());
    return true;
}

void KConfigItem::setDefaultState()
{
    setValue(_def);
}

bool KConfigItem::hasDefault() const
{
    return ( value()==_def );
}

QVariant KConfigItem::configValue() const
{
    KConfigGroupSaver cg(kapp->config(), _group);
    return cg.config()->readPropertyEntry(_key.data(), _def);
}

void KConfigItem::checkType(const QVariant &v) const
{
    if ( !v.canCast(_def.type()) )
        kdWarning() << k_funcinfo << "cannot cast the value to type : "
                    << _def.typeName() << endl;
}

void KConfigItem::setValue(const QVariant &v)
{
    Q_ASSERT( d->object() );
    checkType(v);

    const char *p = d->data().property;
    Q_ASSERT(p);
    bool ok = d->object()->setProperty(p, v);
    Q_ASSERT(ok);
}

QVariant KConfigItem::value() const
{
    Q_ASSERT( d->object() );

    const char *p = d->data().property;
    Q_ASSERT(p);
    QVariant v = d->object()->property(p);
    Q_ASSERT(v.isValid());
    return v;
}

QObject *KConfigItem::object() const
{
    return d->object();
}

uint KConfigItem::objectType() const
{
    return d->objectType();
}

//-----------------------------------------------------------------------------
KSimpleConfigItem::KSimpleConfigItem(QVariant::Type type,
                                    const QCString &group, const QCString &key,
                                    const QVariant &def)
    : KConfigItem(Simple, type, group, key, def)
{}

KSimpleConfigItem::~KSimpleConfigItem()
{}

//-----------------------------------------------------------------------------
KRangedConfigItem::KRangedConfigItem(QVariant::Type type,
                                    const QCString &group, const QCString &key,
                                    const QVariant &def,
                                    const QVariant &min, const QVariant &max)
    : KConfigItem(Ranged, type, group, key, def)
{
    setRange(min, max);
}

KRangedConfigItem::~KRangedConfigItem()
{}

void KRangedConfigItem::setRange(const QVariant &min, const QVariant &max)
{
    checkType(min);
    _min = min;
    checkType(max);
    _max = max;
    if (object()) initObject();
}

void KRangedConfigItem::initObject()
{
    object()->setProperty("minValue", _min);
    object()->setProperty("maxValue", _max);
}

QVariant KRangedConfigItem::minValue() const
{
    return (object() ? object()->property("minValue") : _min);
}

QVariant KRangedConfigItem::maxValue() const
{
    return (object() ? object()->property("maxValue") : _max);
}

QVariant KRangedConfigItem::bound(const QVariant &v) const
{
    checkType(v);

    QVariant min = minValue();
    QVariant max = maxValue();
    switch ( defaultValue().type() ) {
    case QVariant::Int:
        return kMin(kMax(v.toInt(), min.toInt()), max.toInt());
    case QVariant::Double:
        return kMin(kMax(v.toDouble(), min.toDouble()), max.toDouble());
    default:
        break;
    }

    Q_ASSERT(false);
    return QVariant();
}

QVariant KRangedConfigItem::configValue() const
{
    return bound( KConfigItem::configValue() );
}

//-----------------------------------------------------------------------------
KMultiConfigItem::KMultiConfigItem(uint nbItems,
                                   const QCString &group, const QCString &key,
                                   const QVariant &def)
    : KConfigItem(Multi, QVariant::String, group, key, def),
      _entries(nbItems), _items(nbItems)
{}

void KMultiConfigItem::initObject()
{
    KConfigItemPrivate::MultiType type =
        (KConfigItemPrivate::MultiType)objectType();
    if ( type==KConfigItemPrivate::ReadOnlyComboBox
         && object()->property("editable").toBool() ) {
        kdError() << k_funcinfo << "the combobox should be readonly" << endl;
        return;
    }

    if ( type==KConfigItemPrivate::RadioButtonGroup ) {
        for (uint i=0; i<_entries.size(); i++) {
            QButton *button = static_cast<QButtonGroup *>(object())->find(i);
            if ( button && button->inherits("QRadioButton") )
                button->setText(_items[i]);
            else kdError() << k_funcinfo << "cannot find radio button at id #"
                           << i << endl;
        }
    } else {
        QStringList list;
        for (uint i=0; i<_entries.size(); i++) list.append(_items[i]);
        switch (type) {
        case KConfigItemPrivate::ReadOnlyComboBox:
            static_cast<QComboBox *>(object())->insertStringList(list);
            break;
        case KConfigItemPrivate::SelectAction:
            static_cast<KSelectAction *>(object())->setItems(list);
            break;
        default:
            break;
        }
    }
}

KMultiConfigItem::~KMultiConfigItem()
{}

void KMultiConfigItem::map(uint id, const char *entry, const QString &text)
{
    Q_ASSERT( id<_entries.size() );
    _entries[id] = entry;
    _items[id] = text;
}

int KMultiConfigItem::simpleMapToId(const char *entry) const
{
    for (uint i=0; i<_entries.size(); i++)
        if ( _entries[i]==entry ) return i;

    bool ok;
    uint i = QString(entry).toUInt(&ok);
    if ( ok && i<_entries.size() ) return i;
    return -1;
}

int KMultiConfigItem::mapToId(const char *entry) const
{
    int id = simpleMapToId(entry);
    if ( id==-1 ) id = simpleMapToId(defaultValue().toString().utf8());
    if ( id==-1 ) return 0;
    return id;
}

void KMultiConfigItem::setValue(const QVariant &v)
{
    Q_ASSERT(object());
    checkType(v);

    bool ok = false;
    int id = mapToId(v.toString().utf8());
    if ( (KConfigItemPrivate::MultiType)objectType()
         ==KConfigItemPrivate::RadioButtonGroup ) {
        QButton *b = static_cast<QButtonGroup *>(object())->find(id);
        if ( b && b->inherits("QRadioButton") )
            static_cast<QRadioButton *>(b)->setChecked(true);
    } else ok = object()->setProperty("currentItem", id);

    Q_ASSERT(ok);
}

uint KMultiConfigItem::findRadioButtonId(const QButtonGroup *group) const
{
    QObjectList *list = group->queryList("QRadioButton");
    QObjectListIt it(*list);
    for (; it.current()!=0; ++it) {
        QRadioButton *rb = static_cast<QRadioButton *>(it.current());
        if ( rb->isChecked() ) return group->id(rb);
    }
    delete list;
    kdWarning() << k_funcinfo
                << "there is no QRadioButton in this QButtonGroup" << endl;
    return 0;
}

QVariant KMultiConfigItem::value() const
{
    Q_ASSERT(object());

    int id;
    if ( (KConfigItemPrivate::MultiType)objectType()
         ==KConfigItemPrivate::RadioButtonGroup )
        id = findRadioButtonId(static_cast<const QButtonGroup *>(object()));
    else {
        QVariant v = object()->property("currentItem");
        Q_ASSERT(v.isValid());
        id = v.toInt();
    }
    if ( !_entries[id].isNull() ) return _entries[id];
    return id;
}

int KMultiConfigItem::configIndex() const
{
    return mapToId(configValue().toString().utf8());
}

//-----------------------------------------------------------------------------
QAsciiDict<KConfigItem> *KConfigCollection::_items = 0;

KConfigCollection::KConfigCollection(QObject *parent, const char *name)
    : QObject(parent, name)
{}

#define ITEM_NAMED "item named \"" << name << "\" : "

void KConfigCollection::init()
{
    Q_ASSERT( _items==0 );
    _items = new QAsciiDict<KConfigItem>;
    _items->setAutoDelete(true);

    // read XML file
    QDomDocument doc;
    QString name = KGlobal::instance()->instanceName();
    QString xml_file = locate("data", name + '/' + name + "config.rc");
    QFile file(xml_file);
    if ( !file.open( IO_ReadOnly ) )
        kdFatal() << "No XML config file \"" << xml_file << "\"" << endl;
    QByteArray buffer(file.readAll());
    doc.setContent( QString::fromUtf8(buffer.data(), buffer.size()) );
    QDomElement root = doc.namedItem("kconfig").toElement();
    if ( root.isNull() ) kdFatal() << "no \"kconfig\" element" << endl;

    // create all items
    QDomNodeList glist = root.elementsByTagName("Group");
    for (uint i=0; i<glist.count(); i++) {
        QDomElement group = glist.item(i).toElement();
        if ( group.isNull() ) continue;
        QDomNodeList elist = group.elementsByTagName("Entry");
        for (uint j=0; j<elist.count(); j++) {
            QDomElement entry = elist.item(j).toElement();
            if ( entry.isNull() ) continue;
            QString name = entry.attribute("name");
            KConfigItem *item = createItem(group, entry, name.utf8());
            if ( item==0 ) continue;
            if ( _items->find(name.utf8())!=0 )
                kdWarning() << ITEM_NAMED << "duplicated name" << endl;
            else _items->insert(name.utf8(), item);
        }
    }
}

void KConfigCollection::cleanUp()
{
    delete _items;
    _items = 0;
}

KConfigCollection::~KConfigCollection()
{
    Iterator it = _list.begin();
    for (; it!=_list.end(); ++it)
        remove(*it);
}

void KConfigCollection::loadState()
{
    Iterator it = _list.begin();
    blockSignals(true); // do not emit modified()
    for (; it!=_list.end(); ++it) (*it)->load();
    blockSignals(false);
}

bool KConfigCollection::saveState()
{
    bool ok = true;
    Iterator it = _list.begin();
    for (; it!=_list.end(); ++it)
        if ( !(*it)->save() ) ok = false;
    emit saved();
    return ok;
}

void KConfigCollection::setDefaultState()
{
    Iterator it = _list.begin();
    for (; it!=_list.end(); ++it) {
        blockSignals(true); // emit modified() only once
        bool mod = (*it)->setDefault();
        blockSignals(false);
        if (mod) emit modified((*it));
    }
    emit modified();
}

bool KConfigCollection::hasDefault() const
{
    ConstIterator it = _list.begin();
    for (; it!=_list.end(); ++it)
        if ( !(*it)->hasDefault() ) return false;
    return true;
}

KConfigItem *KConfigCollection::item(const char *name)
{
    if ( (*_items)[name]==0 ) kdWarning() << ITEM_NAMED << "not found" << endl;
    return (*_items)[name];
}

KConfigItem *KConfigCollection::plug(const char *name, QObject *object)
{
    Q_ASSERT(object);
    KConfigItem *it = item(name);
    if ( it==0 ) return 0;
    ConstIterator iter = _list.begin();
    for (; iter!=_list.end(); ++iter)
        if ( (*iter)==it) {
            kdWarning() << ITEM_NAMED << "already plugged in the collection"
                        << endl;
            return it;
        }
    it->setObject(object);
    _list.append(it);
    connect(object, it->signal(), SLOT(modifiedSlot()));
    return it;
}

void KConfigCollection::modifiedSlot()
{
    ConstIterator iter = _list.begin();
    for (; iter!=_list.end(); ++iter)
        if ( (*iter)->object()==sender() ) {
            (*iter)->setModified();
            setModified();
            emit modified((*iter));
            emit modified();
            return;
        }
}

void KConfigCollection::unplug(const char *name)
{
    KConfigItem *it = item(name);
    if (it) {
        remove(it);
        Iterator iter = _list.begin();
        for (; iter!=_list.end(); ++it)
            if ( (*iter)==it) {
                _list.remove(iter);
                break;
            }
    }
}

void KConfigCollection::remove(KConfigItem *item)
{
    if ( item->object() ) {
        item->object()->disconnect(this, SLOT(modifiedSlot()));
        item->setObject(0);
    }
}

QVariant KConfigCollection::configValue(const char *name)
{
    const KConfigItem *it = item(name);
    QVariant v;
    if (it) v = it->configValue();
    return v;
}

uint KConfigCollection::configIndex(const char *name)
{
    const KMultiConfigItem *it =
        static_cast<const KMultiConfigItem *>(item(name));
    return (it ? it->configIndex() : 0);
}

QVariant KConfigCollection::maxValue(const char *name)
{
    const KRangedConfigItem *it =
        static_cast<const KRangedConfigItem *>(item(name));
    return (it ? it->maxValue() : QVariant());
}

QVariant KConfigCollection::minValue(const char *name)
{
    const KRangedConfigItem *it =
        static_cast<const KRangedConfigItem *>(item(name));
    return (it ? it->minValue() : QVariant());
}

#define ENTRY_NAMED "entry named \"" << name << "\" : "

KConfigItem *KConfigCollection::createItem(QDomElement &group,
                                      QDomElement &entry, const QCString &name)
{
    // read common attributes
    QCString groupKey = group.attribute("name").utf8();
    QCString key = entry.attribute("key").utf8();
    if ( key.isEmpty() ) key = name;
    QString text = entry.namedItem("text").toElement().text();
    QString whatsthis = entry.namedItem("whatsthis").toElement().text();
    QString tooltip = entry.namedItem("tooltip").toElement().text();

    // recognize item type
    QCString itype = entry.attribute("type").utf8();
    KConfigItem::Type type = KConfigItem::NB_ITEM_TYPES;
    if ( itype.isEmpty() ) type = KConfigItem::Simple;
    else {
        for (uint i=0; i<KConfigItem::NB_ITEM_TYPES; i++)
            if ( itype==KConfigItemPrivate::name((KConfigItem::Type)i) ) {
                type = (KConfigItem::Type)i;
                break;
            }
        if ( type==KConfigItem::NB_ITEM_TYPES ) {
            kdWarning() << ENTRY_NAMED << "unrecognized item type (" << itype
                        << ")" << endl;
            return 0;
        }
    }

    // recognize value type
    QCString stype = entry.attribute("vtype").utf8();
    QVariant::Type vtype = QVariant::String;
    if ( type==KConfigItem::Multi ) {
        if ( !stype.isEmpty() && stype!="QString" ) {
            kdWarning() << ENTRY_NAMED
                        << "only QString value type is allowed for"
                        << " multi config" << endl;
            return 0;
        }
    } else {
        vtype = QVariant::nameToType(stype);
        if ( vtype==QVariant::Invalid ) {
            kdWarning() << ENTRY_NAMED << "invalid value type" << endl;
            return 0;
        }
    }

    // check default value
    QVariant def =
        KConfigString::toVariant(entry.attribute("defaultValue"), vtype);

    // specific
    KConfigItem *ci = 0;
    if ( type==KConfigItem::Simple )
        ci = new KSimpleConfigItem(vtype, groupKey, key, def);
    else if ( type==KConfigItem::Ranged ) {
        QVariant min =
            KConfigString::toVariant(entry.attribute("minValue"), vtype);
        QVariant max =
            KConfigString::toVariant(entry.attribute("maxValue"), vtype);
       ci = new KRangedConfigItem(vtype, groupKey, key, def, min, max);
    } else if ( type==KConfigItem::Multi ) {
        QDomNodeList list = entry.elementsByTagName("Item");
        QStringList names, texts;
        for (uint i=0; i<list.count(); i++) {
            QDomElement item = list.item(i).toElement();
            if ( item.isNull() ) continue;
            names.append(item.attribute("name"));
            texts.append(item.text());
        }
        KMultiConfigItem *mci =
            new KMultiConfigItem(names.count(), groupKey, key, def);
        for (uint i=0; i<names.count(); i++)
            mci->map(i, names[i].utf8(), i18n(texts[i].utf8()));
        ci = mci;
    }

    if ( !text.isNull() ) ci->setText( i18n(text.utf8()) );
    if ( !whatsthis.isNull() ) ci->setWhatsThis( i18n(whatsthis.utf8()) );
    if ( !tooltip.isNull() ) ci->setToolTip( i18n(tooltip.utf8()) );
    return ci;
}

//-----------------------------------------------------------------------------
KConfigWidget::KConfigWidget(const QString &title, const QString &icon,
                               QWidget *parent, const char *name)
    : QWidget(parent, name), _title(title), _icon(icon)
{
    _configCollection = new KConfigCollection(this);
}

KConfigWidget::~KConfigWidget()
{}

//-----------------------------------------------------------------------------
KConfigDialog::KConfigDialog(QWidget *parent, const char *name)
    : KDialogBase(IconList, i18n("Configure"), Ok|Apply|Cancel|Default,
                  Cancel, parent, name, true, true), _unique(0), _saved(false)
{
    connect(this, SIGNAL(aboutToShowPage(QWidget *)),
            SLOT(aboutToShowPageSlot(QWidget *)));
    enableButtonApply(false);
}

KConfigDialog::KConfigDialog(KConfigWidget *widget, QWidget *parent,
                             const char *name)
    : KDialogBase(Swallow, widget->title(), Ok|Apply|Cancel|Default, Cancel,
                  parent, name, true, true), _unique(widget), _saved(false)
{
    setMainWidget(widget);
    _widgets.append(widget);
    widget->configCollection()->load();
    connect(widget->configCollection(), SIGNAL(modified()), SLOT(modified()));
    enableButtonApply(false);
    aboutToShowPageSlot(0);
}

KConfigDialog::~KConfigDialog()
{}

void KConfigDialog::append(KConfigWidget *w)
{
    Q_ASSERT( !_unique );
    QFrame *page = addPage(w->title(), QString::null,
                           BarIcon(w->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);
    _widgets.append(w);

    w->configCollection()->load();
    connect(w->configCollection(), SIGNAL(modified()), SLOT(modified()));
    if ( pageIndex(page)==0 ) aboutToShowPageSlot(page);
}

void KConfigDialog::slotDefault()
{
    KConfigWidget *cw = (_unique ? _unique : _widgets.at(activePageIndex()));
    cw->configCollection()->setDefault();
}

void KConfigDialog::accept()
{
    if ( apply() ) {
        KDialogBase::accept();
        kapp->config()->sync(); // #### safer
    }
}

void KConfigDialog::modified()
{
    KConfigWidget *cw = (_unique ? _unique : _widgets.at(activePageIndex()));
    bool hasDefault = cw->configCollection()->hasDefault();
    enableButton(Default, !hasDefault);
    enableButtonApply(true);
}

bool KConfigDialog::apply()
{
    bool ok = true;
    for (uint i=0; i<_widgets.count(); i++)
        if ( !_widgets.at(i)->configCollection()->save() ) ok = false;
    _saved = true;
    emit saved();
    return ok;
}

void KConfigDialog::slotApply()
{
    if ( apply() ) enableButtonApply(false);
}

void KConfigDialog::aboutToShowPageSlot(QWidget *page)
{
    KConfigWidget *cw = (_unique ? _unique : _widgets.at(pageIndex(page)));
    bool hasDefault = cw->configCollection()->hasDefault();
    enableButton(Default, !hasDefault);
}
