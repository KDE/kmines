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
KConfigItemBase::KConfigItemBase(QObject *parent, const char *name)
    : QObject(parent, name), _modified(false)
{}

KConfigItemBase::~KConfigItemBase()
{}

void KConfigItemBase::load()
{
    blockSignals(true); // do not emit modified()
    loadState();
    blockSignals(false);
    _modified = false;
}

bool KConfigItemBase::save()
{
    if ( !_modified ) return true;
    bool success = saveState();
    if (success) {
        _modified = false;
        emit saved();
    }
    return success;
}

void KConfigItemBase::setDefault()
{
    // NB: we emit modified() by hand because some widget (like QComboBox)
    // reports changes with a signal that only gets activated by user actions.
    blockSignals(true);
    setDefaultState();
    blockSignals(false);
    modifiedSlot();
}

void KConfigItemBase::modifiedSlot()
{
    _modified = true;
    emit modified();
}

//-----------------------------------------------------------------------------
KConfigItemList::KConfigItemList(QObject *parent, const char *name)
    : KConfigItemBase(parent, name)
{}

KConfigItemList::~KConfigItemList()
{
    QPtrListIterator<KConfigItemBase> it(_list);
    for (; it.current()!=0; ++it) {
        it.current()->disconnect(this, SLOT(itemDestroyed(QObject *)));
        delete it.current();
    }
}

void KConfigItemList::insert(KConfigItemBase *setting)
{
    connect(setting, SIGNAL(modified()), SLOT(modifiedSlot()));
    connect(setting, SIGNAL(destroyed(QObject *)),
            SLOT(itemDestroyed(QObject *)));
    _list.append(setting);
}

void KConfigItemList::remove(KConfigItemBase *setting)
{
    delete setting;
}

void KConfigItemList::loadState()
{
    QPtrListIterator<KConfigItemBase> it(_list);
    for (; it.current()!=0; ++it) it.current()->load();
}

bool KConfigItemList::saveState()
{
    QPtrListIterator<KConfigItemBase> it(_list);
    bool ok = true;
    for (; it.current()!=0; ++it)
        if ( !it.current()->save() ) ok = false;
    return ok;
}

void KConfigItemList::setDefaultState()
{
    QPtrListIterator<KConfigItemBase> it(_list);
    for (; it.current()!=0; ++it) it.current()->setDefault();
}

bool KConfigItemList::hasDefault() const
{
    QPtrListIterator<KConfigItemBase> it(_list);
    for (; it.current()!=0; ++it)
        if ( !it.current()->hasDefault() ) return false;
    return true;
}

void KConfigItemList::itemDestroyed(QObject *object)
{
    _list.removeRef(static_cast<KConfigItemBase *>(object));
}

//-----------------------------------------------------------------------------
class KConfigItemPrivate
{
 public:
    struct Data {
        const char *typeName, *className, *signal, *property, *labelProperty;
        QVariant::Type type;
    };

    KConfigItemPrivate(KConfigItem::Type type, QVariant::Type valueType,
                       QObject *object)
        : _type(type), _object(0) {
        if ( object==0 ) return;

        for (uint i=0; i<DATA[type].nb; i++) {
            const Data &data = DATA[type].data[i];
            if ( valueType==data.type && object->inherits(data.className) ) {
                _objectType = i;
                _object = object;
                return;
            }
        }
        kdError() << "object not supported for type \"" << DATA[type].name
                  << "\" and value type \"" << QVariant::typeToName(valueType)
                  << endl;
    }

    QObject *object() const { return _object; }
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
    KConfigItem::Type _type;
    int      _objectType;
    QObject *_object;

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
                         QObject *object,
                         const QString &group, const QString &key,
                         const QVariant &def, const QString &text,
                         KConfigCollection *col, const char *name)
    : KConfigItemBase(col, name), _group(group), _key(key),
      _def(def), _text(text), _label(0)
{
    d = new KConfigItemPrivate(type, valueType, object);

    if ( !_def.cast(valueType) )
        kdWarning() << k_funcinfo << "cannot cast default value to type : "
                    << QVariant::typeToName(valueType) << endl;
    if (col) col->insert(this);

    if ( d->object() ) {
        QObject::connect(object, d->data().signal, SLOT(modifiedSlot()));
        QObject::connect(object, SIGNAL(destroyed(QObject *)),
                         SLOT(objectDestroyed()));
        if ( !text.isNull() ) setText(text);
    }
}

KConfigItem::~KConfigItem()
{
    delete d;
}

void KConfigItem::objectDestroyed()
{
    deleteLater();
}

void KConfigItem::setText(const QString &text)
{
    _text = text;
    setProxyLabel(_label);
}

void KConfigItem::setProxyLabel(QLabel *label)
{
    _label = label;
    if (label) {
        label->setText(_text);
        if ( !_whatsthis.isEmpty() ) QWhatsThis::add(_label, _whatsthis);
        if ( !_tooltip.isEmpty() ) QToolTip::add(_label, _tooltip);
    }

    if ( d->object() ) {
        QString text = (label ? QString::null : _text);
        const char *p = d->data().labelProperty;
        if (p) d->object()->setProperty(p, text);
    }
}

void KConfigItem::setWhatsThis(const QString &text)
{
    _whatsthis = text;
    if ( d->object() ) {
        if ( d->object()->inherits("QWidget") )
            QWhatsThis::add(static_cast<QWidget *>(d->object()), text);
        else if ( d->object()->inherits("KAction") )
            static_cast<KAction *>(d->object())->setWhatsThis(text);
    }
    if (_label) QWhatsThis::add(_label, text);
}

void KConfigItem::setToolTip(const QString &text)
{
    _tooltip = text;
    if ( d->object() ) {
        if ( d->object()->inherits("QWidget") )
            QToolTip::add(static_cast<QWidget *>(d->object()), text);
        else if ( d->object()->inherits("KAction") )
            static_cast<KAction *>(d->object())->setToolTip(text);
    }
    if (_label) QToolTip::add(_label, text);
}

void KConfigItem::loadState()
{
    setValue(configValue());
}

KConfigBase *KConfigItem::config() const
{
    if ( parent()==0 ) return kapp->config();
    return static_cast<KConfigCollection *>(parent())->config();
}

bool KConfigItem::saveState()
{
    KConfigGroupSaver cg(config(), _group);
    cg.config()->writeEntry(_key, value());
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
    KConfigGroupSaver cg(config(), _group);
    return cg.config()->readPropertyEntry(_key, _def);
}

bool KConfigItem::checkType(const QVariant &v) const
{
    bool canCast = v.canCast(_def.type());
    if ( !canCast )
        kdWarning() << k_funcinfo << "cannot cast the value to type : "
                    << _def.typeName() << endl;
    return canCast;
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
KSimpleConfigItem::KSimpleConfigItem(QVariant::Type type, QObject *object,
                                     const QString &group, const QString &key,
                                     const QVariant &def, const QString &text,
                                     KConfigCollection *col, const char *name)
    : KConfigItem(Simple, type, object, group, key, def, text, col, name)
{}

KSimpleConfigItem::~KSimpleConfigItem()
{}

//-----------------------------------------------------------------------------
KRangedConfigItem::KRangedConfigItem(QVariant::Type type, QObject *object,
                                     const QString &group, const QString &key,
                                     const QVariant &def,
                                     const QVariant &min, const QVariant &max,
                                     const QString &text,
                                     KConfigCollection *col, const char *name)
    : KConfigItem(Ranged, type, object, group, key, def, text, col, name)
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

    if ( object() ) {
        blockSignals(true); // do not change the state of the setting
        object()->setProperty("minValue", min);
        object()->setProperty("maxValue", max);
        blockSignals(false);
    }
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

    if ( object() ) {
        const KIntNumInput *in;
        const KDoubleNumInput *dn;
        switch ( (KConfigItemPrivate::RangedType)objectType() ) {
        case KConfigItemPrivate::IntInput:
            in = static_cast<const KIntNumInput *>(object());
            return kMin(kMax(v.toInt(), in->minValue()), in->maxValue());
        case KConfigItemPrivate::DoubleInput:
            dn = static_cast<const KDoubleNumInput *>(object());
            return kMin(kMax(v.toDouble(), dn->minValue()), dn->maxValue());
        case KConfigItemPrivate::SpinBox:
            return static_cast<const QSpinBox *>(object())->bound(v.toInt());
        case KConfigItemPrivate::Slider:
            return static_cast<const QSlider *>(object())->bound(v.toInt());
        case KConfigItemPrivate::Dial:
            return static_cast<const QDial *>(object())->bound(v.toInt());
        case KConfigItemPrivate::Selector:
            return static_cast<const KSelector *>(object())->bound(v.toInt());
        default:
            break;
        }
    } else {
        switch ( defaultValue().type() ) {
        case QVariant::Int:
            return kMin(kMax(v.toInt(), _min.toInt()), _max.toInt());
        case QVariant::Double:
            return kMin(kMax(v.toDouble(), _min.toDouble()), _max.toDouble());
        default:
            break;
        }
    }

    Q_ASSERT(false);
    return QVariant();
}

QVariant KRangedConfigItem::configValue() const
{
    return bound( KConfigItem::configValue() );
}

//-----------------------------------------------------------------------------
KMultiConfigItem::KMultiConfigItem(QObject *obj, uint nbItems,
                                   const QString &group, const QString &key,
                                   const QVariant &def, const QString &text,
                                   KConfigCollection *col, const char *name)
    : KConfigItem(Multi, QVariant::String, obj, group, key, def, text,
                  col, name),
      _entries(nbItems)
{
    if ( object()==0 ) return;

    KConfigItemPrivate::MultiType type =
        (KConfigItemPrivate::MultiType)objectType();
    if ( type==KConfigItemPrivate::ReadOnlyComboBox
         && object()->property("editable").toBool() ) {
        kdError() << k_funcinfo << "the combobox should be readonly" << endl;
        return;
    }

    QStringList list;
    for (uint i=0; i<nbItems; i++)
        list.append(QString::null);
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

KMultiConfigItem::~KMultiConfigItem()
{}

void KMultiConfigItem::map(int id, const char *entry, const QString &text)
{
    _entries[id] = entry;
    if ( object()==0 ) return;

    QButton *button;
    switch ( (KConfigItemPrivate::MultiType)objectType() ) {
    case KConfigItemPrivate::ReadOnlyComboBox:
        static_cast<QComboBox *>(object())->changeItem(text, id);
        break;
    case KConfigItemPrivate::RadioButtonGroup:
        button = static_cast<QButtonGroup *>(object())->find(id);
        if ( button && button->inherits("QRadioButton") )
            button->setText(text);
        else kdError() << k_funcinfo << "cannot find radio button at id #"
                       << id << endl;
        break;
    case KConfigItemPrivate::SelectAction:
        static_cast<KSelectAction *>(object())->changeItem(id, text);
        break;
    default:
        break;
    }
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
QDomDocument *KConfigCollection::_xml = 0;

KConfigCollection::KConfigCollection(KConfigBase *config,
                                     QObject *parent, const char *name)
    : KConfigItemList(parent, name), _config(config)
{}

KConfigCollection::~KConfigCollection()
{}

KConfigBase *KConfigCollection::config() const
{
    return (_config ? _config : kapp->config());
}

KConfigItem *KConfigCollection::configItem(const char *name) const
{
    QPtrListIterator<KConfigItemBase> it(list());
    for (; it.current()!=0; ++it) {
        KConfigItem *s = (KConfigItem *)it.current()->qt_cast("KConfigItem");
        if ( s && s->name()==name ) return s;
    }
    return 0;
}

QVariant KConfigCollection::configItemValue(const char *name)
{
    KConfigItem *item = createConfigItem(name, 0, 0);
    QVariant v;
    if (item) {
        v = item->configValue();
        delete item;
    }
    return v;
}

uint KConfigCollection::configItemIndex(const char *name)
{
    KConfigItem *item = createConfigItem(name, 0, 0);
    uint i = 0;
    if (item) {
        KMultiConfigItem *mci =
            static_cast<KMultiConfigItem *>(item->qt_cast("KMultiConfigItem"));
        if (mci) i = mci->configIndex();
        delete item;
    }
    return i;
}

QVariant KConfigCollection::configItemMaxValue(const char *name)
{
    KConfigItem *item = createConfigItem(name, 0, 0);
    QVariant v;
    if (item) {
        KRangedConfigItem *rci =
          static_cast<KRangedConfigItem *>(item->qt_cast("KRangedConfigItem"));
        if (rci) v = rci->maxValue();
        delete item;
    }
    return v;
}

QVariant KConfigCollection::configItemMinValue(const char *name)
{
    KConfigItem *item = createConfigItem(name, 0, 0);
    QVariant v;
    if (item) {
        KRangedConfigItem *rci =
          static_cast<KRangedConfigItem *>(item->qt_cast("KRangedConfigItem"));
        if (rci) v = rci->minValue();
        delete item;
    }
    return v;
}

void KConfigCollection::readConfigFile()
{
    if (_xml) return;
    _xml = new QDomDocument;

    QString name = KGlobal::instance()->instanceName();
    QString xml_file = locate("data", name + '/' + name + "config.rc");
    QFile file(xml_file);
    if ( !file.open( IO_ReadOnly ) )
        kdFatal() << "No XML config file \"" << xml_file << "\"" << endl;
    else {
        QByteArray buffer(file.readAll());
        _xml->setContent( QString::fromUtf8(buffer.data(), buffer.size()) );
    }
}

#define ENTRY_NAME "entry named \"" << name << "\""

KConfigItem *KConfigCollection::createConfigItem(const char *name,
                                                 QObject *object)
{
    return createConfigItem(name, object, this);
}

KConfigItem *KConfigCollection::createConfigItem(const char *name,
                                       QObject *object, KConfigCollection *col)
{
    readConfigFile();
    QDomElement root = _xml->namedItem("kconfig").toElement();
    if ( root.isNull() ) kdFatal() << "no \"kconfig\" element" << endl;

    // find entry and group
    bool several = false;
    QDomElement entry, group;
    QDomNodeList groups = root.elementsByTagName("Group");
    for (uint i=0; i<groups.count(); i++) {
        QDomElement grp = groups.item(i).toElement();
        if ( grp.isNull() ) continue;
        QDomNodeList list = grp.elementsByTagName("Entry");
        for (uint j=0; j<list.count(); j++) {
            QDomElement elt = list.item(j).toElement();
            if ( elt.isNull() || elt.attribute("name")!=name ) continue;
            if ( !entry.isNull() ) several = true;
            entry = elt;
            group = grp;
        }
    }
    if ( entry.isNull() ) kdFatal() << "no " << ENTRY_NAME << endl;
    if (several) kdWarning() << "several " << ENTRY_NAME << endl;

    // read common attributes
    QString groupKey = group.attribute("name");
    QString key = entry.attribute("key");
    if ( key.isEmpty() ) key = name;
    QString text = entry.namedItem("text").toElement().text();
    QString whatsthis = entry.namedItem("whatsthis").toElement().text();
    QString tooltip = entry.namedItem("tooltip").toElement().text();

    // recognize item type
    QString itype = entry.attribute("type");
    KConfigItem::Type type = KConfigItem::NB_ITEM_TYPES;
    if ( itype.isEmpty() ) type = KConfigItem::Simple;
    else {
        for (uint i=0; i<KConfigItem::NB_ITEM_TYPES; i++)
            if ( itype==KConfigItemPrivate::name((KConfigItem::Type)i) ) {
                type = (KConfigItem::Type)i;
                break;
            }
        if ( type==KConfigItem::NB_ITEM_TYPES ) {
            kdFatal() << "unrecognized item type (" << itype << ") for "
                      << ENTRY_NAME << endl;
            type = KConfigItem::Simple;
        }
    }

    // recognize value type
    QString stype = entry.attribute("vtype");
    QVariant::Type vtype = QVariant::String;
    if ( type==KConfigItem::Multi ) {
        if ( !stype.isEmpty() && stype!="QString" )
            kdWarning() << ENTRY_NAME
                        << " : only QString value type is allowed for"
                        << " multi config" << endl;
    } else {
        vtype = QVariant::nameToType(stype.utf8());
        if ( vtype==QVariant::Invalid )
            kdFatal() << ENTRY_NAME << " : invalid value type" << endl;
    }

    // check default value
    QVariant def = KConfigString::toVariant(entry.attribute("defaultValue"),
                                            vtype);

    // specific
    KConfigItem *ci = 0;
    if ( type==KConfigItem::Simple )
        ci = new KSimpleConfigItem(vtype, object, groupKey, key, def, text,
                                   col, name);
    else if ( type==KConfigItem::Ranged ) {
        QVariant min = KConfigString::toVariant(entry.attribute("minValue"),
                                                vtype);
        QVariant max = KConfigString::toVariant(entry.attribute("maxValue"),
                                                vtype);
       ci = new KRangedConfigItem(vtype, object, groupKey, key, def, min,
                                  max, text, col, name);
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
            new KMultiConfigItem(object, names.count(), groupKey, key, def,
                                 text, col, name);
        for (uint i=0; i<names.count(); i++)
            mci->map(i, names[i].utf8(), i18n(texts[i].utf8()));
        ci = mci;
    }

    if ( !whatsthis.isNull() ) ci->setWhatsThis(whatsthis);
    if ( !tooltip.isNull() ) ci->setToolTip(tooltip);
    return ci;
}

//-----------------------------------------------------------------------------
KConfigWidget::KConfigWidget(const QString &title, const QString &icon,
                               QWidget *parent, const char *name,
                               KConfigBase *config)
    : QWidget(parent, name), _title(title), _icon(icon)
{
    _configCollection = new KConfigCollection(config, this);
}

KConfigWidget::~KConfigWidget()
{}

//-----------------------------------------------------------------------------
KConfigDialog::KConfigDialog(QWidget *parent, const char *name)
    : KDialogBase(IconList, i18n("Configure"),
                  Ok|Apply|Cancel|Default, Cancel, parent, name, true, true)
{
    connect(this, SIGNAL(aboutToShowPage(QWidget *)),
            SLOT(aboutToShowPageSlot(QWidget *)));
    enableButtonApply(false);
}

KConfigDialog::~KConfigDialog()
{}

void KConfigDialog::append(KConfigWidget *w)
{
    QFrame *page = addPage(w->title(), QString::null,
                           BarIcon(w->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);
    _widgets.append(w);

    w->configCollection()->load();
    connect(w->configCollection(), SIGNAL(modified()), SLOT(modified()));
    if ( pageIndex(page)==0 ) aboutToShowPage(page);
}

void KConfigDialog::slotDefault()
{
    int i = activePageIndex();
    _widgets.at(i)->configCollection()->setDefault();
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
    int i = activePageIndex();
    bool hasDefault = _widgets.at(i)->configCollection()->hasDefault();
    enableButton(Default, !hasDefault);
    enableButtonApply(true);
}

bool KConfigDialog::apply()
{
    bool ok = true;
    for (uint i=0; i<_widgets.count(); i++)
        if ( !_widgets.at(i)->configCollection()->save() ) ok = false;
    emit saved();
    return ok;
}

void KConfigDialog::slotApply()
{
    if ( apply() ) enableButtonApply(false);
}

void KConfigDialog::aboutToShowPageSlot(QWidget *page)
{
    int i = pageIndex(page);
    bool hasDefault = _widgets.at(i)->configCollection()->hasDefault();
    enableButton(Default, !hasDefault);
}
