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
KSettingGeneric::KSettingGeneric(QObject *parent)
    : QObject(parent), _modified(false)
{}

KSettingGeneric::~KSettingGeneric()
{}

void KSettingGeneric::load()
{
    blockSignals(true); // do not emit hasBeenModified
    loadState();
    blockSignals(false);
    _modified = false;
}

bool KSettingGeneric::save()
{
    if ( !_modified ) return true;
    bool success = saveState();
    if (success) {
        _modified = false;
        emit hasBeenSaved();
    }
    return success;
}

void KSettingGeneric::setDefaults()
{
    // NB: we emit hasBeenModified by hand because some widget (like QComboBox)
    // reports changes with a signal that only gets activated by user actions.
    blockSignals(true);
    setDefaultsState();
    blockSignals(false);
    hasBeenModifiedSlot();
}

void KSettingGeneric::hasBeenModifiedSlot()
{
    _modified = true;
    emit hasBeenModified();
}

//-----------------------------------------------------------------------------
KSettingList::KSettingList(QObject *parent)
    : KSettingGeneric(parent)
{}

KSettingList::~KSettingList()
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it) {
        it.current()->disconnect(this, SLOT(settingDestroyed(QObject *)));
        delete it.current();
    }
}

void KSettingList::insert(KSettingGeneric *setting)
{
    connect(setting, SIGNAL(hasBeenModified()), SLOT(hasBeenModifiedSlot()));
    connect(setting, SIGNAL(destroyed(QObject *)),
            SLOT(settingDestroyed(QObject *)));
    _settings.append(setting);
}

void KSettingList::remove(KSettingGeneric *setting)
{
    delete setting;
}

void KSettingList::loadState()
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it) it.current()->load();
}

bool KSettingList::saveState()
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    bool ok = true;
    for (; it.current()!=0; ++it)
        if ( !it.current()->save() ) ok = false;
    return ok;
}

void KSettingList::setDefaultsState()
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

void KSettingList::settingDestroyed(QObject *object)
{
    _settings.removeRef(static_cast<KSettingGeneric *>(object));
}

//-----------------------------------------------------------------------------
KSetting::KSetting(const QString &group, const QString &key,
                   const QVariant &def, KSettingCollection *col,
                   const QString &text, QVariant::Type type)
    : KSettingGeneric(col), _object(0), _group(group), _key(key), _def(def),
      _text(text), _label(0)
{
    if ( !_def.cast(type) )
        kdWarning() << k_funcinfo << "cannot cast default value to type : "
                    << QVariant::typeToName(type) << endl;
    if (col) col->insert(this);
}

void KSetting::associate(QObject *object)
{
    Q_ASSERT( _object==0 );
    _object = object;
    if (object) {
        if ( !object->inherits(data().className) ) {
            kdError() << k_funcinfo << "unsupported object type" << endl;
            return;
        }
        QObject::connect(object, data().signal, SLOT(hasBeenModifiedSlot()));
        QObject::connect(object, SIGNAL(destroyed(QObject *)),
                         SLOT(objectDestroyed()));
        if ( !_text.isNull() ) setText(_text);
    }
}

void KSetting::objectDestroyed()
{
    deleteLater();
}

QWidget *KSetting::widget() const
{
    if ( _object==0 || !_object->inherits("QWidget") ) return 0;
    return static_cast<QWidget *>(_object);
}

void KSetting::setText(const QString &text)
{
    _text = text;
    setProxyLabel(_label);
}

void KSetting::setProxyLabel(QLabel *label)
{
    _label = label;
    if (label) label->setText(_text);

    if (_object) {
        QString text = (label ? QString::null : _text);
        const char *p = data().labelProperty;
        if (p) _object->setProperty(p, text);
    }
}

void KSetting::loadState()
{
    setValue(configValue());
}

KConfigBase *KSetting::config() const
{
    if ( parent()==0 ) return kapp->config();
    return static_cast<KSettingCollection *>(parent())->config();
}

bool KSetting::saveState()
{
    KConfigGroupSaver cg(config(), _group);
    cg.config()->writeEntry(_key, value());
    return true;
}

void KSetting::setDefaultsState()
{
    setValue(_def);
}

bool KSetting::hasDefaults() const
{
    return ( value()==_def );
}

QVariant KSetting::configValue() const
{
    KConfigGroupSaver cg(config(), _group);
    return cg.config()->readPropertyEntry(_key, _def);
}

bool KSetting::checkType(const QVariant &v) const
{
    bool canCast = v.canCast(_def.type());
    if ( !canCast )
        kdWarning() << k_funcinfo << "cannot cast the value to type : "
                    << _def.typeName() << endl;
    return canCast;
}

void KSetting::setValue(const QVariant &v)
{
    Q_ASSERT(_object);
    checkType(v);

    bool ok = false;
    const char *p = data().property;
    if (p) ok = _object->setProperty(p, v);
    Q_ASSERT(ok);
}

QVariant KSetting::value() const
{
    Q_ASSERT(_object);

    const char *p = data().property;
    Q_ASSERT(p);
    QVariant v = _object->property(p);
    Q_ASSERT(v.isValid());
    return v;
}

//-----------------------------------------------------------------------------
const KSetting::Data KSimpleSetting::DATA[NB_TYPES] = {
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
     "text",        0,      QVariant::String}
};

KSimpleSetting::KSimpleSetting(Type type, const QString &group,
                               const QString &key, const QVariant &def,
                               KSettingCollection *col, const QString &text)
    : KSetting(group, key, def, col, text, DATA[type].type), _type(type)
{}

//-----------------------------------------------------------------------------
const KSetting::Data KRangedSetting::DATA[NB_TYPES] = {
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
     "value", 0, QVariant::Int}
};

KRangedSetting::KRangedSetting(Type type, const QString &group,
                               const QString &key, const QVariant &def,
                               const QVariant &min, const QVariant &max,
                               KSettingCollection *col, const QString &text)
    : KSetting(group, key, def, col, text, DATA[type].type),
      _type(type), _min(min), _max(max)
{
    checkType(min);
    checkType(max);
}

void KRangedSetting::associate(QObject *object)
{
    KSetting::associate(object);
    setRange(_min, _max);
}

void KRangedSetting::setRange(const QVariant &min, const QVariant &max)
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

QVariant KRangedSetting::minValue() const
{
    return (object() ? object()->property("minValue") : _min);
}

QVariant KRangedSetting::maxValue() const
{
    return (object() ? object()->property("maxValue") : _max);
}

QVariant KRangedSetting::bound(const QVariant &v) const
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

QVariant KRangedSetting::configValue() const
{
    return bound( KSetting::configValue() );
}

//-----------------------------------------------------------------------------
const KSetting::Data KMultiSetting::DATA[NB_TYPES] = {
    {"QComboBox", SIGNAL(activated(const QString &)),
     "currentText", 0, QVariant::String},
    {"QButtonGroup", SIGNAL(clicked(int)),
     0, "text", QVariant::String},
    {"KSelectAction", SIGNAL(activated(int)),
     "currentText", "text", QVariant::String}
};

KMultiSetting::KMultiSetting(Type type, uint nbItems,
                             const QString &group, const QString &key,
                             const QVariant &def, KSettingCollection *col,
                             const QString &text)
    : KSetting(group, key, def, col, text, DATA[type].type),
      _type(type), _nbItems(nbItems)
{}

void KMultiSetting::associate(QObject *object)
{
    KSetting::associate(object);
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

void KMultiSetting::map(int id, const QString &entry, const QString &text)
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

int KMultiSetting::mapToId(const QString &entry) const
{
    QMap<int, QString>::ConstIterator it;
    for (it = _entries.begin(); it != _entries.end(); ++it)
        if ( it.data()==entry ) return it.key();

    bool ok;
    int i = entry.toUInt(&ok);
    if (ok) return i;
    return -1;
}

void KMultiSetting::setValue(const QVariant &v)
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

uint KMultiSetting::findRadioButtonId(const QButtonGroup *group) const
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

QVariant KMultiSetting::value() const
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

int KMultiSetting::configId() const
{
    int id = mapToId(configValue().toString());
    if ( id==-1 ) id = mapToId(defaultValue().toString());
    if ( id==-1 ) return 0;
    return id;
}

//-----------------------------------------------------------------------------
KSettingCollection::KSettingCollection(KConfigBase *config, QObject *parent)
    : KSettingList(parent), _config(config)
{
    if ( config==0 ) _config = kapp->config();
}

KSettingCollection::~KSettingCollection()
{}

KSetting *KSettingCollection::setting(QObject *o) const
{
    QPtrListIterator<KSettingGeneric> it(_settings);
    for (; it.current()!=0; ++it) {
        if ( !it.current()->inherits("KSetting") ) continue;
        KSetting *s = static_cast<KSetting *>(it.current());
        if ( s->object()==o ) return s;
    }
    return 0;
}
/*
void KSettingCollection::createSettings(const QString &xmlFile)
{
    QString xml = (!xmlFile.isNull() ? xmlFile
                 : locate("data", instance()->instanceName() + "settings.rc"));
    QFile file(xml);
    QString content;
    if ( !file.open(IO_ReadOnly) ) {
        content = QString::fromLatin1( "<!DOCTYPE ksetting>\n<ksetting name=\"empty\">\n</ksetting>");
    }
    QByteArray buffer(file.readAll());
    content = QString::fromUtf8(buffer.data(), buffer.size());
    file.close();
    _doc.setContent(content);
}

QColor stringToColor(const QString &s)
{
    QColor color;

    if( def.isEmpty() ) return color;
    if ( def.at(0)=='#' ) {
        color.setNamedColor(def);
        return color;
    }
    int i = def.find(',');
    if( i==-1 ) return color;
    int red = def.left(i).toInt();
    int old = i;
    i = def.find(',', old+1);
    if( i==-1 ) return color;
    int green = def.mid(old+1, i-old-1 ).toInt();
    int blue = def.right(def.length()-i-1 ).toInt();
    color.setRgb(red, green, blue);
    return color;
}

QStringList stringToList(const QString &s, char sep)
{
    QStringList list;
    int i;
    value = "";
    int len = s.length();

    for (i=0; i<len; i++) {
        if ( s[i]!=sep && s[i]!='\\') {
            value += s[i];
            continue;
        }
        if ( s[i]=='\\' ) {
            i++;
            value += s[i];
            continue;
        }
        // if we fell through to here, we are at a separator.  Append
        // contents of value to the list
        // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
        // A QStrList may contain values in 8bit locale cpecified
        // encoding
        list.append(value);
        value.truncate(0);
    }

    if ( s[len-1]!=sep ) list.append(value);
    return list;
}

QDateTime stringToDateTime(const QString &s)
{
    QDateTime dateTime;
    QStringList list = stringToList(s, ',');
    if( list.count() == 6 ) {
        QTime time;
        QDate date;

        date.setYMD( QString::fromLatin1( list.at( 0 ) ).toInt(),
                     QString::fromLatin1( list.at( 1 ) ).toInt(),
                     QString::fromLatin1( list.at( 2 ) ).toInt() );
        time.setHMS( QString::fromLatin1( list.at( 3 ) ).toInt(),
                     QString::fromLatin1( list.at( 4 ) ).toInt(),
                     QString::fromLatin1( list.at( 5 ) ).toInt() );

        dateTime.setTime( time );
        dateTime.setDate( date );
    }
    return dateTime;
}

void KSettingCollection::insert(const QString &name, const QString &group,
                                KSetting *setting)
{
    QDomNodeList list = _doc.elementByTagName("group");
    QDomElement elt;
    uint k;
    for (k=0; k<list.count(); k++) {
        elt = list.item(k).toElement();
        if ( elt.attribute("name")==group ) break;
    }
    if ( k==list.count() ) {
        kdError() << k_funcinfo << "Unknown group \"" << group << "\"" << endl;
        return;
    }

    list = elt.elementByTagName("setting");
    for (k=0; k<list.count(); k++) {
        elt = list.item(k).toElement();
        if ( elt.attribute("name")==name ) break;
    }
    if ( k==list.count() ) {
        kdError() << k_funcinfo << "Unknown setting \"" << name << "\"" <<endl;
        return;
    }

    QVariant::Type type = QVariant::nameToType( elt.attribute("type") );
    QString def = elt.attribute("default").toLowerCase();
    QVariant v;
    bool b, ok;
    int i;
    switch (type) {
    case QVariant::Bool:
        if ( def=="true" || def=="on" || def=="yes" ) b = true;
        else {
            i = def.toInt(&ok);
            b = (ok && i);
        }
        v = QVariant(b, 0);
        break;
    case QVariant::String:
        v = QVariant(def);
        break;
    case QVariant::Color:
        v = QVariant(stringToColor(def));
        break;
    case QVariant::Int:
        v = QVariant(def.toInt());
        // #### TODO min/max
        break;
    case QVariant::Double:
        v = QVariant(def.toDouble());
        // #### TODO min/max
        break;
    case QVariant::Date:
        v = QVariant(stringToDateTime(def).toDate());
        break;
    case QVariant::DateTime:
        v = QVariant(stringToDateTime(def));
        break;
    case QVariant::StringList:
        // #### TODO
        break;
    default:
        kdError() << k_funcinfo << "Setting type not allowed \""
                  << elt.attribute("type") << "\"" << endl;
        return;
    }

    setting->set(name, group, v);
}
*/

//-----------------------------------------------------------------------------
KSettingWidget::KSettingWidget(const QString &title, const QString &icon,
                               QWidget *parent, const char *name,
                               KConfigBase *config)
    : QWidget(parent, name), _title(title), _icon(icon)
{
    _settings = new KSettingCollection(config, this);
}

KSettingWidget::~KSettingWidget()
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

KSettingDialog::~KSettingDialog()
{}

void KSettingDialog::append(KSettingWidget *w)
{
    QFrame *page = addPage(w->title(), QString::null,
                           BarIcon(w->icon(), KIcon::SizeLarge));
    w->reparent(page, 0, QPoint());
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->addWidget(w);
    vbox->addStretch(1);
    _widgets.append(w);

    w->settingCollection()->load();
    connect(w->settingCollection(), SIGNAL(hasBeenModified()),
            SLOT(changed()));
    if ( pageIndex(page)==0 ) aboutToShowPage(page);
}

void KSettingDialog::slotDefault()
{
    int i = activePageIndex();
    _widgets.at(i)->settingCollection()->setDefaults();
}

void KSettingDialog::accept()
{
    if ( apply() ) {
        KDialogBase::accept();
        kapp->config()->sync(); // #### REMOVE when fixed in kdelibs
                                // creating a KPushButton will lose all
                                // unsaved data ...
    }
}

void KSettingDialog::changed()
{
    int i = activePageIndex();
    bool hasDefaults = _widgets.at(i)->settingCollection()->hasDefaults();
    enableButton(Default, !hasDefaults);
    enableButtonApply(true);
}

bool KSettingDialog::apply()
{
    bool ok = true;
    for (uint i=0; i<_widgets.count(); i++)
        if ( !_widgets.at(i)->settingCollection()->save() ) ok = false;
    emit settingsSaved();
    return ok;
}

void KSettingDialog::slotApply()
{
    if ( apply() ) enableButtonApply(false);
}

void KSettingDialog::slotAboutToShowPage(QWidget *page)
{
    int i = pageIndex(page);
    bool hasDefaults = _widgets.at(i)->settingCollection()->hasDefaults();
    enableButton(Default, !hasDefaults);
}
