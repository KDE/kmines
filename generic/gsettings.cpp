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


//-----------------------------------------------------------------------------
KUIConfigBase::KUIConfigBase(QObject *parent)
    : QObject(parent), _modified(false)
{}

KUIConfigBase::~KUIConfigBase()
{}

void KUIConfigBase::load()
{
    blockSignals(true); // do not emit modified()
    loadState();
    blockSignals(false);
    _modified = false;
}

bool KUIConfigBase::save()
{
    if ( !_modified ) return true;
    bool success = saveState();
    if (success) {
        _modified = false;
        emit saved();
    }
    return success;
}

void KUIConfigBase::setDefault()
{
    // NB: we emit modified() by hand because some widget (like QComboBox)
    // reports changes with a signal that only gets activated by user actions.
    blockSignals(true);
    setDefaultState();
    blockSignals(false);
    modifiedSlot();
}

void KUIConfigBase::modifiedSlot()
{
    _modified = true;
    emit modified();
}

//-----------------------------------------------------------------------------
KUIConfigList::KUIConfigList(QObject *parent)
    : KUIConfigBase(parent)
{}

KUIConfigList::~KUIConfigList()
{
    QPtrListIterator<KUIConfigBase> it(_list);
    for (; it.current()!=0; ++it) {
        it.current()->disconnect(this, SLOT(UIConfigDestroyed(QObject *)));
        delete it.current();
    }
}

void KUIConfigList::insert(KUIConfigBase *setting)
{
    connect(setting, SIGNAL(modified()), SLOT(modifiedSlot()));
    connect(setting, SIGNAL(destroyed(QObject *)),
            SLOT(UIConfigDestroyed(QObject *)));
    _list.append(setting);
}

void KUIConfigList::remove(KUIConfigBase *setting)
{
    delete setting;
}

void KUIConfigList::loadState()
{
    QPtrListIterator<KUIConfigBase> it(_list);
    for (; it.current()!=0; ++it) it.current()->load();
}

bool KUIConfigList::saveState()
{
    QPtrListIterator<KUIConfigBase> it(_list);
    bool ok = true;
    for (; it.current()!=0; ++it)
        if ( !it.current()->save() ) ok = false;
    return ok;
}

void KUIConfigList::setDefaultState()
{
    QPtrListIterator<KUIConfigBase> it(_list);
    for (; it.current()!=0; ++it) it.current()->setDefault();
}

bool KUIConfigList::hasDefault() const
{
    QPtrListIterator<KUIConfigBase> it(_list);
    for (; it.current()!=0; ++it)
        if ( !it.current()->hasDefault() ) return false;
    return true;
}

void KUIConfigList::UIConfigDestroyed(QObject *object)
{
    _list.removeRef(static_cast<KUIConfigBase *>(object));
}

//-----------------------------------------------------------------------------
KUIConfig::KUIConfig(const QString &group, const QString &key,
                     const QVariant &def, KUIConfigCollection *col,
                     const QString &text, QVariant::Type type)
    : KUIConfigBase(col), _object(0), _group(group), _key(key), _def(def),
      _text(text), _label(0)
{
    if ( !_def.cast(type) )
        kdWarning() << k_funcinfo << "cannot cast default value to type : "
                    << QVariant::typeToName(type) << endl;
    if (col) col->insert(this);
}

void KUIConfig::associate(QObject *object)
{
    Q_ASSERT( _object==0 );
    _object = object;
    if (object) {
        if ( !object->inherits(data().className) ) {
            kdError() << k_funcinfo << "unsupported object type" << endl;
            return;
        }
        QObject::connect(object, data().signal, SLOT(modifiedSlot()));
        QObject::connect(object, SIGNAL(destroyed(QObject *)),
                         SLOT(objectDestroyed()));
        if ( !_text.isNull() ) setText(_text);
    }
}

void KUIConfig::objectDestroyed()
{
    deleteLater();
}

void KUIConfig::setText(const QString &text)
{
    _text = text;
    setProxyLabel(_label);
}

void KUIConfig::setProxyLabel(QLabel *label)
{
    _label = label;
    if (label) label->setText(_text);

    if (_object) {
        QString text = (label ? QString::null : _text);
        const char *p = data().labelProperty;
        if (p) _object->setProperty(p, text);
    }
}

void KUIConfig::loadState()
{
    setValue(configValue());
}

KConfigBase *KUIConfig::config() const
{
    if ( parent()==0 ) return kapp->config();
    return static_cast<KUIConfigCollection *>(parent())->config();
}

bool KUIConfig::saveState()
{
    KConfigGroupSaver cg(config(), _group);
    cg.config()->writeEntry(_key, value());
    return true;
}

void KUIConfig::setDefaultState()
{
    setValue(_def);
}

bool KUIConfig::hasDefault() const
{
    return ( value()==_def );
}

QVariant KUIConfig::configValue() const
{
    KConfigGroupSaver cg(config(), _group);
    return cg.config()->readPropertyEntry(_key, _def);
}

bool KUIConfig::checkType(const QVariant &v) const
{
    bool canCast = v.canCast(_def.type());
    if ( !canCast )
        kdWarning() << k_funcinfo << "cannot cast the value to type : "
                    << _def.typeName() << endl;
    return canCast;
}

void KUIConfig::setValue(const QVariant &v)
{
    Q_ASSERT(_object);
    checkType(v);

    bool ok = false;
    const char *p = data().property;
    Q_ASSERT(p);
    ok = _object->setProperty(p, v);
    Q_ASSERT(ok);
}

QVariant KUIConfig::value() const
{
    Q_ASSERT(_object);

    const char *p = data().property;
    Q_ASSERT(p);
    QVariant v = _object->property(p);
    Q_ASSERT(v.isValid());
    return v;
}

//-----------------------------------------------------------------------------
const KUIConfig::Data KSimpleUIConfig::DATA[NB_TYPES] = {
    {"QCheckBox", SIGNAL(toggled(bool)),
     "checked",     "text", QVariant::Bool},
    {"QLineEdit", SIGNAL(textChanged(const QString &)),
     "text",        0,      QVariant::String},
    {"KColorButton", SIGNAL(changed(const QColor &)),
     "color",       0,      QVariant::Color},
    {"KToggleAction", SIGNAL(toggled(bool)),
     "checked",     "text", QVariant::Bool},
    // KColorComboBox must to be before QComboBox
    {"KColorCombo", SIGNAL(activated(const QString &)),
     "color",       0,      QVariant::UInt},
    {"QComboBox", SIGNAL(activated(const QString &)),
     "currentText", 0,      QVariant::UInt},
    {"KDatePicker", SIGNAL(dateChanged(QDate)),
     "date",        0,      QVariant::Date},
    {"KFontAction", SIGNAL(activated(int)),
     "font",        "text", QVariant::String},
    {"KFontSizeAction", SIGNAL(activated(int)),
     "fontSize",    "text", QVariant::Int},
    {"QDateTimeEdit", SIGNAL(valueChanged(const QDateTime &)),
     "dateTime",    0,      QVariant::DateTime},
    {"QTextEdit", SIGNAL(textChanged(const QString &)),
     "text",        0,      QVariant::String},
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid }
};

KSimpleUIConfig::KSimpleUIConfig(Type type, const QString &group,
                                 const QString &key, const QVariant &def,
                                 KUIConfigCollection *col, const QString &text)
    : KUIConfig(group, key, def, col, text, DATA[type].type), _type(type)
{}

//-----------------------------------------------------------------------------
const KUIConfig::Data KRangedUIConfig::DATA[NB_TYPES] = {
    {"KIntNumInput", SIGNAL(valueChanged(int)),
     "value", "label", QVariant::Int},
    {"KDoubleNumInput", SIGNAL(valueChanged(double)),
     "value", "label", QVariant::Double},
    {"QSpinBox", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    {"QSlider", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    {"QDial", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    {"KSelector", SIGNAL(valueChanged(int)),
     "value", 0, QVariant::Int},
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid }
};

KRangedUIConfig::KRangedUIConfig(Type type, const QString &group,
                               const QString &key, const QVariant &def,
                               const QVariant &min, const QVariant &max,
                               KUIConfigCollection *col, const QString &text)
    : KUIConfig(group, key, def, col, text, DATA[type].type),
      _type(type), _min(min), _max(max)
{
    checkType(min);
    checkType(max);
}

void KRangedUIConfig::associate(QObject *object)
{
    KUIConfig::associate(object);
    setRange(_min, _max);
}

void KRangedUIConfig::setRange(const QVariant &min, const QVariant &max)
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

QVariant KRangedUIConfig::minValue() const
{
    return (object() ? object()->property("minValue") : _min);
}

QVariant KRangedUIConfig::maxValue() const
{
    return (object() ? object()->property("maxValue") : _max);
}

QVariant KRangedUIConfig::bound(const QVariant &v) const
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

QVariant KRangedUIConfig::configValue() const
{
    return bound( KUIConfig::configValue() );
}

//-----------------------------------------------------------------------------
const KUIConfig::Data KMultiUIConfig::DATA[NB_TYPES] = {
    {"QComboBox", SIGNAL(activated(const QString &)),
     "currentText", 0, QVariant::String},
    {"QButtonGroup", SIGNAL(clicked(int)),
     0, "text", QVariant::String},
    {"KSelectAction", SIGNAL(activated(int)),
     "currentText", "text", QVariant::String},
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid },
    { 0, 0, 0, 0, QVariant::Invalid }
};

KMultiUIConfig::KMultiUIConfig(Type type, uint nbItems,
                             const QString &group, const QString &key,
                             const QVariant &def, KUIConfigCollection *col,
                             const QString &text)
    : KUIConfig(group, key, def, col, text, DATA[type].type),
      _type(type), _nbItems(nbItems)
{}

void KMultiUIConfig::associate(QObject *object)
{
    KUIConfig::associate(object);
    if ( _type==ReadOnlyComboBox && object->property("editable").toBool() ) {
        kdError() << k_funcinfo << "the combobox should be readonly" << endl;
        return;
    }

    QStringList list;
    for (uint i=0; i<_nbItems; i++)
        list.push_back(_entries[i]);

    switch (_type) {
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

void KMultiUIConfig::map(int id, const QString &entry, const QString &text)
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

int KMultiUIConfig::mapToId(const QString &entry) const
{
    QMap<int, QString>::ConstIterator it;
    for (it = _entries.begin(); it != _entries.end(); ++it)
        if ( it.data()==entry ) return it.key();

    bool ok;
    int i = entry.toUInt(&ok);
    if (ok) return i;
    return -1;
}

void KMultiUIConfig::setValue(const QVariant &v)
{
    Q_ASSERT(object());
    checkType(v);

    bool ok = false;
    int id = mapToId(v.toString());
    if ( id==-1 ) return;
    if ( _type==RadioButtonGroup ) {
        QButton *b = static_cast<QButtonGroup *>(object())->find(id);
        if ( b && b->inherits("QRadioButton") )
            static_cast<QRadioButton *>(b)->setChecked(true);
    } else ok = object()->setProperty("currentItem", id);

    Q_ASSERT(ok);
}

uint KMultiUIConfig::findRadioButtonId(const QButtonGroup *group) const
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

QVariant KMultiUIConfig::value() const
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
    if ( _entries.contains(id) ) return _entries[id];
    return id;
}

int KMultiUIConfig::configId() const
{
    int id = mapToId(configValue().toString());
    if ( id==-1 ) id = mapToId(defaultValue().toString());
    if ( id==-1 ) return 0;
    return id;
}

//-----------------------------------------------------------------------------
KUIConfigCollection::KUIConfigCollection(KConfigBase *config, QObject *parent)
    : KUIConfigList(parent), _config(config)
{}

KUIConfigCollection::~KUIConfigCollection()
{}

KConfigBase *KUIConfigCollection::config() const
{
    return (_config ? _config : kapp->config());
}

KUIConfig *KUIConfigCollection::UIConfig(QObject *obj) const
{
    QPtrListIterator<KUIConfigBase> it(list());
    for (; it.current()!=0; ++it) {
        KUIConfig *s = (KUIConfig *)it.current()->qt_cast("KUIConfig");
        if ( s && s->object()==obj ) return s;
    }
    return 0;
}

//-----------------------------------------------------------------------------
KUIConfigWidget::KUIConfigWidget(const QString &title, const QString &icon,
                               QWidget *parent, const char *name,
                               KConfigBase *config)
    : QWidget(parent, name), _title(title), _icon(icon)
{
    _UIConfigCollection = new KUIConfigCollection(config, this);
}

KUIConfigWidget::~KUIConfigWidget()
{}

//-----------------------------------------------------------------------------
KUIConfigDialog::KUIConfigDialog(QWidget *parent, const char *name)
    : KDialogBase(IconList, i18n("Configure..."),
                  Ok|Apply|Cancel|Default, Cancel, parent, name, true, true)
{
    connect(this, SIGNAL(aboutToShowPage(QWidget *)),
            SLOT(aboutToShowPageSlot(QWidget *)));
    enableButtonApply(false);
}

KUIConfigDialog::~KUIConfigDialog()
{}

void KUIConfigDialog::append(KUIConfigWidget *w)
{
    QFrame *page = addPage(w->title(), QString::null,
                           BarIcon(w->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);
    _widgets.append(w);

    w->UIConfigCollection()->load();
    connect(w->UIConfigCollection(), SIGNAL(modified()), SLOT(modified()));
    if ( pageIndex(page)==0 ) aboutToShowPage(page);
}

void KUIConfigDialog::slotDefault()
{
    int i = activePageIndex();
    _widgets.at(i)->UIConfigCollection()->setDefault();
}

void KUIConfigDialog::accept()
{
    if ( apply() ) {
        KDialogBase::accept();
        kapp->config()->sync(); // #### safer
    }
}

void KUIConfigDialog::modified()
{
    int i = activePageIndex();
    bool hasDefault = _widgets.at(i)->UIConfigCollection()->hasDefault();
    enableButton(Default, !hasDefault);
    enableButtonApply(true);
}

bool KUIConfigDialog::apply()
{
    bool ok = true;
    for (uint i=0; i<_widgets.count(); i++)
        if ( !_widgets.at(i)->UIConfigCollection()->save() ) ok = false;
    emit saved();
    return ok;
}

void KUIConfigDialog::slotApply()
{
    if ( apply() ) enableButtonApply(false);
}

void KUIConfigDialog::aboutToShowPageSlot(QWidget *page)
{
    int i = pageIndex(page);
    bool hasDefault = _widgets.at(i)->UIConfigCollection()->hasDefault();
    enableButton(Default, !hasDefault);
}
