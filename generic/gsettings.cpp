/*
    This file is part of the KDE games library
    Copyright (C) 2001 Nicolas Hadacek (hadacek@kde.org)

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
#include <qguardedptr.h>
#include <qtextedit.h>
#include <qdial.h>
#include <qdatetimeedit.h>

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
// #include <kurlrequester.h>


//-----------------------------------------------------------------------------
KSettingProxy::KSettingProxy()
    : QObject(0, "KSettingProxy")
{}

//-----------------------------------------------------------------------------
KSettingGeneric::KSettingGeneric()
{}

//-----------------------------------------------------------------------------
KSettingList::KSettingList()
{
    _settings.setAutoDelete(true);
}

void KSettingList::plug(KSettingGeneric *setting)
{
    QObject::connect(setting->proxy(), SIGNAL(changed()),
                     proxy(), SIGNAL(changed()));
    _settings.append(setting);
}

void KSettingList::load()
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it) it.current()->load();
}

void KSettingList::save()
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it) it.current()->save();
}

bool KSettingList::isSaved() const
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it)
        if ( !it.current()->isSaved() ) return false;
    return true;
}

void KSettingList::setDefaults()
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it) it.current()->setDefaults();
}

bool KSettingList::hasDefaults() const
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it)
        if ( !it.current()->hasDefaults() ) return false;
    return true;
}

//-----------------------------------------------------------------------------

const KSettingCollection::Item::Data
KSettingCollection::Item::DATA[KSettingCollection::Item::NB_TYPES] = {
{ "QCheckBox", SIGNAL(toggled(bool)),                  QVariant::Bool, false },
{ "KToggleAction", SIGNAL(toggled(bool)),              QVariant::Bool, false },

{ "QlineEdit", SIGNAL(textChanged(const QString &)), QVariant::String, false },
{ "QTextEdit", SIGNAL(textChanged(const QString &)), QVariant::String, false },
//{ "KURLRequester", SIGNAL(textChanged(const QString &)),
//                                                     QVariant::String, false },
{ "KDatePicker", SIGNAL(dateChanged(QDate)),           QVariant::Date, false },
{ "QDateTimeEdit", SIGNAL(valueChanged(const QDateTime &)),
                                                   QVariant::DateTime, false },

{ "KColorButton", SIGNAL(changed(const QColor &)),    QVariant::Color, false },
{ "QButtonGroup", SIGNAL(clicked(int)),              QVariant::String, true  },

// KColorComboBox must to be before QComboBox
{ "KColorCombo", SIGNAL(activated(const QString &)),  QVariant::Color, false },
{ "QComboBox", SIGNAL(activated(const QString &)),   QVariant::String, true  },

{ "QSpinBox", SIGNAL(valueChanged(int)),                QVariant::Int, false },
{ "QSlider", SIGNAL(valueChanged(int)),                 QVariant::Int, false },
{ "QDial", SIGNAL(valueChanged(int)),                   QVariant::Int, false },
{ "KSelector", SIGNAL(valueChanged(int)),               QVariant::Int, false },

{ "KIntNumInput", SIGNAL(valueChanged(int)),            QVariant::Int, false },
{ "KDoubleNumInput", SIGNAL(valueChanged(double)),   QVariant::Double, false },

// KFontAction and KFontSizeAction must be before KSelectAction
{ "KFontAction", SIGNAL(activated(int)),             QVariant::String, false },
{ "KFontSizeAction", SIGNAL(activated(int)),            QVariant::Int, false },
{ "KSelectAction", SIGNAL(activated(int)),           QVariant::String, true  }
};

KSettingCollection::Item::Item(QObject *o, const QString &group,
                               const QString &key, const QVariant &def)
    : _group(group), _key(key), _gobj(o), _obj(o), _def(def)
{
    uint i = 0;
    for (; i<NB_TYPES; i++)
        if ( o->inherits(DATA[i].className) ) break;
    _type = (Type)i;
    if ( i==NB_TYPES ) {
        kdError() << k_funcinfo << "unsupported object type" << endl;
        return;
    }

    bool canCast = _def.cast(DATA[_type].type);
    if ( !canCast )
        kdWarning() << k_funcinfo << "cannot cast default value to type : "
                    << def.typeName() << endl;

    QObject::connect(o, DATA[_type].signal, proxy(), SIGNAL(changed()));

    // create the entry in config file if not there
    KConfigGroupSaver cg(kapp->config(), group);
    if ( !cg.config()->hasKey(key) ) cg.config()->writeEntry(key, _def);
}

void KSettingCollection::Item::map(int id, const QString &entry)
{
    if ( !isMulti() ) {
        kdError() << k_funcinfo
                  << "it makes no sense to define a mapping for this object"
                  << endl;
        return;
    }
    _entries[id] = entry;
}

int KSettingCollection::Item::mapToId(const QString &entry) const
{
    QMap<int, QString>::ConstIterator it;
    for ( it = _entries.begin(); it != _entries.end(); ++it )
        if ( it.data()==entry ) return it.key();

    bool ok;
    int i = entry.toUInt(&ok);
    if (ok) return i;
    return -1;
}

int KSettingCollection::Item::findRadioButtonId(const QButtonGroup *group)
{
    QObjectList *list = group->queryList("QRadioButton");
    QObjectListIt it(*list);
    for (; it.current()!=0; ++it) {
        QRadioButton *rb = (QRadioButton *)it.current();
        if ( rb->isChecked() ) return group->id(rb);
    }
    delete list;
    kdWarning() << k_funcinfo
                << "there is no QRadioButton in this QButtonGroup" << endl;
    return -1;
}

bool KSettingCollection::Item::isMulti() const
{
    if ( !DATA[_type].multi ) return false;
    if ( _type==ComboBox && static_cast<const QComboBox *>(_obj)->editable() )
        return false;
    return true;
}

QVariant KSettingCollection::Item::currentValue() const
{
    const QComboBox *combo;
    int id;

    switch (_type) {
    case CheckBox:
        return QVariant(static_cast<const QCheckBox *>(_obj)->isChecked(), 0);
    case LineEdit:
        return static_cast<const QLineEdit *>(_obj)->text();
    case TextEdit:
        return static_cast<const QTextEdit *>(_obj)->text();
//    case URLRequester:
//        return static_cast<const KURLRequester *>(_obj)->url();
    case ColorButton:
        return static_cast<const KColorButton *>(_obj)->color();
    case ComboBox:
        combo = static_cast<const QComboBox *>(_obj);
        if ( combo->editable() ) return combo->currentText();
        id = combo->currentItem();
        break;
    case IntInput:
        return static_cast<const KIntNumInput *>(_obj)->value();
    case DoubleInput:
        return static_cast<const KDoubleNumInput *>(_obj)->value();
    case SpinBox:
        return static_cast<const QSpinBox *>(_obj)->value();
    case Slider:
        return static_cast<const QSlider *>(_obj)->value();
    case Dial:
        return static_cast<const QDial *>(_obj)->value();
    case Selector:
        return static_cast<const KSelector *>(_obj)->value();
    case ButtonGroup:
        id = findRadioButtonId(static_cast<const QButtonGroup *>(_obj));
        break;
    case ToggleAction:
        return QVariant(static_cast<const KToggleAction *>(_obj)->isChecked(),
                        0);
    case SelectAction:
        id = static_cast<const KSelectAction *>(_obj)->currentItem();
        break;
    case ColorComboBox:
        return static_cast<const KColorCombo *>(_obj)->color();
    case DatePicker:
        // #### KDatePicker::getDate should be const
        return static_cast<KDatePicker *>(_obj)->getDate();
    case DateTimeEdit:
        return static_cast<QDateTimeEdit *>(_obj)->dateTime();
    case FontAction:
        return static_cast<const KFontAction *>(_obj)->font();
    case FontSizeAction:
        return static_cast<const KFontSizeAction *>(_obj)->fontSize();
    default:
        Q_ASSERT(false);
        return QVariant();
    }

    // multiple choices only
    Q_ASSERT( isMulti() );
    if ( _entries.contains(id) ) return _entries[id];
    return id;
}

void KSettingCollection::Item::setCurrentValue(const QVariant &value)
{
    int id;
    if ( isMulti() ) {
        id = mapToId(value.toString());
        if ( id==-1 ) return;
    }
    QButton *button;
    QComboBox *combo;

    switch (_type) {
    case CheckBox:
        static_cast<QCheckBox *>(_obj)->setChecked(value.toBool());
        break;
    case LineEdit:
        static_cast<QLineEdit *>(_obj)->setText(value.toString());
        break;
    case TextEdit:
        static_cast<QTextEdit *>(_obj)->setText(value.toString());
        break;
//    case URLRequester:
//        static_cast<KURLRequester *>(_obj)->setURL(value.toString());
//        break;
    case ColorButton:
        static_cast<KColorButton *>(_obj)->setColor(value.toColor());
        break;
    case ComboBox:
        combo = static_cast<QComboBox *>(_obj);
        if ( combo->editable() ) combo->setCurrentText(value.toString());
        else combo->setCurrentItem(id);
        break;
    case IntInput:
        static_cast<KIntNumInput *>(_obj)->setValue(value.toInt());
        break;
    case DoubleInput:
        static_cast<KDoubleNumInput *>(_obj)->setValue(value.toDouble());
        break;
    case SpinBox:
        static_cast<QSpinBox *>(_obj)->setValue(value.toInt());
        break;
    case Slider:
        static_cast<QSlider *>(_obj)->setValue(value.toInt());
        break;
    case Dial:
        static_cast<QDial *>(_obj)->setValue(value.toInt());
        break;
    case Selector:
        static_cast<KSelector *>(_obj)->setValue(value.toInt());
        break;
    case ButtonGroup:
        button = static_cast<QButtonGroup *>(_obj)->find(id);
        if ( button && button->inherits("QRadioButton") )
            static_cast<QRadioButton *>(button)->setChecked(true);
        break;
    case ToggleAction:
        static_cast<KToggleAction *>(_obj)->setChecked(value.toBool());
        break;
    case SelectAction:
        static_cast<KSelectAction *>(_obj)->setCurrentItem(id);
        break;
    case ColorComboBox:
        static_cast<KColorCombo *>(_obj)->setColor(value.toColor());
        break;
    case DatePicker:
        static_cast<KDatePicker *>(_obj)->setDate(value.toDate());
        break;
    case DateTimeEdit:
        static_cast<QDateTimeEdit *>(_obj)->setDateTime(value.toDateTime());
    case FontAction:
        static_cast<KFontAction *>(_obj)->setFont(value.toString());
        break;
    case FontSizeAction:
        static_cast<KFontSizeAction *>(_obj)->setFontSize(value.toInt());
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

QVariant KSettingCollection::Item::loadValue() const
{
    KConfigGroupSaver cg(kapp->config(), _group);
    return cg.config()->readPropertyEntry(_key, _def);
}

void KSettingCollection::Item::load()
{
    if ( _gobj.isNull() ) return; // object destroyed
    setCurrentValue(loadValue());
}

void KSettingCollection::Item::save()
{
    if ( _gobj.isNull() ) return; // object destroyed
    KConfigGroupSaver cg(kapp->config(), _group);
    cg.config()->writeEntry(_key, currentValue());
}

void KSettingCollection::Item::setDefaults()
{
    if ( _gobj.isNull() ) return; // object destroyed
    setCurrentValue(_def);
    proxy()->emitChanged();
}

bool KSettingCollection::Item::hasDefaults() const
{
    if ( _gobj.isNull() ) return true; // object destroyed
    return ( currentValue()==_def );
}

QVariant KSettingCollection::Item::read() const
{
    if ( _gobj.isNull() ) return QVariant(); // object destroyed
    QVariant v = loadValue();
    int i;
    double d;
    const KIntNumInput *in;
    const KDoubleNumInput *dn;

    switch (_type) {
    case IntInput:
        in = static_cast<const KIntNumInput *>(_obj);
        i = kMax(v.toInt(), in->minValue());
        return kMin(i, in->maxValue());
    case DoubleInput:
        dn = static_cast<const KDoubleNumInput *>(_obj);
        d = kMax(v.toDouble(), dn->minValue());
        return kMin(d, dn->maxValue());
    case SpinBox:
        return static_cast<const QSpinBox *>(_obj)->bound(v.toInt());
    case Slider:
        return static_cast<const QSlider *>(_obj)->bound(v.toInt());
    case Dial:
        return static_cast<const QDial *>(_obj)->bound(v.toInt());
    case Selector:
        return static_cast<const KSelector *>(_obj)->bound(v.toInt());
    }

    return v;
}

int KSettingCollection::Item::readId() const
{
    if ( _gobj.isNull() ) return 0; // object destroyed
    if (  !isMulti() ) {
        kdError() << k_funcinfo
                  << "it makes no sense to use this method for this object"
                  << endl;
        return 0;
    }
    QString entry = loadValue().toString();
    int id = mapToId(entry);
    if ( id==-1 ) id = mapToId(_def.toString());
    if ( id==-1 ) return 0;
    return id;
}

//-----------------------------------------------------------------------------
KSettingCollection::KSettingCollection()
{}

void KSettingCollection::plug(QObject *o, const QString &group,
                              const QString &key, const QVariant &def)
{
    Item *item = new Item(o, group, key, def);
    if ( !item->objectRecognized() ) {
        delete item;
        return;
    }
    KSettingList::plug(item);
    _items.append(item);
}

KSettingCollection::Item *KSettingCollection::find(const QObject *o) const
{
    QPtrListIterator<Item> it(_items);
    for (; it.current()!=0; ++it)
        if ( it.current()->contains(o) ) return it.current();
    return 0;
}

void KSettingCollection::map(const QObject *o, int id, const QString &entry)
{
    Item *item = find(o);
    if ( item==0 ) {
        kdError() << k_funcinfo << "you need to plug the object before"
                  << endl;
        return;
    }
    item->map(id, entry);
}

QVariant KSettingCollection::readValue(const QObject *o) const
{
    const Item *item = find(o);
    if ( item==0 ) {
        kdError() << k_funcinfo << "you need to plug the object before"
                  << endl;
        return QVariant();
    }
    return item->read();
}

int KSettingCollection::readId(const QObject *o) const
{
    const Item *item = find(o);
    if ( item==0 ) {
        kdError() << k_funcinfo << "you need to plug the object before"
                  << endl;
        return 0;
    }
    return item->readId();
}

//-----------------------------------------------------------------------------
KSettingWidget::KSettingWidget(const QString &title, const QString &icon,
                               QWidget *parent, const char *name)
    : QWidget(parent, name), _title(title), _icon(icon)
{}

//-----------------------------------------------------------------------------
KSettingDialog::KSettingDialog(QWidget *parent, const char *name)
    : KDialogBase(IconList, i18n("Configure..."),
                  Ok|Apply|Cancel|Default, Cancel, parent, name, true, true)
{
    setIconListAllVisible(true);
    connect(this, SIGNAL(aboutToShowPage(QWidget *)),
            SLOT(slotAboutToShowPage(QWidget *)));
    enableButtonApply(false);
}

void KSettingDialog::append(KSettingWidget *w)
{
    QFrame *page = addPage(w->title(), QString::null,
                           BarIcon(w->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);
    _widgets.append(w);
    _changed.append(false);

    w->load();
    connect(w->proxy(), SIGNAL(changed()), SLOT(changed()));
    if ( pageIndex(page)==0 ) aboutToShowPage(page);
}

void KSettingDialog::slotDefault()
{
    int i = activePageIndex();
    _widgets.at(i)->setDefaults();
}

void KSettingDialog::accept()
{
    if ( apply() ) KDialogBase::accept();
}

void KSettingDialog::changed()
{
    int i = activePageIndex();
    _changed[i] = true;
    bool hasDefaults = _widgets.at(i)->hasDefaults();
    enableButton(Default, !hasDefaults);

    enableButtonApply(true);
}

bool KSettingDialog::apply()
{
    bool changed = false;
    bool ok = true;

    for (uint i=0; i<_widgets.count(); i++) {
        if ( !_changed[i] ) continue;
        _widgets.at(i)->save();
        if ( !_widgets.at(i)->isSaved() ) ok = false;
        else {
            changed = true;
            _changed[i] = false;
        }
    }

    if (changed) emit settingsSaved();
    return ok;
}

void KSettingDialog::slotApply()
{
    if ( apply() ) enableButtonApply(false);
}

void KSettingDialog::slotAboutToShowPage(QWidget *page)
{
    int i = pageIndex(page);
    bool hasDefaults = _widgets.at(i)->hasDefaults();
    enableButton(Default, !hasDefaults);
}
