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
KConfigItem::KConfigItem(const Data &data, QObject *object,
                         const QString &group, const QString &key,
                         const QVariant &def, const QString &text,
                         KConfigCollection *col, const char *name)
    : KConfigItemBase(col, name), _data(data),
      _group(group), _key(key), _def(def), _text(text), _label(0)
{
    if ( !_def.cast(data.type) )
        kdWarning() << k_funcinfo << "cannot cast default value to type : "
                    << QVariant::typeToName(data.type) << endl;
    if (col) col->insert(this);

    _object = object;
    if (object) {
        if ( !object->inherits(data.className) ) {
            kdError() << k_funcinfo << "unsupported object type" << endl;
            return;
        }
        QObject::connect(object, data.signal, SLOT(modifiedSlot()));
        QObject::connect(object, SIGNAL(destroyed(QObject *)),
                         SLOT(objectDestroyed()));
        if ( !text.isNull() ) setText(text);
    }
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

    if (_object) {
        QString text = (label ? QString::null : _text);
        const char *p = _data.labelProperty;
        if (p) _object->setProperty(p, text);
    }
}

void KConfigItem::setWhatsThis(const QString &text)
{
    _whatsthis = text;
    if ( _object->inherits("QWidget") )
        QWhatsThis::add(static_cast<QWidget *>(_object), text);
    else if ( _object->inherits("KAction") )
        static_cast<KAction *>(_object)->setWhatsThis(text);
    if (_label) QWhatsThis::add(_label, text);
}

void KConfigItem::setToolTip(const QString &text)
{
    _tooltip = text;
    if ( _object->inherits("QWidget") )
        QToolTip::add(static_cast<QWidget *>(_object), text);
    else if ( _object->inherits("KAction") )
        static_cast<KAction *>(_object)->setToolTip(text);
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
    Q_ASSERT(_object);
    checkType(v);

    bool ok = false;
    const char *p = _data.property;
    Q_ASSERT(p);
    ok = _object->setProperty(p, v);
    Q_ASSERT(ok);
}

QVariant KConfigItem::value() const
{
    Q_ASSERT(_object);

    const char *p = _data.property;
    Q_ASSERT(p);
    QVariant v = _object->property(p);
    Q_ASSERT(v.isValid());
    return v;
}

//-----------------------------------------------------------------------------
const KConfigItem::Data KSimpleConfigItem::DATA[NB_TYPES] = {
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
     "text",        0,      QVariant::String},
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid }
};

KSimpleConfigItem::KSimpleConfigItem(Type type, QObject *object,
                                     const QString &group, const QString &key,
                                     const QVariant &def, const QString &text,
                                     KConfigCollection *col, const char *name)
    : KConfigItem(DATA[type], object, group, key, def, text, col, name)
{}

//-----------------------------------------------------------------------------
const KConfigItem::Data KRangedConfigItem::DATA[NB_TYPES] = {
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
     "value", 0, QVariant::Int},
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid }
};

KRangedConfigItem::KRangedConfigItem(Type type, QObject *object,
                                     const QString &group, const QString &key,
                                     const QVariant &def,
                                     const QVariant &min, const QVariant &max,
                                     const QString &text,
                                     KConfigCollection *col, const char *name)
    : KConfigItem(DATA[type], object, group, key, def, text, col, name),
      _type(type)
{
    setRange(min, max);
}

void KRangedConfigItem::setRange(const QVariant &min, const QVariant &max)
{
    checkType(min);
    _min = min;
    checkType(max);
    _max = max;

    if (object()) {
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

    if (object()) {
        const KIntNumInput *in;
        const KDoubleNumInput *dn;
        switch (_type) {
        case IntInput:
            in = static_cast<const KIntNumInput *>(object());
            return kMin(kMax(v.toInt(), in->minValue()), in->maxValue());
        case DoubleInput:
            dn = static_cast<const KDoubleNumInput *>(object());
            return kMin(kMax(v.toDouble(), dn->minValue()), dn->maxValue());
        case SpinBox:
            return static_cast<const QSpinBox *>(object())->bound(v.toInt());
        case Slider:
            return static_cast<const QSlider *>(object())->bound(v.toInt());
        case Dial:
            return static_cast<const QDial *>(object())->bound(v.toInt());
        case Selector:
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

    return QVariant();
}

QVariant KRangedConfigItem::configValue() const
{
    return bound( KConfigItem::configValue() );
}

//-----------------------------------------------------------------------------
const KConfigItem::Data KMultiConfigItem::DATA[NB_TYPES] = {
    {"ReadOnlyComboBox", "QComboBox", SIGNAL(activated(const QString &)),
     "currentText", 0, QVariant::String},
    {"RadioButtonGroup", "QButtonGroup", SIGNAL(clicked(int)),
     0, "text", QVariant::String},
    {"SelectAction", "KSelectAction", SIGNAL(activated(int)),
     "currentText", "text", QVariant::String},
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, 0, QVariant::Invalid }
};

KMultiConfigItem::KMultiConfigItem(Type type, QObject *object, uint nbItems,
                                   const QString &group, const QString &key,
                                   const QVariant &def, const QString &text,
                                   KConfigCollection *col, const char *name)
    : KConfigItem(DATA[type], object, group, key, def, text, col, name),
      _type(type), _entries(nbItems)
{
    if ( type==ReadOnlyComboBox
         && object && object->property("editable").toBool() ) {
        kdError() << k_funcinfo << "the combobox should be readonly" << endl;
        return;
    }

    if ( object==0 ) return;
    QStringList list;
    for (uint i=0; i<nbItems; i++)
        list.append(QString::null);
    switch (type) {
    case ReadOnlyComboBox:
        static_cast<QComboBox *>(object)->insertStringList(list);
        break;
    case SelectAction:
        static_cast<KSelectAction *>(object)->setItems(list);
        break;
    default:
        break;
    }
}

void KMultiConfigItem::map(int id, const char *entry, const QString &text)
{
    _entries[id] = entry;
    if ( object()==0 ) return;

    QButton *button;
    switch (_type) {
    case ReadOnlyComboBox:
        static_cast<QComboBox *>(object())->changeItem(text, id);
        break;
    case RadioButtonGroup:
        button = static_cast<QButtonGroup *>(object())->find(id);
        if ( button && button->inherits("QRadioButton") )
            button->setText(text);
        else kdError() << k_funcinfo << "cannot find radio button at id #"
                       << id << endl;
        break;
    case SelectAction:
        static_cast<KSelectAction *>(object())->changeItem(id, text);
        break;
    default:
        break;
    }
}

int KMultiConfigItem::mapToId(const char *entry) const
{
    for (uint i=0; i<_entries.size(); i++)
        if ( _entries[i]==entry ) return i;

    bool ok;
    uint i = QString(entry).toUInt(&ok);
    if ( ok && i<_entries.size() ) return i;
    return -1;
}

void KMultiConfigItem::setValue(const QVariant &v)
{
    Q_ASSERT(object());
    checkType(v);

    bool ok = false;
    int id = mapToId(v.toString().utf8());
    if ( id==-1 ) return;
    if ( _type==RadioButtonGroup ) {
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
    if ( _type==RadioButtonGroup )
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
    int id = mapToId(configValue().toString().utf8());
    if ( id==-1 ) id = mapToId(defaultValue().toString().utf8());
    if ( id==-1 ) return 0;
    return id;
}

//-----------------------------------------------------------------------------
const KConfigCollection::ItemData
KConfigCollection::ITEM_DATA[NB_ITEM_TYPES] = {
    { KSimpleConfigItem::NB_TYPES, KSimpleConfigItem::DATA },
    { KRangedConfigItem::NB_TYPES, KRangedConfigItem::DATA },
    { KMultiConfigItem::NB_TYPES,  KMultiConfigItem::DATA  }
};

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
            (KMultiConfigItem *)item->qt_cast("KMultiConfigItem");
        if (mci) i = mci->configIndex();
        delete item;
    }
    return i;
}

void KConfigCollection::readConfigFile()
{
    if (_xml) return;
    _xml = new QDomDocument;

    QString name = KGlobal::instance()->instanceName();
    QString xml_file = locate("data", name + '/' + name + "config.rc");
    QFile file(xml_file);
    if ( !file.open( IO_ReadOnly ) )
        kdError(1000) << "No XML config file " << xml_file << endl;
    else {
        QByteArray buffer(file.readAll());
        _xml->setContent( QString::fromUtf8(buffer.data(), buffer.size()) );
    }
}

#define ENTRY_NAME "entry named \"" << name << "\")"

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
    if ( entry.isNull() ) kdError() << "no " << ENTRY_NAME << endl;
    if (several) kdWarning() << "several " << ENTRY_NAME << endl;

    // read common attributes
    QString groupKey = group.attribute("name");
    QString key = entry.attribute("key");
    if ( key.isEmpty() ) kdWarning() << ENTRY_NAME << " has empty key" << endl;
    QString text = entry.namedItem("text").toElement().text();
    QString whatsthis = entry.namedItem("whatsthis").toElement().text();
    QString tooltip = entry.namedItem("tooltip").toElement().text();

    // recognize type
    QString stype = entry.attribute("type");
    ItemType type;
    uint specificType;
    const KConfigItem::Data *data = 0;
    for (uint i = 0; i<NB_ITEM_TYPES; i++)
        for (uint k = 0; k<ITEM_DATA[i].nb; k++)
            if ( stype==ITEM_DATA[i].data[k].typeName ) {
                data = &ITEM_DATA[i].data[k];
                type = (ItemType)i;
                specificType = k;
                break;
            }
    if ( data==0 ) {
        kdError() << "type not recognized for " << ENTRY_NAME << endl;
        data = &ITEM_DATA[0].data[0];
    }

    // check default value
    QVariant def = KConfigString::toVariant(entry.attribute("defaultValue"),
                                            data->type);

    // specific
    if ( type==Simple )
        return new KSimpleConfigItem((KSimpleConfigItem::Type)specificType,
                                     object, groupKey, key, def, text,
                                     col, name);
    else if ( type==Ranged ) {
        QVariant min = KConfigString::toVariant(entry.attribute("minValue"),
                                                data->type);
        QVariant max = KConfigString::toVariant(entry.attribute("maxValue"),
                                                data->type);
        return new KRangedConfigItem((KRangedConfigItem::Type)specificType,
                                     object, groupKey, key, def, min, max,
                                     text, col, name);
    } else if ( type==Multi ) {
        QDomNodeList list = entry.elementsByTagName("Item");
        QStringList names, texts;
        for (uint i=0; i<list.count(); i++) {
            QDomElement item = list.item(i).toElement();
            if ( item.isNull() ) continue;
            names.append(item.attribute("name"));
            texts.append(item.text());
        }
        KMultiConfigItem *mci =
            new KMultiConfigItem((KMultiConfigItem::Type)specificType, object,
                                 names.count(), groupKey, key, def, text,
                                 col, name);
        for (uint i=0; i<names.count(); i++)
            mci->map(i, names[i].utf8(), i18n(texts[i].utf8()));
        return mci;
    }
    Q_ASSERT(false);
    return 0;
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
    : KDialogBase(IconList, i18n("Configure..."),
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
